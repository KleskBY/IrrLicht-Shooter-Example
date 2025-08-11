#pragma once
#include "main.h"

namespace collision
{
	extern irrBulletWorld* world;
	void InitCollision();
	void UpdateCollision();
	void AddCollisionForMesh(ISceneNode* n, IMesh* collMesh);
	void DebugCollisions();
};