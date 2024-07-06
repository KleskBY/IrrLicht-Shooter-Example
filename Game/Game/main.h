#pragma once
#include <irrlicht.h>

#include <windows.h>
#include <stdio.h>
#include <string>



using namespace irr;
using namespace core;
using namespace video;
using namespace scene;
using namespace gui;
using namespace io;
using namespace std;

extern IrrlichtDevice* device;
extern scene::ICameraSceneNode* MainCamera;
extern float DeltaTime;
extern float Time;
extern scene::ISceneCollisionManager* collMan;
extern scene::IMetaTriangleSelector* meta;
extern video::IVideoDriver* driver;
extern scene::ISceneManager* smgr;
extern gui::IGUIEnvironment* guienv;


#include <irrBullet.h>
#include <irrBulletCommon.h>
//IRRBULLET
extern irrBulletWorld* world;

enum
{
	// I use this ISceneNode ID to indicate a scene node that is
	// not pickable by getSceneNodeAndCollisionPointFromRay()
	ID_IsNotPickable = 0,

	// I use this flag in ISceneNode IDs to indicate that the
	// scene node can be picked by ray selection.
	IDFlag_IsPickable = 1 << 0,

	// I use this flag in ISceneNode IDs to indicate that the
	// scene node can be highlighted.  In this example, the
	// homonids can be highlighted, but the level mesh can't.
	IDFlag_IsHighlightable = 1 << 1
};

inline core::vector3df ClampVector(const core::vector3df& value, const core::vector3df& min, const core::vector3df& max)
{
	return core::vector3df(core::clamp(value.X, min.X, max.X), core::clamp(value.Y, min.Y, max.Y), core::clamp(value.Z, min.Z, max.Z));
}

#include <random>
inline float Random(float min, float max)
{
	static std::default_random_engine engine;
	std::uniform_real_distribution<float> distribution(min, max);
	return distribution(engine);
}

inline bool traceline(core::vector3df start, core::vector3df end)
{
	core::line3d<f32> ray;
	ray.start = start;
	ray.end = end;

	core::vector3df intersection;
	core::triangle3df hitTriangle;

	scene::ISceneNode* selectedSceneNode = collMan->getSceneNodeAndCollisionPointFromRay(ray, intersection, hitTriangle, IDFlag_IsPickable, 0);
	if (selectedSceneNode)
	{
		return true;
	}
	return false;
}


//inline irr::scene::IParticleSystemSceneNode* const createParticleSystem(vector3df pos, vector3df dir, vector2df min_max_emit_rate, SColor color, vector2df min_max_age, f32 angle, 

inline irr::scene::IParticleSystemSceneNode* const createParticleSystem(const irr::core::vector3df& pos, const irr::core::vector3df& dir, irr::f32 min, irr::f32 max,
	const irr::video::SColor& darkest, const irr::video::SColor& brightest, irr::f32 minAge, irr::f32 maxAge, irr::f32 angle,
	const irr::core::dimension2df& minSize, const irr::core::dimension2df maxSize, const irr::core::stringc& textureFile,
	irr::f32 lifeTime, bool gravity, irr::video::E_MATERIAL_TYPE materialType, bool fadeOut)
{
	irr::scene::IParticleSystemSceneNode* ps =
		device->getSceneManager()->addParticleSystemSceneNode(false);


	irr::core::aabbox3d<irr::f32> emitter_size = irr::core::aabbox3d<irr::f32>(core::vector3df(-0.25, -0.25, -0.25), core::vector3df(0.25, 0.25, 0.25));
	irr::scene::IParticleEmitter* em = ps->createBoxEmitter(
		emitter_size, // emitter size
		dir,   // initial direction core::vector3df(0.0f,0.01f,0.0f)
		min, max,                             // emit rate
		darkest,       // darkest color
		brightest,       // brightest color
		minAge, maxAge, angle,                         // min and max age, angle
		minSize,         // min size
		maxSize);        // max size

	ps->setEmitter(em); // this grabs the emitter
	em->drop(); // so we can drop it here without deleting it

	if (fadeOut)
	{
		irr::scene::IParticleAffector* paf = ps->createFadeOutParticleAffector();
		ps->addAffector(paf); // same goes for the affector
		paf->drop();
	}

	if (gravity == true)
	{
		irr::scene::IParticleAffector* paf = ps->createGravityAffector(irr::core::vector3df(0.0f, -0.007f, 0.0f), 1800);
		ps->addAffector(paf); // same goes for the affector
		paf->drop();
	}

	ps->setScale(irr::core::vector3df(0.2, 0.2, 0.2));
	ps->setMaterialFlag(irr::video::EMF_LIGHTING, false);
	ps->setMaterialFlag(irr::video::EMF_ZWRITE_ENABLE, false);
	ps->setMaterialTexture(0, device->getVideoDriver()->getTexture(textureFile.c_str()));
	ps->setMaterialType(materialType);

	//ps->setParent(info->getBody0()->getCollisionShape()->getSceneNode());
	ps->setPosition(pos);

	if (lifeTime > 0.0f)
	{
		irr::scene::ISceneNodeAnimator* anim = device->getSceneManager()->createDeleteAnimator(lifeTime);
		ps->addAnimator(anim);
		anim->drop();
	}

	return ps;
}



