#include "items.h"
#include "entity.h"
#include "weapons.h"
#include "ak74.h"

CEntity* SpawnItem(vector3df pos, vector3df angles)
{
	CEntity* e = new CEntity();
	e->position = pos;
	e->rotation = angles;
	e->name = "Item";
	e->classname = "Item";
	return e;
}


void WeaponTouch(CEntity* self)
{
	CWeapon* ak74 = new WEAPON_AK74();
	weapons::AddWeapon(ak74);
	self->Remove();
}

void Item_AK74()
{
	CEntity* item = SpawnItem();
	item->model = "models/weapons/w_ak.md3";
	item->scale = vector3df(0.025, 0.025, 0.025);
	item->touch = WeaponTouch;
}
