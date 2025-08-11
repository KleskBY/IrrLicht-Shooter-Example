
#include "player.h"
#include "main.h"
#include "input.h"
#include <memory>

#include "bulletholes.h"
#include "viewmodel.h"
#include "footsteps.h"

#include "weapons.h"
#include "ak74.h"
#include "pm.h"
#include "knife.h"

#include "collision.h"

namespace player
{
	core::vector3df PlayerPosition = core::vector3df(-1.0f, 20.0f, 0.0f);
	core::vector3df VecView = core::vector3df(0.0f, 0.85f, 0.0f);
	core::vector3df PlayerVelocity;
	core::vector3df CameraPos;
	core::vector3df PlayerSize = STAND_SIZE;
	
	aabbox3df PlayerCollider(-PlayerSize, PlayerSize);

	IKinematicCharacterController* PlayerController;
	bool PlayerGrounded = false;
	bool PlayerIsCrouching = false;
	bool noclip = false;
	int PlayerHealth = 100;


	void CreatePlayer()
	{
		if (PlayerController) delete PlayerController;
		PlayerController = new IKinematicCharacterController(collision::world, PlayerSize.Y, PlayerSize.X, STEP_HEIGHT);
		PlayerController->warp(PlayerPosition);
		PlayerController->setJumpForce(JUMP_FORCE);
		PlayerController->setGravity(GRAVITY);
		PlayerController->setUpInterpolate(true);
	}

	float lastJump;
	void PlayerJump()
	{
		if (lastJump + 0.25f > Time) return;
		lastJump = Time;
		PlayerGrounded = false;
		PlayerController->jump();
	}


	void ApplyFriction()
	{
		if (PlayerVelocity.X || PlayerVelocity.Z)
		{
			float f = 1.0f - DeltaTime * FRICTION;
			PlayerVelocity.X = PlayerVelocity.X * f;
			PlayerVelocity.Z = PlayerVelocity.Z * f;
		}
	}

	void ApplyAcceleration()
	{
		core::vector3df forward = MainCamera->getTarget() - MainCamera->getPosition();
		forward.Y = 0;
		forward.normalize();

		core::vector3df side = MainCamera->getTarget() - MainCamera->getPosition();
		side.Y = 0;
		side.normalize();
		side = side.crossProduct(MainCamera->getUpVector());

		core::vector3df wishvel = forward * input::Movement.X + side * input::Movement.Z;
		wishvel.Y = 0.f;
		wishvel.normalize();
		if (PlayerIsCrouching) wishvel = wishvel * 0.5;
		PlayerVelocity = PlayerVelocity + wishvel * ACCELERATION * SPEED * DeltaTime;
	}








	void PlayerMovement()
	{
		if (PlayerHealth <= 0) return;
		noclip = GetAsyncKeyState(VK_MENU);//(input::Movement.Y < 0.f);
		if (noclip)
		{
			core::vector3df forward = MainCamera->getTarget() - MainCamera->getPosition();
			forward.normalize();

			core::vector3df side = MainCamera->getTarget() - MainCamera->getPosition();
			side.normalize();
			side = side.crossProduct(MainCamera->getUpVector());

			core::vector3df wishvel = forward * input::Movement.X + side * input::Movement.Z;
			PlayerPosition += wishvel * NOCLIP_SPEED * 100.f * DeltaTime;
			PlayerVelocity = core::vector3df(0, 0, 0);
			PlayerController->warp(PlayerPosition);

			MainCamera->setPosition(PlayerPosition + VecView);
			MainCamera->updateAbsolutePosition();
		}
		else
		{
			PlayerPosition = PlayerController->getWorldTransform().getTranslation();

			ApplyFriction();
			ApplyAcceleration();

			core::triangle3df out;
			core::vector3df hit;
			scene::ISceneNode* node;

			//if (input::Movement.Y < 0.f) // crouch input
			if (input::Crouch) // crouch input
			{
				if (!PlayerIsCrouching)
				{
					if (PlayerGrounded && PlayerController->isOnGround())
					{
						PlayerIsCrouching = true;
						PlayerSize = CROUCH_SIZE;
						VecView = CROUCH_VIEW;
						CreatePlayer();
					}
				}
			}
			else if (PlayerIsCrouching)
			{
				bool canStand = false;
				core::vector3df fallvector = collMan->getCollisionResultPosition(meta, PlayerPosition, STAND_SIZE, core::vector3df(0, 0, 0), out, hit, canStand, node, 0.0005f, core::vector3df(0, 0.05f, 0));
				//core::vector3df checkPos = PlayerPosition;
				//checkPos.Y += (STAND_SIZE.Y - CROUCH_SIZE.Y); // check above head
				//bool canStand = !collMan->getCollisionResultPosition(meta, checkPos, STAND_SIZE, core::vector3df(0, 0, 0), out, hit, fall, node, 0.0005f, core::vector3df(0, 1, 0)).getLengthSQ();
				if (canStand)
				{
					PlayerIsCrouching = false;
					PlayerSize = STAND_SIZE;
					VecView = STAND_VIEW;
					CreatePlayer();
				}
			}

			if (PlayerController->isOnGround() && PlayerController->canJump())
			{
				if (input::Movement.Y > 0) PlayerJump();
			}


			bool fall = false;
			core::vector3df fallvector = collMan->getCollisionResultPosition(meta, PlayerPosition, PlayerSize, core::vector3df(0, 0, 0), out, hit, fall, node, 0.0005f, core::vector3df(0, -0.01f, 0));
			PlayerGrounded = !fall;

			if (PlayerGrounded && PlayerController->isOnGround()) PlayerController->setGravity(0.0f); //disable gravity on ground so we dont slide down the slope
			else PlayerController->setGravity(GRAVITY);



			PlayerController->setWalkDirection(PlayerVelocity);

			core::vector3df needPos = PlayerPosition + VecView;
			core::vector3df camerapos = MainCamera->getPosition();
			camerapos = core::lerp<core::vector3df>(camerapos, PlayerPosition + VecView, 15.0f * DeltaTime);
			if (camerapos.getDistanceFrom(needPos) > 30.f) camerapos = needPos;
			MainCamera->setPosition(camerapos);

			//MainCamera->setPosition(PlayerPosition + VecView);
			MainCamera->updateAbsolutePosition();

			PlayerCollider.MinEdge = PlayerPosition - PlayerSize;
			PlayerCollider.MaxEdge = PlayerPosition + PlayerSize;
		}
	}



	void PlayerUpdate()
	{
		if (!MainCamera) return;
		if (!PlayerController) CreatePlayer();
		
		PlayerMovement();
		PlayerFootsteps();

		weapons::UpdateWeapon();
		if (weapons::ActiveWeapon)
		{
			weapons::ActiveWeapon->WeaponThink();
			ViewmodelModel = weapons::ActiveWeapon->viewModel;
			DrawViewmodel();
		}
		else
		{
			weapons::AddWeapon(new WEAPON_KNIFE());
			weapons::AddWeapon(new WEAPON_PM());
		}
	}
	

}