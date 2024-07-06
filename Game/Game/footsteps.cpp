#include "footsteps.h"
#include "main.h"
#include "Player.h"
#include "sound.h"

float FOOTSTEP_RATE_RUN = 0.4f;
float FOOTSTEP_VOLUME = 0.9f;
float footstep_finished;
void PlayerFootsteps()
{
	if (player::PlayerVelocity.getLength() < 0.02f) return;

	int r = Random(0, 4);
	if (player::PlayerGrounded /*&& !self.crouching && !self.button3 && self.movetype != MOVETYPE_NOCLIP*/)
	{
		if (Time > footstep_finished || footstep_finished == 0)
		{
			if (r == 0) sound::Play("data/sound/footsteps/footstep1.wav", MainCamera->getPosition(), FOOTSTEP_VOLUME); //sound(self, CHAN_BODY, "footsteps/footstep1.wav", FOOTSTEP_VOLUME, ATTN_IDLE);
			else if (r == 1)  sound::Play("data/sound/footsteps/footstep2.wav", MainCamera->getPosition(), FOOTSTEP_VOLUME);
			else if (r == 2)  sound::Play("data/sound/footsteps/footstep3.wav", MainCamera->getPosition(), FOOTSTEP_VOLUME);
			else sound::Play("data/sound/footsteps/footstep4.wav", MainCamera->getPosition(), FOOTSTEP_VOLUME);
			footstep_finished = Time + FOOTSTEP_RATE_RUN;
			//if (!self.button4) self.footstep_finished = time + FOOTSTEP_RATE_WALK;
		}
	}
}