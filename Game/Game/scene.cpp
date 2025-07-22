#include "scene.h"
#include "materials.h"


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
			//node->setMaterialFlag(video::EMF_LIGHTING, true);
			//node->setMaterialFlag(video::EMF_BILINEAR_FILTER, false);
			//node->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
			break;

		case scene::ESNT_TERRAIN:
			selector = smgr->createTerrainTriangleSelector((scene::ITerrainSceneNode*)node);
			break;

		case scene::ESNT_OCTREE:
			selector = smgr->createOctreeTriangleSelector(((scene::IMeshSceneNode*)node)->getMesh(), node);
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


bool LoadLevel(std::string levelName)
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
			scene::IMesh* mesh = smgr->getMesh(mdlname.c_str());
			scene::IMeshSceneNode* node = smgr->addMeshSceneNode(mesh);

			//The sun (trash)
			//scene::ILightSceneNode* light = smgr->addLightSceneNode(MainCamera, core::vector3df(0, 100, 100), video::SColor(255, 255, 200, 200));
			//light->setLightType(irr::video::ELT_DIRECTIONAL);
			//light->setRotation(core::vector3df(0.025, 45.025, 0.025));

			auto shape = new IBvhTriangleMeshShape(node, node->getMesh(), 0);
			world->addRigidBody(shape);

			SetDefaultMaterial(node);
		}
	}

	ProcessNodes();
}
