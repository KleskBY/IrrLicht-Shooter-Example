#include "viewmodel.h"
#include "main.h"
#include "player.h"
#include "weapons.h"
#include "scene.h"
#include "materials.h"
#include "lightgrid.h"
#include "dylight.h"

IAnimatedMesh* ViewmodelMesh;
IAnimatedMeshSceneNode* ViewmodelNode;
std::string ViewmodelModel;
std::string lastViewmodelModel;
int ViewmodelFrame = 0;

scene::ILightSceneNode* MuzzleflashLight;
scene::IAnimatedMesh* MuzzleflashMesh;
scene::IAnimatedMeshSceneNode* MuzzleflashNode;
vector3df MuzzleflashPosition(0, 0, 0);

vector3df weaponOffset(0, 0, 0);

vector3df weaponLengthOffset(0, 0, 0);

core::vector3df bobOffset(0, 0, 0);
f32 bobIntensity = 0.1f;
f32 bobFrequency = 9.0f;
void WeaponBob()
{
	bobOffset.Z = sinf(Time * bobFrequency) * bobIntensity * player::PlayerVelocity.getLength();
	bobOffset.Y = sinf(Time * bobFrequency) * bobIntensity * player::PlayerVelocity.getLength();
}


void WeaponLean()
{
	core::vector3df forward = MainCamera->getTarget() - MainCamera->getPosition();
	forward.normalize();

	core::vector3df right = MainCamera->getTarget() - MainCamera->getPosition();
	right.normalize();
	right = right.crossProduct(MainCamera->getUpVector());
}


core::vector3df lastCameraPosition(0, 0, 0);
core::vector3df lastCameraRotation(0, 0, 0);
core::vector3df swayOffset(0,0,0);
void WeaponSway()
{
	core::vector3df forward = MainCamera->getTarget() - MainCamera->getPosition();
	forward.normalize();

	core::vector3df right = MainCamera->getTarget() - MainCamera->getPosition();
	right.normalize();
	right = right.crossProduct(MainCamera->getUpVector());

	core::vector3df cameraRotation = MainCamera->getTarget();
	core::vector3df sway;
	sway.X = (cameraRotation.X - lastCameraRotation.X); 
	sway.Y = (cameraRotation.Y - lastCameraRotation.Y);
	lastCameraRotation = cameraRotation;
	
	swayOffset.X = swayOffset.X + sway.X * DeltaTime * 10.f;
	swayOffset.Y = swayOffset.Y + sway.Y * DeltaTime * 10.f;
	swayOffset = ClampVector(swayOffset, vector3df(-0.1f, -0.1f, 0), vector3df(0.1f, 0.1f, 0));
	//ViewmodelNode->setPosition(lerp(ViewmodelNode->getPosition(), swayOffset, 15.f * DeltaTime));
	swayOffset = lerp(swayOffset, vector3df(), 5 * DeltaTime);
}


void DrawMuzzleflashLight()
{
	if (!MuzzleflashLight)
	{
		MuzzleflashLight = smgr->addLightSceneNode(MainCamera, core::vector3df(0, 0.05f, 0), video::SColor(255, 255, 200, 200), 0.01f);
		MuzzleflashLight->setLightType(irr::video::ELT_POINT);
	}
	MuzzleflashLight->setRadius(MuzzleflashLight->getRadius() - 5.f * DeltaTime);
	if (MuzzleflashLight->getRadius() < 0.f) MuzzleflashLight->setRadius(0.f);

	if (!ViewmodelNode) return;
	if (!MuzzleflashNode)
	{
		MuzzleflashMesh = smgr->getMesh("data/models/weapons/muzzle.md3");
		MuzzleflashNode = smgr->addAnimatedMeshSceneNode(MuzzleflashMesh, MainCamera, -1);
		MuzzleflashNode->setScale(vector3df(0.025f, 0.025f, 0.025f));
		//MuzzleflashNode->setPosition(vector3df(2.65f, -0.40f, -0.22f));
		//MuzzleflashNode->setPosition(vector3df(0.0f, 0.02f, 0.06f));
		MuzzleflashNode->setRotation(vector3df(0.0f, -90.f, 0.0f));
		MuzzleflashNode->setPosition(vector3df(0.1f, -0.15f, 0.99f));

		MuzzleflashNode->setMaterialType(video::EMT_TRANSPARENT_ALPHA_CHANNEL);
		MuzzleflashNode->setMaterialFlag(video::EMF_LIGHTING, false);
		MuzzleflashNode->setMaterialFlag(video::EMF_BILINEAR_FILTER, false);
	}
	else
	{
		MuzzleflashPosition = vector3df(0.1f, -0.15f, 0.99f);
		MuzzleflashNode->setRotation(vector3df(0.0f, -90.f, 0.0f));
		MuzzleflashNode->setPosition(bobOffset + weaponOffset + weaponLengthOffset + swayOffset + MuzzleflashPosition);
		MuzzleflashNode->setScale(lerp(MuzzleflashNode->getScale(), vector3df(0, 0, 0), 25 * DeltaTime));
	}

}

void Muzzleflash()
{
	if(MuzzleflashLight) MuzzleflashLight->setRadius(0.4f);
	if (MuzzleflashNode)
	{
		MuzzleflashNode->setScale(vector3df(0.025f, 0.025f, 0.025f));
		IParticleSystemSceneNode* partSys = createParticleSystem(MuzzleflashNode->getAbsolutePosition(), vector3df(Random(-1, 1), Random(-1, 1), Random(-1, 1)), 2, 8,
			SColor(0, 255, 255, 255), SColor(0, 255, 255, 255),
			500.0f, 500.0f,
			0,
			dimension2df(1.1f, 1.1f), dimension2df(2.2f, 2.2f),
			"data/smoke2.jpg",
			500.0f,
			false,
			EMT_TRANSPARENT_ADD_COLOR,
			true);
	}
}


void WeaponLength()
{
	float max_weapon_length = 0.1f; //1.1f;
	if (weapons::ActiveWeapon) max_weapon_length = weapons::ActiveWeapon->weaponModelLength;
	core::vector3df direction = (MainCamera->getTarget() - MainCamera->getPosition()).normalize();
	core::line3d<f32> ray;
	ray.start = MainCamera->getPosition();
	ray.end = ray.start + direction * max_weapon_length;

	core::vector3df intersection;
	core::triangle3df hitTriangle;

	float targetZ = 0.f;
	scene::ISceneNode* selectedSceneNode = collMan->getSceneNodeAndCollisionPointFromRay(ray, intersection, hitTriangle, IDFlag_IsPickable, 0);
	if (selectedSceneNode)
	{
		float vlen = MainCamera->getPosition().getDistanceFrom(intersection);
		targetZ = 0.f - (max_weapon_length - vlen);
	}

	weaponLengthOffset = lerp(weaponLengthOffset, vector3df(0, 0, targetZ), 20 * DeltaTime);
}

void DrawViewmodel()
{
	if (!MainCamera) return;

	if (ViewmodelModel != lastViewmodelModel)
	{
		ViewmodelMesh = smgr->getMesh(ViewmodelModel.c_str());
		if (!ViewmodelNode)
		{
			ViewmodelNode = smgr->addAnimatedMeshSceneNode(ViewmodelMesh, MainCamera);
			//ViewmodelNode->addShadowVolumeSceneNode();

		}
		else
		{
			ViewmodelNode->setMesh(ViewmodelMesh);
			ViewmodelNode->setMaterialFlag(video::EMF_LIGHTING, true);
			SetDefaultMaterial(ViewmodelNode);
			for (int i = 0; i < ViewmodelNode->getMaterialCount(); i++)
			{
				ViewmodelNode->getMaterial(i).AmbientColor = SColor(0, 0, 0, 0);
			}
		}
	}

	if (!ViewmodelNode) return;

	//LightSample s = GetLightColorAtPoint(MainCamera->getPosition(), SceneMesh);
	//video::SColor color = video::SColor(255.f, s.color.getRed() + s.brightness, s.color.getGreen() + s.brightness, s.color.getBlue() + s.brightness);
	//std::cout << color.getRed() << " " << color.getGreen() << " " << color.getBlue() << std::endl;

	//video::SColor color = GetLightmapColorAtRayHit(MainCamera->getPosition());
	//color = video::SColor(255, color.getRed() * 4, color.getGreen() * 4, color.getBlue() * 4);
	//std::cout << color.getRed() << " " << color.getGreen() << " " << color.getBlue() << std::endl;

	//video::SColor color = GetDynamicLightColorAtPosition(MainCamera->getPosition()).toSColor();
	//std::cout << color.getRed() << " " << color.getGreen() << " " << color.getBlue() << std::endl;
	//for (int i = 0; i < ViewmodelNode->getMaterialCount(); i++)
	//{
	//	ViewmodelNode->getMaterial(i).EmissiveColor = color;
	//	ViewmodelNode->getMaterial(i).AmbientColor = color;
	//}


	WeaponBob();
	//WeaponLean();
	WeaponSway();
	WeaponLength();

	ViewmodelNode->setRotation(vector3df(0, -90, 0));
	ViewmodelNode->setScale(vector3df(0.025f, 0.025f, 0.025f));
	ViewmodelNode->setCurrentFrame(ViewmodelFrame);
	ViewmodelNode->setPosition(bobOffset + weaponOffset + weaponLengthOffset + swayOffset);
	ViewmodelNode->updateAbsolutePosition();
		
	DrawMuzzleflashLight();
}