#pragma once
#include "main.h"
#include <vector>
#include <irrlicht.h>

class CEntity; // Forward declaration of CEntity
typedef void (*EntityFunc)(CEntity*);
class CEntity {
public:
	std::string name = "";
	std::string classname = "";
	std::string model = "";
	irr::core::vector3df position = vector3df(0, 0, 0);
	irr::core::vector3df rotation = vector3df(0, 0, 0);
	irr::core::vector3df size = vector3df(1, 1, 1);
	irr::core::vector3df scale = vector3df(1, 1, 1);;
	IAnimatedMeshSceneNode* MeshNode = nullptr;
	IMeshSceneNode* ColliderNode = nullptr;
	ICollisionShape* shape = nullptr;
	aabbox3df BoxCollider;
	bool deleted = false;



	EntityFunc think = nullptr;
	EntityFunc touch = nullptr;

	void CollisionCheck();
	void UpdateEntity();
	void Remove();

	~CEntity();
	CEntity();
private:
	std::string lastModel = "";

};




extern std::vector<CEntity*> EntityList;

void UpdateEntities();
void UpdateEntitiesCollision();