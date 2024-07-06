#pragma once
#include "main.h"
#include "CWeapon.h"
#include "sound.h"

class WEAPON_PM : public CWeapon
{
public:
    WEAPON_PM() {
        damage = 20;
        rate = 0.1;
        semi = true;
        distance = 256;
        ammo = 8; // assuming ammoRifle is defined elsewhere
        maxClip = 8;
        clip = maxClip;
        framerate = 0.01;

        reloadStartFrame = 36;
        reloadEndFrame = 156;
        drawStartFrame = 1;
        drawEndFrame = 25;
        attackStartFrame = 26;
        attackEndFrame = 35;

        weaponModelLength = 0.55f;
        viewModel = "models/weapons/v_pm.md3";
        playerModel = "models/players/ussr.md3";
        worldModel = "models/weapons/w_ak.md3";
        fireSound = "data/sound/weapons/pm/shot.wav";
        clipinSound = "data/sound/weapons/pm/clipin.wav";
        clipoutSound = "data/sound/weapons/ak74/clipout.wav";
        slide1Sound = "data/sound/weapons/sniper/boltback.wav";
        slide2Sound = "data/sound/weapons/sniper/boltrelease.wav";
        noammoSound = "data/sound/weapons/noammo.wav";
    }

    void Fire() override
    {
        clip -= 1;
        AddRecoil(Random(-0.4f, 0.4f), Random(-1.f, -0.5f));
        FireBullet(vector3df(0.02f, 0.02f, 0), distance, damage);
        sound::Play2D(fireSound.c_str());
    }

    void Reload() override
    {
        if (frame == 56)  sound::Play2D(clipoutSound.c_str(), 0.5f);//sound(self, CHAN_ITEM, AK74_CLIPOUT_SOUND, 0.5, ATTN_NORM);
        if (frame == 104) sound::Play2D(clipinSound.c_str(), 0.5f);//sound(self, CHAN_ITEM, AK74_CLIPIN_SOUND, 0.5, ATTN_NORM);
        if (frame == 136) sound::Play2D(slide1Sound.c_str(), 0.5f);//sound(self, CHAN_ITEM, AK74_SLIDE1, 0.5, ATTN_NORM);
        if (frame == 142) sound::Play2D(slide2Sound.c_str(), 0.5f); //sound(self, CHAN_ITEM, AK74_SLIDE2, 0.5, ATTN_NORM);
        if (frame > reloadEndFrame) FinishReload();
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