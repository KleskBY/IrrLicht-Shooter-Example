#include "collision.h"
#include "entity.h"
#include "Player.h"
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

    void DebugCollisions()
    {
        world->debugDrawWorld(true);
        world->debugDrawProperties(true);

        //driver->setTransform(video::ETS_WORLD, core::matrix4());
        //driver->setMaterial();
        //driver->draw3DBox(player::PlayerController);
    }

    IRigidBody* const addCube(const vector3df& pos, const vector3df& scale, f32 mass, stringc textureFile)
    {
        irr::scene::ISceneNode* Node = device->getSceneManager()->addCubeSceneNode(1.0f);
        Node->setScale(scale);
        Node->setPosition(pos);
        Node->setMaterialFlag(irr::video::EMF_LIGHTING, true);
        Node->setMaterialFlag(irr::video::EMF_NORMALIZE_NORMALS, true);
        Node->setMaterialTexture(0, device->getVideoDriver()->getTexture(textureFile.c_str()));
        //Node->setMaterialFlag(irr::video::EMF_WIREFRAME, drawWireFrame);

        ICollisionShape* shape = new IBoxShape(Node, mass, false);

        //shape->setMargin(0.01);

        IRigidBody* body;
        body = world->addRigidBody(shape);

        return body;
    }

    IRigidBody* const shootCube(const vector3df pos, vector3df rot, const vector3df& scale, f32 mass)
    {
        IRigidBody* body = addCube(pos, scale, mass, "");

        irr::core::matrix4 mat;
        mat.setRotationDegrees(rot);
        irr::core::vector3df forwardDir(irr::core::vector3df(mat[8], mat[9], mat[10]) * 120);

        body->setLinearVelocity(forwardDir);
        //body->setActivationState(EAS_DISABLE_DEACTIVATION);

        body->setDamping(0.2, 0.2);
        body->setFriction(0.4f);

        //ICollisionObjectAffectorDelete *deleteAffector = new ICollisionObjectAffectorDelete(4000);
        //body->addAffector(deleteAffector);

        body->getAttributes()->addBool("collide", true);

        //ICollisionObjectAffectorAttract* affector = new ICollisionObjectAffectorAttract(irr::core::vector3df(0,1000,0), 2500);
        //ICollisionObjectAffectorAttract* affector = new ICollisionObjectAffectorAttract(device->getSceneManager()->getActiveCamera(), 2500);
        //body->addAffector(affector);

        return body;
    }

    IRigidBody* const shootSphere(const vector3df pos, vector3df rot, const vector3df& scale, f32 mass)
    {
        irr::scene::ISceneNode* Node = device->getSceneManager()->addSphereSceneNode();
        Node->setScale(scale);
        Node->setPosition(pos);
        Node->setMaterialFlag(irr::video::EMF_LIGHTING, true);
        Node->setMaterialFlag(irr::video::EMF_NORMALIZE_NORMALS, true);
        Node->setMaterialTexture(0, device->getVideoDriver()->getTexture("rockwall.jpg"));

        //if (drawWireFrame) Node->setMaterialFlag(irr::video::EMF_WIREFRAME, drawWireFrame);

        ICollisionShape* shape = new ISphereShape(Node, mass, true);

        //shape->setMargin(0.01);

        IRigidBody* body = world->addRigidBody(shape);
        body->setDamping(0.2, 0.2);

        // Since we use a "bumpy" triangle mesh for the terrain, we need to set the CCD radius and
        // motion threshold to ensure that we don't have any tunneling problems.
        // This doesn't work very well for box shapes.
        // ALSO: If you use setLocalScaling() on the collision shape, be sure to call this again
        // after scaling to make sure that it matches the actual object.

        irr::core::matrix4 mat;
        mat.setRotationDegrees(rot);
        irr::core::vector3df forwardDir(irr::core::vector3df(mat[8], mat[9], mat[10]) * 120);

        body->setLinearVelocity(forwardDir);



        //ICollisionObjectAffectorDelete *deleteAffector = new ICollisionObjectAffectorDelete(4000);
        //body->addAffector(deleteAffector);

        return body;
    }


}