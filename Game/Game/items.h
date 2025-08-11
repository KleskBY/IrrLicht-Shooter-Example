#pragma once
#include "entity.h";

#define ITEM_PICKUP_SOUND "data/sound/player/pickup.wav"

CEntity* SpawnItem(vector3df pos = vector3df(0.f, 0.f, 0.f), vector3df angles = vector3df(0.f, 0.f, 0.f));
void Item_pickup(CEntity* self);
void StartItem(CEntity* self);
CEntity* SpawnItem(vector3df pos, vector3df angles);


#include "item_ak74.h"

class CItem {
public:
	virtual ~CItem() {}
	virtual void Spawn(CEntity* ent) = 0; // Called when entity is parsed
};
