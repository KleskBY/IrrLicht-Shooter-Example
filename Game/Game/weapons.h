#pragma once
#include "main.h"
#include "CWeapon.h"

namespace weapons
{
#define MAX_WEAPONS 10
	extern CWeapon* ActiveWeapon;
	extern CWeapon* Weapons[MAX_WEAPONS];


	bool SelectWeapon(int index);
	bool AddWeapon(CWeapon* weapon);
	void UpdateWeapon();

}