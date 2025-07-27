#include "weapons.h"

namespace weapons
{
	CWeapon* ActiveWeapon;
	CWeapon* Weapons[MAX_WEAPONS];

	bool changingWeapon = false;
	int nextWeaponIndex = 0;
	float chaningWeaponEnd = 0;
	float chaningWeaponTime = 0;


	int GetCurrentWeaponIndex()
	{
		for (int i = 0; i < MAX_WEAPONS; i++)
		{
			if (Weapons[i] == ActiveWeapon) return i;
		}
		return 0;
	}

	bool SelectWeapon(int index)
	{
		if (Weapons[index])
		{
			ActiveWeapon = Weapons[index];
			return true;
		}
		printf("No such weapon\n");
		return false;
	}

	bool SwitchWeapon(int index)
	{
		if (Weapons[index])
		{
			ActiveWeapon->Hide();
			changingWeapon = true;
			nextWeaponIndex = index;
			chaningWeaponEnd = Time + 0.3f;
			return true;
		}
		printf("No such weapon\n");
		return false;
	}

	void CycleWeapon()
	{
		int currentIndex = GetCurrentWeaponIndex();
		for (int i = currentIndex + 1; i < MAX_WEAPONS + currentIndex; i++)
		{
			int index = i;
			if (index > MAX_WEAPONS -1) index = 1;
			if (index < 1) index = 9;
			std::cout << index << std::endl;

			if (SwitchWeapon(index)) return;
		}
		//int index = GetCurrentWeaponIndex() + 1;
		//if (index > MAX_WEAPONS) index = 1;
		//if (index < 1) index = 9;
		//SelectWeapon(index);
	}

	void CycleWeaponReverse()
	{
		int index = GetCurrentWeaponIndex() - 1;
		if (index > MAX_WEAPONS) index = 1;
		if (index < 1) index = 9;
		SelectWeapon(index);
	}


	bool AddWeapon(CWeapon* weapon)
	{
		for (int i = 1; i < MAX_WEAPONS; i++)
		{
			if (!Weapons[i])
			{
				Weapons[i] = weapon;
				SelectWeapon(i);
				return true;
			}
		}
		return false;
	}


	void UpdateWeapon()
	{
		if (ActiveWeapon)
		{
			if (changingWeapon)
			{
				if (Time > chaningWeaponEnd)
				{
					SelectWeapon(nextWeaponIndex);
					if (ActiveWeapon)
					{
						ActiveWeapon->Get();
						nextWeaponIndex = 0;
						changingWeapon = false;
					}
				}
				return;
			}
		}

		int index = input::GetWeaponSelectInput();
		if (index > 0)
		{
			if (Weapons[index] == ActiveWeapon)
			{
				printf("Already selected.\n");
				return;
			}

			SwitchWeapon(index);
			//if (Weapons[index])
			//{
			//	ActiveWeapon->Hide();
			//	changingWeapon = true;
			//	nextWeaponIndex = index;
			//	chaningWeaponEnd = Time + 0.3f;
			//}
		}

		if (input::GetMouseScrollInput() > 0)
		{
			CycleWeapon();
		}
	}
}