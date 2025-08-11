#include "main.h"
#include "scene.h"
#include "LoadBSP.h"
#include "LoadOBJ.h"

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


bool LoadLevel(std::string levelName)
{
	bool ret = false;
	if (levelName.find(".bsp") != std::string::npos)
	{
		ret = LoadLevelBsp8(levelName);
	}
	else ret = LoadLevelObj(levelName);

	ProcessNodes();
	return ret;
}
