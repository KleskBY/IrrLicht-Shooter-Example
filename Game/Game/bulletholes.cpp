#include "bulletholes.h"

#define MAX_BULLET_HOLES 100
int currentBulletHoleNumber = 0;
IMeshSceneNode* bulletHoles[MAX_BULLET_HOLES];
ITexture* bulletHoleTexture;
IAnimatedMesh* bulletMesh;
void AddBulletHole(vector3df pos, vector3df normal)
{
	if (!bulletHoleTexture) bulletHoleTexture = driver->getTexture("data/gfx/hole.png");
	if (!bulletMesh) bulletMesh = smgr->getMesh("data/models/plane.obj");

	vector3df forward = -normal; // Opposite direction of the normal
	vector3df up(0, 1, 0); // Assuming Y is up in your coordinate system
	if (forward.equals(up) || forward.equals(-up)) 
	{
		up.set(1, 0, 0); // Use a different up vector if forward is parallel to the default up vector
	}
	vector3df right = up.crossProduct(forward).normalize();
	up = forward.crossProduct(right).normalize();

	// Create the rotation matrix
	core::matrix4 rotationMatrix;
	rotationMatrix[0] = right.X;
	rotationMatrix[1] = right.Y;
	rotationMatrix[2] = right.Z;
	rotationMatrix[4] = up.X;
	rotationMatrix[5] = up.Y;
	rotationMatrix[6] = up.Z;
	rotationMatrix[8] = forward.X;
	rotationMatrix[9] = forward.Y;
	rotationMatrix[10] = forward.Z;

	// Convert the rotation matrix to Euler angles
	core::vector3df rot  = rotationMatrix.getRotationDegrees();

	if (!bulletHoles[currentBulletHoleNumber])
	{
		bulletHoles[currentBulletHoleNumber] = smgr->addMeshSceneNode(bulletMesh, 0, -1, pos, rot, vector3df(0.025f, 0.025f, 0.025f));//smgr->addCubeSceneNode(0.05f, 0, -1, pos, rot);
		bulletHoles[currentBulletHoleNumber]->setMaterialTexture(0, bulletHoleTexture);
		bulletHoles[currentBulletHoleNumber]->setMaterialType(video::EMT_TRANSPARENT_ALPHA_CHANNEL);
		bulletHoles[currentBulletHoleNumber]->setMaterialFlag(video::EMF_LIGHTING, true);
		bulletHoles[currentBulletHoleNumber]->setMaterialFlag(video::EMF_BILINEAR_FILTER, false);
		bulletHoles[currentBulletHoleNumber]->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
	}
	else
	{
		bulletHoles[currentBulletHoleNumber]->setPosition(pos);
		bulletHoles[currentBulletHoleNumber]->setRotation(rot);
	}

	currentBulletHoleNumber = currentBulletHoleNumber + 1;
	if (currentBulletHoleNumber > MAX_BULLET_HOLES - 1) currentBulletHoleNumber = 0;


}