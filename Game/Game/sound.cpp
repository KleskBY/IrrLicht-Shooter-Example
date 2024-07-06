#include "sound.h"

using namespace irrklang;
namespace sound
{
	irrklang::ISoundEngine* engine;

	bool InitSoundSystem()
	{
		engine = createIrrKlangDevice();
		if (!engine) return false;
		return true;
	}

	void UpdateSoundSystem(irr::core::vector3df pos, irr::core::vector3df target)
	{
		if (!engine) return;
		engine->setListenerPosition(vec3df(pos.X, pos.Y, pos.Z), vec3df(target.X, target.Y, target.Z)); // update 3D position for sound engine
	}

	void Play(const char* path, irr::core::vector3df pos, float vol, float min, float max)
	{
		if (!engine) return;
		ISound* snd = engine->play3D(path, vec3df(pos.X, pos.Y, pos.Z), false, true, true);
		if (!snd) return;
		snd->setMinDistance(min);
		snd->setMaxDistance(max);
		snd->setVolume(vol);
		snd->setIsPaused(false);
	}

	void Play2D(const char* path, float vol)
	{
		if (!engine) return;
		ISound* snd = engine->play2D(path, false, true);
		if (!snd) return;
		snd->setVolume(vol);
		snd->setIsPaused(false);
	}

}

