#pragma once
#include "items.h"
#include <map>
#include <string>

typedef CItem* (*ItemFactory)();
class ItemRegistry {
public:
    static std::map<std::string, ItemFactory>& GetRegistry();
    static void Register(const std::string& name, ItemFactory factory);
    static CItem* Create(const std::string& name);
};



// Helper macro for auto-registration
#define REGISTER_ITEM(NAME, CLASS) \
    namespace { \
        CItem* Create##CLASS() { return new CLASS(); } \
        struct CLASS##Registrator { \
            CLASS##Registrator() { ItemRegistry::Register(NAME, Create##CLASS); } \
        } CLASS##RegistratorInstance; \
    }
