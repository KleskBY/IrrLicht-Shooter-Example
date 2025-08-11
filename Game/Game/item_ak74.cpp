#include "item_ak74.h"
#include "items.h"
#include "ak74.h"
#include "weapons.h"
#include "item_registry.h"

void Item_AK74_Touch(CEntity* self)
{
    Item_pickup(self);
	CWeapon* ak74 = new WEAPON_AK74();
	weapons::AddWeapon(ak74);
	self->Remove();
}

class Item_AK74 : public CItem {
public:
    void Spawn(CEntity* ent) override {
        StartItem(ent);
        ent->model = "models/weapons/w_ak.md3";
        ent->scale = vector3df(0.025f, 0.025f, 0.025f);
        ent->touch = Item_AK74_Touch;
    }
};

REGISTER_ITEM("item_health", Item_AK74)