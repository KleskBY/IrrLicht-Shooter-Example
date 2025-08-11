#include "items.h"
#include "entity.h"
#include "weapons.h"
#include "ak74.h"
#include "sound.h"
#include "viewflash.h"


CEntity* SpawnItem(vector3df pos, vector3df angles)
{
	CEntity* e = new CEntity();
	e->position = pos;
	e->rotation = angles;
	e->name = "Item";
	e->classname = "Item";
	return e;
}

void Item_pickup(CEntity* self)
{
	sound::Play2D(ITEM_PICKUP_SOUND, 0.5f);
	TriggerViewFlash(0.6f, 0.6f, 0.1f, 0.3f, 0.4f); // White flash, quick fade

}

void StartItem(CEntity* self)
{
	self->classname = "item";
	self->flags = FL_ITEM;
}


