#include "LoadOBJ.h"
#include "scene.h"
#include "collision.h"
#include "materials.h"

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
}