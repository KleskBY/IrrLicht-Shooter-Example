#define NOMINMAX
#undef BT_DEBUG

#include <iostream>
#include "main.h"
#include "input.h"
#include "player.h"
#include "sound.h"
#include "scene.h"
#include "entity.h"
#include "ak74.h"
#include "weapons.h"

using namespace irr;
using namespace core;
using namespace video;
using namespace scene;
using namespace gui;
using namespace io;
using namespace std;

IrrlichtDevice* device;
video::IVideoDriver* driver;
float DeltaTime;
float Time;
scene::ISceneCollisionManager* collMan;
scene::IMetaTriangleSelector* meta;
scene::ISceneManager* smgr;
gui::IGUIEnvironment* guienv;
scene::ICameraSceneNode* MainCamera;

irrBulletWorld* world;


void handleCollisions()
{
	//printf("%i\n", world->getNumManifolds());
	for (int i = 0; i < world->getNumManifolds(); i++)
	{
		auto info = world->getCollisionCallback(i);

		//bool hasCollideAttribute = (info->getBody0()->getAttributes()->existsAttribute("collide"));
		//auto obj = (hasCollideAttribute) ? info->getBody0() : info->getBody1();
	
		//world->addToDeletionQueue(obj);
		//if (obj->getAttributes()->existsAttribute("collide") && obj->getAttributes()->getAttributeAsBool("collide") == true)
		//{
		//	obj->getAttributes()->setAttribute("collide", false);
		//	world->addToDeletionQueue(obj);
		//}

		//obj = (hasCollideAttribute) ? info->getBody1() : info->getBody0();
		//if (obj->getAttributes()->existsAttribute("vehicle"))
		//{
		//	if (obj->getAttributes()->existsAttribute("destroyable"))
		//	{
		//		if (obj->getAttributes()->getAttributeAsBool("isDestroyed") == false)
		//		{
		//			obj->getAttributes()->setAttribute("isDestroyed", true);


		//			// Since we set a reference to the raycast vehicle of each tank, it will be removed automatically
		//			// by irrBullet when the object is removed!
		//			world->removeCollisionObject(obj);
		//		}
		//	}
		//}
	}

	// If you wanted to get all contact points, you could do something like this:
	/*
	for(int i=0; i < DynamicsWorld->getNumManifolds(); i++)
	{
		ICollisionCallbackInformation *info = DynamicsWorld->getCollisionCallback(i);

		int numContacts = info->getPointer()->getNumContacts();
		for(int j=0; j < numContacts; j++)
		{
			if(verifyCollisionCallback(info))
			{
				if(info->getContactPoint(j).getDistance()<1.5f && info->getContactPoint(j).getLifeTime() < 2.0f)
				{
					// Handle contact point
				}
			}
		}
		info->getPointer()->clearManifold();
	}
	*/
}

void WeaponTouch(CEntity* self)
{
	CWeapon* ak74 = new WEAPON_AK74();
	weapons::AddWeapon(ak74);
	self->Remove();
}

int main(int argc, char* argv[])
{
	bool fullscreen = false;
	bool music = true;
	bool shadows = true;
	bool additive = false;
	bool vsync = true;
	bool aa = true;

	irr::core::dimension2d<u32> resolution(1280, 720);
	irr::SIrrlichtCreationParameters params;
	params.DriverType = irr::video::EDT_OPENGL; // irr::video::EDT_DIRECT3D9;
	params.WindowSize = resolution;
	params.Bits = 32;
	params.Fullscreen = fullscreen;
	params.Stencilbuffer = shadows;
	params.Vsync = vsync;
	params.AntiAlias = aa ? 8 : 0;
	
	device = createDeviceEx(params);
	if (!device) return 1;
	device->setWindowCaption(L"Game");
	device->setEventReceiver(&InputRecevier);
	device->getCursorControl()->setVisible(false);

	driver = device->getVideoDriver();
	smgr = device->getSceneManager();
	guienv = device->getGUIEnvironment();
	collMan = smgr->getSceneCollisionManager();
	meta = smgr->createMetaTriangleSelector();

	world = createIrrBulletWorld(device, true, true);
	world->setDebugMode(irrPhysicsDebugMode::EPDM_DrawAabb | irrPhysicsDebugMode::EPDM_DrawContactPoints);
	world->setGravity(core::vector3df(0, -10.f, 0));
	world->setPropertiesTextPosition(EDPT_POSITION::EDPT_TOP_RIGHT);

	sound::InitSoundSystem();

	smgr->getFileSystem()->addFileArchive("data");

	LoadLevel("level1");
	CEntity* Lol = new CEntity();
	Lol->model = "models/weapons/w_ak.md3";
	Lol->scale = vector3df(0.025, 0.025, 0.025);
	Lol->position = vector3df(0, 1, 0);
	Lol->touch = WeaponTouch;


	smgr->setAmbientLight(video::SColorf(0.8f, 0.8f, 0.8f));
	smgr->setShadowColor(video::SColor(150, 0, 0, 0));

	MainCamera = smgr->addCameraSceneNode();
	MainCamera->setNearValue(0.01f);
	MainCamera->setFarValue(999.f);
	MainCamera->bindTargetAndRotation(true);

	scene::IBillboardSceneNode* bill = smgr->addBillboardSceneNode();
	bill->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
	bill->setMaterialTexture(0, driver->getTexture("data/particle.bmp"));
	bill->setMaterialFlag(video::EMF_LIGHTING, false);
	bill->setMaterialFlag(video::EMF_ZBUFFER, true);
	bill->setSize(core::dimension2d<f32>(2.0f, 2.0f));
	bill->setID(ID_IsNotPickable);

	video::SMaterial material;
	material.setTexture(0, 0);
	material.Lighting = false;
	material.ZBuffer = false;
	material.Wireframe = true;

	IGUIStaticText* fpstext = guienv->addStaticText(L"", core::rect<s32>(0, 0, 200, 24), false, false);



	float lastFrameTime = 0;
	while (driver && device->run())
	{
		if (device->isWindowActive())
		{
			Time = (f32)(device->getTimer()->getTime()) / 1000.f;
			DeltaTime = Time - lastFrameTime;
			lastFrameTime = Time;

			if (DeltaTime > 0.000f && DeltaTime < 1.f)
			{
				player::PlayerUpdate();
				input::ProcessMovement();
				input::MouseLook();
			}

			if (MainCamera) sound::UpdateSoundSystem(MainCamera->getAbsolutePosition(), MainCamera->getTarget());
			if (driver->beginScene(true, true))
			{
				smgr->drawAll();
				guienv->drawAll();

				core::line3d<f32> ray;
				ray.start = MainCamera->getPosition();
				ray.end = ray.start + (MainCamera->getTarget() - ray.start).normalize() * 1000.0f;
				core::vector3df intersection; // Tracks the current intersection point with the level or a mesh
				core::triangle3df hitTriangle; // Used to show with triangle has been hit
				scene::ISceneNode* selectedSceneNode = collMan->getSceneNodeAndCollisionPointFromRay(ray, intersection, hitTriangle, IDFlag_IsPickable, 0); 
				if (selectedSceneNode)
				{
					bill->setPosition(intersection);

					driver->setTransform(video::ETS_WORLD, core::matrix4());
					driver->setMaterial(material);
					driver->draw3DTriangle(hitTriangle, video::SColor(0, 255, 0, 0));
				}

				UpdateEntities();
				UpdateEntitiesCollision();

				world->stepSimulation(DeltaTime);
				handleCollisions();
				//world->debugDrawWorld(true);
				//world->debugDrawProperties(true);

				driver->setTransform(video::ETS_WORLD, core::matrix4());
				driver->setMaterial(material);
				driver->draw3DBox(player::PlayerCollider);

				core::stringw str(L"FPS: ");
				str.append(core::stringw(driver->getFPS()));
				str += L" Tris: ";
				str.append(core::stringw(driver->getPrimitiveCountDrawn()));
				fpstext->setText(str.c_str());
				fpstext->setOverrideColor(SColor(255, 255, 0, 0));

				driver->endScene();
			}
		}
	}

	device->drop();
	return 0;
}