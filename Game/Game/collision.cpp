#include "collision.h"
#include "entity.h"

namespace collision
{
	irrBulletWorld* world;

	void InitCollision()
	{
		world = createIrrBulletWorld(device, true, true);
		world->setDebugMode(irrPhysicsDebugMode::EPDM_DrawAabb | irrPhysicsDebugMode::EPDM_DrawContactPoints);
		world->setGravity(core::vector3df(0, -10.f, 0));
		world->setPropertiesTextPosition(EDPT_POSITION::EDPT_TOP_RIGHT);
	}

	void UpdateEntitiesCollision()
	{
		for (const auto& ent : EntityList)
		{
			ent->CollisionCheck();
		}
	}

	void UpdateCollision()
	{
		UpdateEntitiesCollision();
		world->stepSimulation(DeltaTime);
	}

    void AddCollisionForMesh(ISceneNode* n, IMesh* collMesh)
    {
        if (!n || !collMesh) {
            std::cout << "Error: Invalid node or mesh passed to AddCollisionForMesh" << std::endl;
            return;
        }

        bool hasValidGeometry = false;
        for (u32 i = 0; i < collMesh->getMeshBufferCount(); ++i)
        {
            auto buf = collMesh->getMeshBuffer(i);
            if (buf && buf->getVertexCount() > 0 && buf->getIndexCount() > 0)
            {
                hasValidGeometry = true;
                break;
            }
        }

        if (!hasValidGeometry)
        {
            std::cout << "Error: Mesh has no valid geometry for collision" << std::endl;
            return;
        }

        auto shape = new IBvhTriangleMeshShape(n, collMesh, 0);
        collision::world->addRigidBody(shape);
    }


}