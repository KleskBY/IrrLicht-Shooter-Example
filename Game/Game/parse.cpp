#include "parse.h"
#include "main.h"
#include "math.h"
#include "entity.h"
#include "ak74.h"
#include "items.h"
#include "item_registry.h"

void ParseEntity(IEntity e)
{
    CEntity* ent = new CEntity();
    for (u32 g = 0; g != e.getGroupSize(); ++g)
    {
        const SVarGroup* group = e.getGroup(g);
        for (u32 index = 0; index < group->Variable.size(); ++index)
        {
            const SVariable& v = group->Variable[index];
            if (v.name == "origin")
            {
                vector3df vec = stov(v.content.c_str()) * 0.025f;
                float t = vec.Y;
                vec.Y = vec.Z;
                vec.Z = t;
                ent->position = vec;
            }
            if (v.name == "classname") 
            {
                CItem* item = ItemRegistry::Create(v.content.c_str());
                if (item) 
                {
                    item->Spawn(ent);
                    delete item; // Clean up after spawning
                }
            }

        }
    }
}

void ParseEntities(IQ3LevelMesh* mesh)
{
    tQ3EntityList& entityList = mesh->getEntityList();
    for (u32 e = 0; e != entityList.size(); ++e)
    {
        ParseEntity(entityList[e]);
    }
}