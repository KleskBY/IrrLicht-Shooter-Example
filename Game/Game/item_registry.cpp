#include "item_registry.h"
#include "items.h"


    std::map<std::string, ItemFactory>& ItemRegistry::GetRegistry() 
    {
        static std::map<std::string, ItemFactory> registry;
        return registry;
    }

    void ItemRegistry::Register(const std::string& name, ItemFactory factory)
    {
        GetRegistry()[name] = factory;
    }

    CItem* ItemRegistry::Create(const std::string& name)
    {
        auto it = GetRegistry().find(name);
        if (it != GetRegistry().end()) 
        {
            return it->second();
        }
        return nullptr;
    }
