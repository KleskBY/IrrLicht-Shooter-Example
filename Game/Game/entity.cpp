#include "entity.h"
#include "Player.h"

std::vector<CEntity*> EntityList;



void CEntity::CollisionCheck()
{
	BoxCollider.MinEdge = position - size;
	BoxCollider.MaxEdge = position + size;
	if (BoxCollider.intersectsWithBox(player::PlayerCollider))
	{
		if (touch) touch(this);
	}
	for (const auto& ent : EntityList)
	{
		if (ent == this) continue;
		if (BoxCollider.intersectsWithBox(ent->BoxCollider))
		{
			if (touch) touch(this);
		}
	}

	driver->setTransform(video::ETS_WORLD, core::matrix4());
	driver->draw3DBox(BoxCollider);
}


void CEntity::UpdateEntity()
{
	if (model.size())
	{
		if (!MeshNode)
		{
			MeshNode = smgr->addAnimatedMeshSceneNode(smgr->getMesh(model.c_str()));
			lastModel = model;
		}
		else
		{
			if (model != lastModel)
			{
				MeshNode->setMesh(smgr->getMesh(model.c_str()));
				lastModel = model;
			}
		}
	}
	if (MeshNode)
	{
		MeshNode->setPosition(position);
		MeshNode->setRotation(rotation);
		MeshNode->setScale(scale);
	}
	if (think) think(this);
}


void CEntity::Remove() 
{
	if (MeshNode) 
	{
		MeshNode->remove();
	}
	if (ColliderNode) 
	{
		ColliderNode->remove();
	}
	if (shape) 
	{
		delete shape;
	}

	delete this;
}

CEntity::~CEntity() 
{
	auto it = std::find(EntityList.begin(), EntityList.end(), this);
	if (it != EntityList.end())
	{
		std::cout << "Removed ent from entity list\n" << std::endl;
		EntityList.erase(it);
	}
}

CEntity::CEntity()
{
	EntityList.push_back(this);
	std::cout << "spawned entity\n";
}











void UpdateEntities()
{
	for (const auto& ent : EntityList)
	{
		ent->UpdateEntity();
	}
}


void UpdateEntitiesCollision()
{
	for (const auto& ent : EntityList)
	{
		ent->CollisionCheck();
	}
}

