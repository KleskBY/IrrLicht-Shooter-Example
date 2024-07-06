#pragma once
#include <irrKlang.h>
#include <irrlicht.h>

namespace sound
{
	extern irrklang::ISoundEngine* engine;

	extern bool InitSoundSystem();
	extern void UpdateSoundSystem(irr::core::vector3df pos, irr::core::vector3df target);
	extern void Play(const char* path, irr::core::vector3df pos, float vol = 1.f, float min = 5.0f, float max = 100.f);
	extern void Play2D(const char* path, float vol = 0.7f);

}