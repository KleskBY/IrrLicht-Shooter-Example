#include "scene.h"
#include "materials.h"
#include <unordered_map>
#include <fstream>
#include "lightgrid.h"
#include "collision.h"
#include "dylight.h"
#include "Player.h"

scene::ISceneNode* SceneNode;
scene::IMesh* SceneMesh;

void ProcessNodes()
{
	core::array<scene::ISceneNode*> nodes;
	smgr->getSceneNodesFromType(scene::ESNT_ANY, nodes); // Find all nodes
	for (u32 i = 0; i < nodes.size(); ++i)
	{
		scene::ISceneNode* node = nodes[i];
		scene::ITriangleSelector* selector = 0;

		node->setID(IDFlag_IsPickable);

		switch (node->getType())
		{
		case scene::ESNT_CUBE:
		case scene::ESNT_ANIMATED_MESH:
			// Because the selector won't animate with the mesh,
			// and is only being used for camera collision, we'll just use an approximate
			// bounding box instead of ((scene::IAnimatedMeshSceneNode*)node)->getMesh(0)
			selector = smgr->createTriangleSelectorFromBoundingBox(node);
			break;

		case scene::ESNT_MESH:
		case scene::ESNT_SPHERE: // Derived from IMeshSceneNode
			selector = smgr->createTriangleSelector(((scene::IMeshSceneNode*)node)->getMesh(), node);
			selector = smgr->createOctreeTriangleSelector(((scene::IMeshSceneNode*)node)->getMesh(), node, 128);
			//((scene::IMeshSceneNode*)node)->addShadowVolumeSceneNode();
			break;

		case scene::ESNT_TERRAIN:
			selector = smgr->createTerrainTriangleSelector((scene::ITerrainSceneNode*)node);
			break;

		//case scene::ESNT_OCTREE:
		//	selector = smgr->createOctreeTriangleSelector(((scene::IMeshSceneNode*)node)->getMesh(), node);
		//	break;
		case scene::ESNT_OCTREE:
		{
			// Safe cast to mesh node
			scene::IMeshSceneNode* meshNode = dynamic_cast<scene::IMeshSceneNode*>(node);
			if (!meshNode)
				break;

			scene::IMesh* mesh = meshNode->getMesh();
			if (!mesh || mesh->getMeshBufferCount() == 0)
				break;

			// Validate that at least one buffer has vertices and indices
			bool hasValidGeometry = false;
			for (u32 mb = 0; mb < mesh->getMeshBufferCount(); ++mb)
			{
				scene::IMeshBuffer* mbuf = mesh->getMeshBuffer(mb);
				if (mbuf && mbuf->getVertexCount() > 0 && mbuf->getIndexCount() > 0)
				{
					hasValidGeometry = true;
					break;
				}
			}

			if (!hasValidGeometry)
				break;

			// Use octree selector for performance on large meshes
			selector = smgr->createOctreeTriangleSelector(mesh, node, 128);
		}
		break;

		default:
			// Don't create a selector for this node type
			break;
		}

		if (selector)
		{
			node->setTriangleSelector(selector);
			meta->addTriangleSelector(selector); // Add it to the meta selector, which will take a reference to it
			selector->drop(); // And drop my reference to it, so that the meta selector owns it.
		}
	}
}

bool LoadLevelObj(std::string levelName)
{
	if (!smgr->getFileSystem()->existFile(std::string("data/maps/" + levelName + "/").c_str()))
	{
		return false;
	}

	for (int i = 0; i < 100; i++)
	{
		std::string mdlname = "data/maps/" + levelName + "/" + std::to_string(i) + ".obj";
		if (smgr->getFileSystem()->existFile(mdlname.c_str()))
		{
			scene::IMesh* meshTangents = smgr->getMeshManipulator()->createMeshWithTangents(smgr->getMesh(mdlname.c_str()));//smgr->getMesh(mdlname.c_str());
			scene::IMesh* mesh = smgr->getMesh(mdlname.c_str());

			scene::IMeshSceneNode* node = smgr->addMeshSceneNode(meshTangents);

			//The sun (trash)
			scene::ILightSceneNode* light = smgr->addLightSceneNode(0, core::vector3df(0, 100, 100), video::SColor(255, 255, 200, 200));
			light->setLightType(irr::video::ELT_DIRECTIONAL);
			light->setRotation(core::vector3df(0.0, 45.0, 40.0));

			auto shape = new IBvhTriangleMeshShape(node, mesh, 0);
			collision::world->addRigidBody(shape);

			SetDefaultMaterial(node);
			meshTangents->drop();
		}
	}

	ProcessNodes();
}

void ScaleMeshVertices(scene::IMesh* mesh, float scale)
{
	if (!mesh) return;

	for (u32 i = 0; i < mesh->getMeshBufferCount(); ++i)
	{
		scene::IMeshBuffer* buf = mesh->getMeshBuffer(i);
		if (buf->getVertexType() != video::EVT_2TCOORDS) continue;

		video::S3DVertex2TCoords* verts = (video::S3DVertex2TCoords*)buf->getVertices();
		for (u32 j = 0; j < buf->getVertexCount(); ++j)
			verts[j].Pos *= scale;

		buf->recalculateBoundingBox();
	}

	// Scale the mesh's bounding box
	core::aabbox3df box = mesh->getBoundingBox();

	core::vector3df min = box.MinEdge * scale;
	core::vector3df max = box.MaxEdge * scale;

	core::aabbox3df scaledBox(min, max);
	mesh->setBoundingBox(scaledBox);
}

std::vector<scene::SMesh*> SplitMeshBuffer(const scene::IMeshBuffer* buffer, u32 maxVerts = 65535)
{
	std::vector<scene::SMesh*> chunks;

	if (!buffer || buffer->getVertexType() != video::EVT_2TCOORDS)
		return chunks;

	const video::S3DVertex2TCoords* srcVerts = (const video::S3DVertex2TCoords*)buffer->getVertices();
	const u16* srcIndices = buffer->getIndices();
	u32 indexCount = buffer->getIndexCount();

	std::unordered_map<u16, u16> indexMap;
	std::vector<video::S3DVertex2TCoords> verts;
	std::vector<u16> indices;

	for (u32 i = 0; i < indexCount; ++i)
	{
		u16 original = srcIndices[i];

		if (indexMap.find(original) == indexMap.end())
		{
			if (verts.size() >= maxVerts)
			{
				auto* mesh = new scene::SMesh();
				auto* newBuf = new scene::SMeshBufferLightMap();

				newBuf->Material = buffer->getMaterial();
				for (const auto& v : verts) newBuf->Vertices.push_back(v);
				for (const auto& idx : indices) newBuf->Indices.push_back(idx);

				if (mesh->getMeshBufferCount() == 0) continue;

				newBuf->recalculateBoundingBox();
				mesh->addMeshBuffer(newBuf);
				mesh->recalculateBoundingBox();
				newBuf->drop();
				chunks.push_back(mesh);

				verts.clear();
				indices.clear();
				indexMap.clear();
			}

			indexMap[original] = (u16)verts.size();
			verts.push_back(srcVerts[original]);
		}

		indices.push_back(indexMap[original]);
	}

	if (!indices.empty())
	{
		auto* mesh = new scene::SMesh();
		auto* newBuf = new scene::SMeshBufferLightMap();

		newBuf->Material = buffer->getMaterial();
		for (const auto& v : verts) newBuf->Vertices.push_back(v);
		for (const auto& idx : indices) newBuf->Indices.push_back(idx);

		newBuf->recalculateBoundingBox();
		mesh->addMeshBuffer(newBuf);
		mesh->recalculateBoundingBox();
		newBuf->drop();
		chunks.push_back(mesh);
	}

	for (auto it = chunks.begin(); it != chunks.end(); )
	{
		scene::IMesh* chunk = *it;
		if (!chunk || chunk->getMeshBufferCount() == 0)
		{
			it = chunks.erase(it);
			continue;
		}

		bool valid = false;
		for (u32 mb = 0; mb < chunk->getMeshBufferCount(); ++mb)
		{
			scene::IMeshBuffer* mbuf = chunk->getMeshBuffer(mb);
			if (mbuf && mbuf->getVertexCount() > 0 &&
				mbuf->getIndexCount() > 0 &&
				(mbuf->getIndexCount() % 3 == 0))
			{
				valid = true;
				break;
			}
		}

		if (!valid)
			it = chunks.erase(it);
		else
			++it;
	}

	return chunks;
}
s32 Q3StartPosition(IQ3LevelMesh* mesh, s32 startposIndex, float scale)
{
	if (0 == mesh) return 0;

	quake3::tQ3EntityList& entityList = mesh->getEntityList();
	quake3::IEntity search;
	search.name = "info_player_start";	// "info_player_deathmatch";

	// find all entities in the multi-list
	s32 lastIndex;
	s32 index = entityList.binary_search_multi(search, lastIndex);
	if (index < 0)
	{
		search.name = "info_player_deathmatch";
		index = entityList.binary_search_multi(search, lastIndex);
	}

	if (index < 0) return 0;
	index += core::clamp(startposIndex, 0, lastIndex - index);

	u32 parsepos;

	const quake3::SVarGroup* group;
	group = entityList[index].getGroup(1);

	parsepos = 0;
	vector3df pos = quake3::getAsVector3df(group->get("origin"), parsepos);

	parsepos = 0;
	f32 angle = quake3::getAsFloat(group->get("angle"), parsepos);

	parsepos = 0;
	vector3df angles = quake3::getAsVector3df(group->get("angles"), parsepos);
	//std::cout << angle << " " << std::endl;

	vector3df target(0.f, 0.f, 1.f);
	target.rotateXZBy(angle - 90.f, vector3df());

	pos = pos * scale;

	player::PlayerPosition = pos;
	player::CameraPos = pos;
	input::m_yaw = degToRad(angle - 90.f);
	return lastIndex - index + 1;
}

bool LoadLevelBsp5(std::string levelName)
{
	IQ3LevelMesh* q3mesh = (IQ3LevelMesh*)smgr->getMesh(levelName.c_str());
	if (!q3mesh) return false;

	scene::IMesh* fullMesh = q3mesh->getMesh(0);
	ScaleMeshVertices(fullMesh, 0.025f);
	SceneMesh = fullMesh;

	ParseLights(q3mesh);
	Q3StartPosition(q3mesh, 1, 0.025f);

	// Split buffers and add to scene
	for (u32 i = 0; i < fullMesh->getMeshBufferCount(); ++i)
	{
		scene::IMeshBuffer* buf = fullMesh->getMeshBuffer(i);
		if (buf->getVertexType() != video::EVT_2TCOORDS) continue;

		auto chunks = SplitMeshBuffer(buf); // no scale inside
		for (size_t k = 0; k < chunks.size(); ++k)
		{
			scene::ISceneNode* node = smgr->addOctreeSceneNode(chunks[k], 0, -1, 512);
			node->setName(("BSP_Chunk_" + std::to_string(i) + "_" + std::to_string(k)).c_str());
			node->setDebugDataVisible(EDS_BBOX | EDS_MESH_WIRE_OVERLAY);
			SetDefaultMaterial(node);
			collision::AddCollisionForMesh(node, chunks[k]);
		}
	}

	irr::scene::quake3::tQ3EntityList& entityList = q3mesh->getEntityList();
	for (u32 e = 0; e != entityList.size(); ++e)
	{
		for (u32 g = 0; g != entityList[e].getGroupSize(); ++g)
		{
			const irr::scene::quake3::SVarGroup* group = entityList[e].getGroup(g);

			for (u32 index = 0; index < group->Variable.size(); ++index)
			{
				const irr::scene::quake3::SVariable& v = group->Variable[index];
				//std::cout << v.content.c_str() << std::endl;
				if (v.content.find("func_") != std::string::npos)
				{
					scene::IMesh* brushMesh = q3mesh->getBrushEntityMesh(e);
					if (!brushMesh) continue;
					ScaleMeshVertices(brushMesh, 0.025f);

					scene::ISceneNode* node = smgr->addOctreeSceneNode(brushMesh, 0, -1, 512);
					node->setName((v.content.c_str() + std::to_string(e)).c_str());
					node->setDebugDataVisible(EDS_BBOX | EDS_MESH_WIRE_OVERLAY);
					SetDefaultMaterial(node);
					collision::AddCollisionForMesh(node, brushMesh);
				}
			}
		}
	}

	ProcessNodes();
	return true;
}



bool LoadLevelBsp6(std::string levelName)
{
	IQ3LevelMesh* mesh = (IQ3LevelMesh*)smgr->getMesh(levelName.c_str());
	if (!mesh) return false;
	scene::ISceneNode* node = 0;

	scene::IMesh* geometry = mesh->getMesh(quake3::E_Q3_MESH_GEOMETRY);
	ScaleMeshVertices(geometry, 0.025f);
	node = smgr->addOctreeSceneNode(geometry, 0, -1, 4096);
	collision::AddCollisionForMesh(node, geometry);

	scene::IMesh* additional_mesh = mesh->getMesh(quake3::E_Q3_MESH_ITEMS); // the additional mesh can be quite huge and is unoptimized
	ScaleMeshVertices(additional_mesh, 0.025f);
	for (u32 i = 0; i != additional_mesh->getMeshBufferCount(); ++i)
	{
		const IMeshBuffer* meshBuffer = additional_mesh->getMeshBuffer(i);
		const video::SMaterial& material = meshBuffer->getMaterial();
		const s32 shaderIndex = (s32)material.MaterialTypeParam2; // The ShaderIndex is stored in the material parameter
		std::cout << "shaderIndex: " << shaderIndex << std::endl;

		const quake3::IShader* shader = mesh->getShader(shaderIndex); // the meshbuffer can be rendered without additional support, or it has no shader
		if (!shader) continue;
		stringc s;
		dumpShader(s, shader, false);
		printf(s.c_str());
		node = smgr->addQuake3SceneNode(meshBuffer, shader);
	}


	irr::scene::quake3::tQ3EntityList& entityList = mesh->getEntityList();
	for (u32 e = 0; e != entityList.size(); ++e)
	{
		for (u32 g = 0; g != entityList[e].getGroupSize(); ++g)
		{
			const irr::scene::quake3::SVarGroup* group = entityList[e].getGroup(g);

			for (u32 index = 0; index < group->Variable.size(); ++index)
			{
				const irr::scene::quake3::SVariable& v = group->Variable[index];
				//std::cout << v.content.c_str() << std::endl;
				if (v.content.find("func_") != std::string::npos)
				{
					scene::IMesh* brushMesh = mesh->getBrushEntityMesh(entityList[e]);
					if (!brushMesh) continue;
					ScaleMeshVertices(brushMesh, 0.025f);

					for (u32 i = 0; i < brushMesh->getMeshBufferCount(); ++i)
					{
						const IMeshBuffer* meshBuffer = brushMesh->getMeshBuffer(i);
						const video::SMaterial& material = meshBuffer->getMaterial();
						ITexture* tex = material.getTexture(0);
						if (!tex) continue;
						stringc shaderName = tex->getName();
						shaderName.replace('\\', '/');
						shaderName = shaderName.subString(0, shaderName.findLast('.'));
						auto ind = shaderName.find("textures/");
						if(ind) shaderName = shaderName.subString(ind, shaderName.size() - ind);
						const quake3::IShader* shader = mesh->getShader(shaderName.c_str());
						if (!shader)
						{
							scene::ISceneNode* node = smgr->addOctreeSceneNode(brushMesh, 0, -1, 512);
							continue;
						}
						node = smgr->addQuake3SceneNode(meshBuffer, shader);
					}
				}
			}
		}
	}

	ProcessNodes();
	return true;
}

bool LoadLevelBsp7(std::string levelName)
{
	IQ3LevelMesh* mesh = (IQ3LevelMesh*)smgr->getMesh(levelName.c_str());
	if (!mesh) return false;

	ParseLights(mesh);
	Q3StartPosition(mesh, 1, 0.025f);

	// ---- WORLD GEOMETRY (E_Q3_MESH_GEOMETRY) ----
	scene::IMesh* geometry = mesh->getMesh(quake3::E_Q3_MESH_GEOMETRY);
	ScaleMeshVertices(geometry, 0.025f);

	for (u32 i = 0; i < geometry->getMeshBufferCount(); ++i)
	{
		scene::IMeshBuffer* buf = geometry->getMeshBuffer(i);
		if (buf->getVertexType() != video::EVT_2TCOORDS)
			continue;

		auto chunks = SplitMeshBuffer(buf); // split large buffers into smaller ones

		for (size_t k = 0; k < chunks.size(); ++k)
		{
			const video::SMaterial& material = chunks[k]->getMeshBuffer(0)->getMaterial();
			const s32 shaderIndex = (s32)material.MaterialTypeParam2;

			const quake3::IShader* shader = mesh->getShader(shaderIndex);

			if (shader)
			{
				smgr->addQuake3SceneNode(chunks[k]->getMeshBuffer(0), shader);
			}
			else
			{
				scene::ISceneNode* node = smgr->addOctreeSceneNode(chunks[k], 0, -1, 512);
				SetDefaultMaterial(node);
				collision::AddCollisionForMesh(node, chunks[k]);
			}
		}
	}

	// ---- ADDITIONAL ITEMS MESH (E_Q3_MESH_ITEMS) ----
	scene::IMesh* additional_mesh = mesh->getMesh(quake3::E_Q3_MESH_ITEMS);
	ScaleMeshVertices(additional_mesh, 0.025f);

	for (u32 i = 0; i < additional_mesh->getMeshBufferCount(); ++i)
	{
		const IMeshBuffer* meshBuffer = additional_mesh->getMeshBuffer(i);
		const video::SMaterial& material = meshBuffer->getMaterial();
		const s32 shaderIndex = (s32)material.MaterialTypeParam2;

		const quake3::IShader* shader = mesh->getShader(shaderIndex);
		if (!shader) continue;

		smgr->addQuake3SceneNode(meshBuffer, shader);
	}

	// ---- FUNC_* ENTITIES ----
	irr::scene::quake3::tQ3EntityList& entityList = mesh->getEntityList();
	for (u32 e = 0; e != entityList.size(); ++e)
	{
		for (u32 g = 0; g != entityList[e].getGroupSize(); ++g)
		{
			const irr::scene::quake3::SVarGroup* group = entityList[e].getGroup(g);

			for (u32 index = 0; index < group->Variable.size(); ++index)
			{
				const irr::scene::quake3::SVariable& v = group->Variable[index];

				if (v.content.find("func_") != std::string::npos)
				{
					scene::IMesh* brushMesh = mesh->getBrushEntityMesh(entityList[e]);
					if (!brushMesh) continue;
					ScaleMeshVertices(brushMesh, 0.025f);

					for (u32 i = 0; i < brushMesh->getMeshBufferCount(); ++i)
					{
						scene::IMeshBuffer* buf = brushMesh->getMeshBuffer(i);
						if (buf->getVertexType() != video::EVT_2TCOORDS)
							continue;

						// Split brush mesh buffers like worldspawn
						auto chunks = SplitMeshBuffer(buf);

						for (size_t k = 0; k < chunks.size(); ++k)
						{
							const video::SMaterial& material = chunks[k]->getMeshBuffer(0)->getMaterial();
							ITexture* tex = material.getTexture(0);
							if (!tex) continue;

							// Normalize texture name to Quake 3 shader name
							stringc shaderName = tex->getName();
							shaderName.replace('\\', '/');
							shaderName = shaderName.subString(0, shaderName.findLast('.'));
							auto ind = shaderName.find("textures/");
							if (ind != -1)
								shaderName = shaderName.subString(ind, shaderName.size() - ind);

							const quake3::IShader* shader = mesh->getShader(shaderName.c_str());

							if (shader)
							{
								smgr->addQuake3SceneNode(chunks[k]->getMeshBuffer(0), shader);
							}
							else
							{
								scene::ISceneNode* node = smgr->addOctreeSceneNode(chunks[k], 0, -1, 512);
								SetDefaultMaterial(node);
								collision::AddCollisionForMesh(node, chunks[k]);
							}
						}
					}
				}
			}
		}
	}

	ProcessNodes();
	return true;
}


bool LoadLevelBsp(std::string levelName)
{
	IQ3LevelMesh* mesh = (IQ3LevelMesh*)smgr->getMesh(levelName.c_str());
	if (!mesh) return false;
	SceneMesh = mesh->getMesh(0);

	//scene::ISceneNode* node = smgr->addMeshSceneNode(mesh->getMesh(0));
	//scene::ISceneNode* node = smgr->addOctreeSceneNode(mesh->getMesh(0), 0, -1, 1024);
	SceneNode = smgr->addOctreeSceneNode(mesh->getMesh(0), 0, -1, 1024);
	//node->setScale(vector3df(0.025f, 0.025f, 0.025f)); //breaks octree

	auto shape = new IBvhTriangleMeshShape(SceneNode, mesh->getMesh(0), 0);
	collision::world->addRigidBody(shape);

	ProcessNodes();
	return true;
}

bool LoadLevel(std::string levelName)
{
	if (levelName.find(".bsp") != std::string::npos)
	{
		LoadLevelBsp7(levelName);
	}
	return LoadLevelObj(levelName);
}
