#pragma once
#pragma once
#include "main.h"
#include "CWeapon.h"
#include "sound.h"

class WEAPON_KNIFE : public CWeapon
{
public:
    WEAPON_KNIFE() {
        damage = 25;
        rate = 0.55;
        semi = false;
        distance = 1;
        ammo = 9999; // assuming ammoRifle is defined elsewhere
        maxClip = 9999;
        clip = maxClip;
        framerate = 0.01;

        reloadStartFrame = 36;
        reloadEndFrame = 156;
        drawStartFrame = 1;
        drawEndFrame = 25;
        attackStartFrame = 35;
        attackEndFrame = 89;

        weaponModelLength = 0.55f;
        viewModel = "models/weapons/v_knife.md3";
        playerModel = "models/players/ussr.md3";
        worldModel = "models/weapons/w_ak.md3";
        fireSound = "data/sound/weapons/knife/hit.wav";
        clipinSound = "data/sound/weapons/pm/clipin.wav";
        clipoutSound = "data/sound/weapons/ak74/clipout.wav";
        slide1Sound = "data/sound/weapons/knife/whoosh1.wav";
        slide2Sound = "data/sound/weapons/knife/whoosh2.wav";
        noammoSound = "";
    }

    void Fire() override
    {
        FireBullet(vector3df(0.02f, 0.02f, 0), distance, damage);
        sound::Play2D(fireSound.c_str());
    }

    void Reload() override
    {
    }

    void Switch() override
    {
    }

    void Select() override
    {
    }

    void Precache() override
    {
    }
};