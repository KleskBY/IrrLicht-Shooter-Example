#pragma once
#include "string"
#include "vector"
#include <memory>
#include <iostream>

//struct INVENTORY_ITEM
//{
//	int id;
//	std::string name;
//	std::string type;
//	int amount;
//};
//
//
//class Item {
//public:
//	int id;
//	std::string name;
//	std::string type;
//	int amount;
//	virtual ~Item() = default;
//	virtual std::string GetName() const = 0;
//	virtual void Use() = 0;
//};
//

#define INVENTORY_MAX_ITEMS 100

enum INVENTORY_ITEM_TYPE
{
	ITEM_TYPE_NONE,
	ITEM_TYPE_WEAPON,
	ITEM_TYPE_ARMOR,
	ITEM_TYPE_AID,
	ITEM_TYPE_MISC,
	ITEM_TYPE_COUNT
};

struct INVENTORY_ITEM
{
	int id;
	std::string name;
	INVENTORY_ITEM_TYPE type;
	int amount;
	float mass;
};


//INVENTORY_ITEM Items[INVENTORY_MAX_ITEMS];
std::vector<INVENTORY_ITEM> Items;
float MaxMass = 100.f;
float CurrentMass = 0.f;
float GetTotalMass()
{
	float mass = 0.f;
	for (int i = 0; i < Items.size(); i++)
	{
		mass = mass + Items[i].mass;
	}
	CurrentMass = mass;
	return mass;
}


bool AddItem(INVENTORY_ITEM item)
{
	for (int i = 0; i < Items.size(); i++)
	{
		if (GetTotalMass() > MaxMass)
		{
			printf("Cant carry more\n");
			return false;
		}

		if (Items[i].name == item.name)
		{
			//if (Items[i].type == ITEM_TYPE_WEAPON)
			//{

			//}
			//else
			{
				Items.push_back(item);
			}
		}
	}
}





class Item {
public:
	virtual ~Item() = default;
	virtual std::string GetName() const = 0;
	virtual void Use() = 0;
	float mass = 0.f;
	int type = 0;
};

// Inventory class
class Inventory {
public:
	void AddItem(std::unique_ptr<Item> item) 
	{
		items.push_back(std::move(item));
		std::cout << item->GetName() << " added to inventory.\n";
	}

	void RemoveItem(const std::string& itemName) 
	{
		auto it = std::find_if(items.begin(), items.end(),
			[&itemName](const std::unique_ptr<Item>& item) {
				return item->GetName() == itemName;
			});

		if (it != items.end()) 
		{
			std::cout << (*it)->GetName() << " removed from inventory.\n";
			items.erase(it);
		}
		else 
		{
			std::cout << itemName << " not found in inventory.\n";
		}
	}

	void UseItem(const std::string& itemName) 
	{
		auto it = std::find_if(items.begin(), items.end(),
			[&itemName](const std::unique_ptr<Item>& item) {
				return item->GetName() == itemName;
			});

		if (it != items.end()) 
		{
			(*it)->Use();
		}
		else 
		{
			std::cout << itemName << " not found in inventory.\n";
		}
	}

	void DisplayItems() const 
	{
		if (items.empty()) 
		{
			std::cout << "Inventory is empty.\n";
			return;
		}

		std::cout << "Inventory:\n";
		for (const auto& item : items) 
		{
			std::cout << "- " << item->GetName() << "\n";
		}
	}

	float GetMass()
	{
		float mass = 0;
		for (const auto& item : items)
		{
			mass = mass + item->mass;
		}
		return mass;
	}

private:
	std::vector<std::unique_ptr<Item>> items;
};