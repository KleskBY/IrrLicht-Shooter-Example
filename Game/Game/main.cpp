#define NOMINMAX
#undef BT_DEBUG

//#define GAME_DEBUG

#include <iostream>
#include "main.h"
#include "input.h"
#include "player.h"
#include "sound.h"
#include "scene.h"
#include "entity.h"
#include "ak74.h"
#include "weapons.h"
#include "collision.h"

using namespace irr;
using namespace core;
using namespace video;
using namespace scene;
using namespace gui;
using namespace io;
using namespace std;
using namespace irr::scene::quake3;

IrrlichtDevice* device;
video::IVideoDriver* driver;
float DeltaTime;
float Time;
scene::ISceneCollisionManager* collMan;
scene::IMetaTriangleSelector* meta;
scene::ISceneManager* smgr;
gui::IGUIEnvironment* guienv;
scene::ICameraSceneNode* MainCamera;


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
	driver->setTextureCreationFlag(video::ETCF_ALWAYS_32_BIT, true);
	smgr = device->getSceneManager();
	guienv = device->getGUIEnvironment();
	collMan = smgr->getSceneCollisionManager();
	meta = smgr->createMetaTriangleSelector();

	collision::InitCollision();
	sound::InitSoundSystem();

	smgr->getFileSystem()->addFileArchive("data");

	//LoadLevel("level1");
	LoadLevel("data/maps/e1m1.bsp");

	smgr->setAmbientLight(video::SColorf(0.1f, 0.1f, 0.1f));
	smgr->setShadowColor(video::SColor(150, 0, 0, 0));
	driver->setFog(video::SColor(0, 125, 125, 138), video::EFT_FOG_LINEAR, 5.f, 100.f, .003f, true, false);
	smgr->getParameters()->setAttribute(scene::ALLOW_ZWRITE_ON_TRANSPARENT, true);

	MainCamera = smgr->addCameraSceneNode(0, player::CameraPos);
	MainCamera->setNearValue(0.01f);
	MainCamera->setFarValue(999.f);
	MainCamera->bindTargetAndRotation(true);

#ifdef GAME_DEBUG
	scene::IBillboardSceneNode* bill = smgr->addBillboardSceneNode();
	bill->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
	bill->setMaterialTexture(0, driver->getTexture("data/particle.bmp"));
	bill->setMaterialFlag(video::EMF_LIGHTING, false);
	bill->setMaterialFlag(video::EMF_ZBUFFER, true);
	bill->setSize(core::dimension2d<f32>(0.20f, 0.20f));
	bill->setID(ID_IsNotPickable);

	video::SMaterial material;
	material.setTexture(0, 0);
	material.Lighting = false;
	material.ZBuffer = false;
	material.Wireframe = true;
#endif

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

				UpdateEntities();
				collision::UpdateCollision();


#ifdef GAME_DEBUG

				world->debugDrawWorld(true);
				world->debugDrawProperties(true);

				driver->setTransform(video::ETS_WORLD, core::matrix4());
				driver->setMaterial(material);
				driver->draw3DBox(player::PlayerCollider);

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

#endif // GAME_DEBUG

				driver->endScene();
			}

			core::stringw str(L"FPS: ");
			str.append(core::stringw(driver->getFPS()));
			str += L" Tris: ";
			str.append(core::stringw(driver->getPrimitiveCountDrawn()));
			fpstext->setText(str.c_str());
			fpstext->setOverrideColor(SColor(255, 255, 0, 0));
		}
	}

	device->drop();
	return 0;
}














class MyEventReceiver : public IEventReceiver
{
public:

	MyEventReceiver(scene::ISceneNode* room, scene::ISceneNode* earth, gui::IGUIEnvironment* env, video::IVideoDriver* driver)
	{
		Room = room;
		Earth = earth;
		Driver = driver;

		// add window and listbox
		gui::IGUIWindow* window = env->addWindow(core::rect<s32>(460, 375, 630, 470), false, L"Use 'E' + 'R' to change");
		ListBox = env->addListBox(core::rect<s32>(2, 22, 165, 88), window);
		ListBox->addItem(L"Diffuse");
		ListBox->addItem(L"Bump mapping");
		ListBox->addItem(L"Parallax mapping");
		ListBox->setSelected(1);

		// set start material (prefer parallax mapping if available)
		video::IMaterialRenderer* renderer = Driver->getMaterialRenderer(video::EMT_PARALLAX_MAP_SOLID);
		if (renderer && renderer->getRenderCapability() == 0) ListBox->setSelected(2);
		setMaterial();
	}

	bool OnEvent(const SEvent& event)
	{
		// check if user presses the key 'E' or 'R'
		if (event.EventType == irr::EET_KEY_INPUT_EVENT && !event.KeyInput.PressedDown && Room && ListBox)
		{
			// change selected item in listbox

			int sel = ListBox->getSelected();
			if (event.KeyInput.Key == irr::KEY_KEY_R)
				++sel;
			else
				if (event.KeyInput.Key == irr::KEY_KEY_E)
					--sel;
				else
					return false;

			if (sel > 2) sel = 0;
			if (sel < 0) sel = 2;
			ListBox->setSelected(sel);

			// set the material which is selected in the listbox
			setMaterial();
		}

		return false;
	}

private:

	// sets the material of the room mesh the the one set in the
	// list box.
	void setMaterial()
	{
		if (!Room) return;
		video::E_MATERIAL_TYPE type = video::EMT_SOLID;

		// change material setting
		switch (ListBox->getSelected())
		{
		case 0: type = video::EMT_SOLID;
			break;
		case 1: type = video::EMT_NORMAL_MAP_SOLID;
			break;
		case 2: type = video::EMT_PARALLAX_MAP_SOLID;
			break;
		}

		Room->setMaterialType(type);

		// change material setting
		switch (ListBox->getSelected())
		{
		case 0: type = video::EMT_TRANSPARENT_VERTEX_ALPHA;
			break;
		case 1: type = video::EMT_NORMAL_MAP_TRANSPARENT_VERTEX_ALPHA;
			break;
		case 2: type = video::EMT_PARALLAX_MAP_TRANSPARENT_VERTEX_ALPHA;
			break;
		}

		Earth->setMaterialType(type);

		/*
		We need to add a warning if the materials will not be able to
		be displayed 100% correctly. This is no problem, they will be
		rendered using fall back materials, but at least the user
		should know that it would look better on better hardware. We
		simply check if the material renderer is able to draw at full
		quality on the current hardware. The
		IMaterialRenderer::getRenderCapability() returns 0 if this is
		the case.
		*/
		video::IMaterialRenderer* renderer = Driver->getMaterialRenderer(type);

		// display some problem text when problem
		if (!renderer || renderer->getRenderCapability() != 0)
			ProblemText->setVisible(true);
		else
			ProblemText->setVisible(false);
	}

private:

	gui::IGUIStaticText* ProblemText;
	gui::IGUIListBox* ListBox;

	scene::ISceneNode* Room;
	scene::ISceneNode* Earth;
	video::IVideoDriver* Driver;
};


/*
Now for the real fun. We create an Irrlicht Device and start to setup the scene.
*/
int main3()
{
	video::E_DRIVER_TYPE driverType = EDT_OPENGL;
	IrrlichtDevice* device = createDevice(driverType, core::dimension2d<u32>(640, 480));
	if (device == 0) return 1; // could not create selected driver.

	/*
	Before we start with the interesting stuff, we do some simple things:
	Store pointers to the most important parts of the engine (video driver,
	scene manager, gui environment) to safe us from typing too much, add an
	irrlicht engine logo to the window and a user controlled first person
	shooter style camera. Also, we let the engine know that it should store
	all textures in 32 bit. This necessary because for parallax mapping, we
	need 32 bit textures.
	*/

	video::IVideoDriver* driver = device->getVideoDriver();
	scene::ISceneManager* smgr = device->getSceneManager();
	gui::IGUIEnvironment* env = device->getGUIEnvironment();
	driver->setTextureCreationFlag(video::ETCF_ALWAYS_32_BIT, true);


	// add camera
	scene::ICameraSceneNode* camera = smgr->addCameraSceneNodeFPS();
	camera->setPosition(core::vector3df(-200, 200, -200));
	camera->setNearValue(0.01f);
	camera->setFarValue(9999.f);

	// disable mouse cursor
	device->getCursorControl()->setVisible(false);
	driver->setFog(video::SColor(0, 138, 125, 81), video::EFT_FOG_LINEAR, 250, 1000, .003f, true, false);
	scene::IAnimatedMesh* roomMesh = smgr->getMesh("data/maps/level1/1.obj");
	scene::ISceneNode* room = 0;
	if (roomMesh)
	{
		// The Room mesh doesn't have proper Texture Mapping on the
		// floor, so we can recreate them on runtime
		//smgr->getMeshManipulator()->makePlanarTextureMapping(
		//	roomMesh->getMesh(0), 0.003f);

		/*
		Now for the first exciting thing: If we successfully loaded the
		mesh we need to apply textures to it. Because we want this room
		to be displayed with a very cool material, we have to do a
		little bit more than just set the textures. Instead of only
		loading a color map as usual, we also load a height map which
		is simply a grayscale texture. From this height map, we create
		a normal map which we will set as second texture of the room.
		If you already have a normal map, you could directly set it,
		but I simply didn't find a nice normal map for this texture.
		The normal map texture is being generated by the
		makeNormalMapTexture method of the VideoDriver. The second
		parameter specifies the height of the heightmap. If you set it
		to a bigger value, the map will look more rocky.
		*/

		video::ITexture* normalMap =
			driver->getTexture("../../media/rockwall_height.bmp");

		if (normalMap)
			driver->makeNormalMapTexture(normalMap, 9.0f);
		/*
				// The Normal Map and the displacement map/height map in the alpha channel
				video::ITexture* normalMap =
					driver->getTexture("../../media/rockwall_NRM.tga");
		*/
		/*
		But just setting color and normal map is not everything. The
		material we want to use needs some additional informations per
		vertex like tangents and binormals. Because we are too lazy to
		calculate that information now, we let Irrlicht do this for us.
		That's why we call IMeshManipulator::createMeshWithTangents().
		It creates a mesh copy with tangents and binormals from another
		mesh. After we've done that, we simply create a standard
		mesh scene node with this mesh copy, set color and normal map
		and adjust some other material settings. Note that we set
		EMF_FOG_ENABLE to true to enable fog in the room.
		*/

		scene::IMesh* tangentMesh = smgr->getMeshManipulator()->
			createMeshWithTangents(roomMesh->getMesh(0));

		room = smgr->addMeshSceneNode(tangentMesh);

		//room->setDebugDataVisible(EDS_BBOX | EDS_MESH_WIRE_OVERLAY);
		//room->setMaterialTexture(0,
		//	driver->getTexture("../../media/rockwall.jpg"));
		//room->setMaterialTexture(1, normalMap);

		// Stones don't glitter..
		//room->getMaterial(0).SpecularColor.set(0, 0, 0, 0);
		//room->getMaterial(0).Shininess = 0.f;

		//room->setMaterialFlag(video::EMF_FOG_ENABLE, true);
		//room->setMaterialType(video::EMT_PARALLAX_MAP_SOLID);
		// adjust height for parallax effect
		//room->getMaterial(0).MaterialTypeParam = 1.f / 64.f;

		// drop mesh because we created it with a create.. call.
		tangentMesh->drop();
	}
	else std::cout << "ERROR LOADING MESH" << std::endl;
	/*
	After we've created a room shaded by per pixel lighting, we add a
	sphere into it with the same material, but we'll make it transparent.
	In addition, because the sphere looks somehow like a familiar planet,
	we make it rotate. The procedure is similar as before. The difference
	is that we are loading the mesh from an .x file which already contains
	a color map so we do not need to load it manually. But the sphere is a
	little bit too small for our needs, so we scale it by the factor 50.
	*/

	// add earth sphere


	/*
	Per pixel lighted materials only look cool when there are moving
	lights. So we add some. And because moving lights alone are so boring,
	we add billboards to them, and a whole particle system to one of them.
	We start with the first light which is red and has only the billboard
	attached.
	*/

	// add light 1 (more green)
	scene::ILightSceneNode* light1 =
		smgr->addLightSceneNode(0, core::vector3df(0, 0, 0),
			video::SColorf(0.5f, 1.0f, 0.5f, 0.0f), 800.0f);

	light1->setDebugDataVisible(scene::EDS_BBOX);


	// add fly circle animator to light 1
	scene::ISceneNodeAnimator* anim =
		smgr->createFlyCircleAnimator(core::vector3df(50, 300, 0), 190.0f, -0.003f);
	light1->addAnimator(anim);
	anim->drop();

	// attach billboard to the light
	scene::IBillboardSceneNode* bill =
		smgr->addBillboardSceneNode(light1, core::dimension2d<f32>(60, 60));

	bill->setMaterialFlag(video::EMF_LIGHTING, false);
	bill->setMaterialFlag(video::EMF_ZWRITE_ENABLE, false);
	bill->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
	bill->setMaterialTexture(0, driver->getTexture("../../media/particlegreen.jpg"));

	/*
	Now the same again, with the second light. The difference is that we
	add a particle system to it too. And because the light moves, the
	particles of the particlesystem will follow. If you want to know more
	about how particle systems are created in Irrlicht, take a look at the
	specialFx example. Maybe you will have noticed that we only add 2
	lights, this has a simple reason: The low end version of this material
	was written in ps1.1 and vs1.1, which doesn't allow more lights. You
	could add a third light to the scene, but it won't be used to shade the
	walls. But of course, this will change in future versions of Irrlicht
	where higher versions of pixel/vertex shaders will be implemented too.
	*/

	// add light 2 (red)
	scene::ISceneNode* light2 =
		smgr->addLightSceneNode(0, core::vector3df(0, 0, 0),
			video::SColorf(1.0f, 0.2f, 0.2f, 0.0f), 800.0f);

	// add fly circle animator to light 2
	anim = smgr->createFlyCircleAnimator(core::vector3df(0, 150, 0), 200.0f,
		0.001f, core::vector3df(0.2f, 0.9f, 0.f));
	light2->addAnimator(anim);
	anim->drop();

	// attach billboard to light
	bill = smgr->addBillboardSceneNode(light2, core::dimension2d<f32>(120, 120));
	bill->setMaterialFlag(video::EMF_LIGHTING, false);
	bill->setMaterialFlag(video::EMF_ZWRITE_ENABLE, false);
	bill->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
	bill->setMaterialTexture(0, driver->getTexture("../../media/particlered.bmp"));

	// add particle system
	scene::IParticleSystemSceneNode* ps =
		smgr->addParticleSystemSceneNode(false, light2);

	// create and set emitter
	scene::IParticleEmitter* em = ps->createBoxEmitter(
		core::aabbox3d<f32>(-3, 0, -3, 3, 1, 3),
		core::vector3df(0.0f, 0.03f, 0.0f),
		80, 100,
		video::SColor(10, 255, 255, 255), video::SColor(10, 255, 255, 255),
		400, 1100);
	em->setMinStartSize(core::dimension2d<f32>(30.0f, 40.0f));
	em->setMaxStartSize(core::dimension2d<f32>(30.0f, 40.0f));

	ps->setEmitter(em);
	em->drop();

	// create and set affector
	scene::IParticleAffector* paf = ps->createFadeOutParticleAffector();
	ps->addAffector(paf);
	paf->drop();

	// adjust some material settings
	ps->setMaterialFlag(video::EMF_LIGHTING, false);
	ps->setMaterialFlag(video::EMF_ZWRITE_ENABLE, false);
	ps->setMaterialTexture(0, driver->getTexture("../../media/fireball.bmp"));
	ps->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);

	MyEventReceiver receiver(room, 0, env, driver);
	device->setEventReceiver(&receiver);

	/*
	Finally, draw everything. That's it.
	*/

	int lastFPS = -1;

	while (device->run())
		if (device->isWindowActive())
		{
			driver->beginScene(true, true, 0);

			smgr->drawAll();
			env->drawAll();

			driver->endScene();

			int fps = driver->getFPS();

			if (lastFPS != fps)
			{
				core::stringw str = L"Per pixel lighting example - Irrlicht Engine [";
				str += driver->getName();
				str += "] FPS:";
				str += fps;

				device->setWindowCaption(str.c_str());
				lastFPS = fps;
			}
		}

	device->drop();

	return 0;
}
