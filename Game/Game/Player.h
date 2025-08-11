#pragma once
#include "main.h"
#include "CWeapon.h"

namespace player
{

	const float FRICTION = 8.0f;
	const float ACCELERATION = 1.00f;
	const float SPEED = 0.50f;
	const core::vector3df MAX_VELOCITY = core::vector3df(0.5f, 1.7f, 0.5f);
	const float GRAVITY = 10.0f;
	const float JUMP_FORCE = 4.f;
	//const float STEP_HEIGHT = 0.45f;
	const float STEP_HEIGHT = 0.5f;
	const float NOCLIP_SPEED = 1.0f;
	const core::vector3df STAND_SIZE = core::vector3df(0.25f, 0.9f, 0.25f);
	const core::vector3df CROUCH_SIZE = core::vector3df(0.25f, 0.42f, 0.25f);
	const core::vector3df STAND_VIEW = core::vector3df(0.0f, 0.75f, 0.0f);
	const core::vector3df CROUCH_VIEW = core::vector3df(0.0f, 0.45f, 0.0f);

	//Variables
	extern core::vector3df PlayerPosition;
	extern core::vector3df VecView;
	extern core::vector3df PlayerVelocity;
	extern core::vector3df CameraPos;
	extern core::vector3df CameraStartRotation;
	extern core::vector3df PlayerSize;
	extern aabbox3df PlayerCollider;


	extern bool PlayerGrounded;
	extern bool noclip;
	extern int PlayerHealth;

	extern void PlayerUpdate();


	extern IKinematicCharacterController* PlayerController;
}