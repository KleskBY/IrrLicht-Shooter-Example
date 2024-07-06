#pragma once
#include "main.h"
#include "CWeapon.h"
#include "sound.h"

class WEAPON_AK74 : public CWeapon
{
public:
    WEAPON_AK74() {
        damage = 35;
        rate = 0.11;
        semi = false;
        distance = 4096;
        ammo = 30; // assuming ammoRifle is defined elsewhere
        maxClip = 30;
        clip = maxClip;
        framerate = 0.01;

        reloadStartFrame = 37;
        reloadEndFrame = 150;
        drawStartFrame = 1;
        drawEndFrame = 30;
        attackStartFrame = 31;
        attackEndFrame = 36;

        weaponModelLength = 1.1f;
        viewModel = "models/weapons/v_ak74.md3";
        playerModel = "models/players/ussr.md3";
        worldModel = "models/weapons/w_ak.md3";
        fireSound = "data/sound/weapons/ak74/shot.wav";
        clipinSound = "data/sound/weapons/ak74/clipin.wav";
        clipoutSound = "data/sound/weapons/ak74/clipout.wav";
        slide1Sound = "data/sound/weapons/sniper/boltback.wav";
        slide2Sound = "data/sound/weapons/sniper/boltrelease.wav";
        noammoSound = "data/sound/weapons/noammo.wav";
    }

    void Fire() override
    {
        clip -= 1;
        AddRecoil(Random(-0.4f, 0.4f), Random(-1.0f, -0.5f));
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
        std::cout << "Switching weapon...\n";
    }

    void Select() override {
        std::cout << "Weapon selected: AK74\n";
        std::cout << "Model: " << viewModel << "\n";
        std::cout << "World Model: " << worldModel << "\n";
    }

    void Precache() override {
        std::cout << "Precaching assets for AK74...\n";
        std::cout << "Model: " << viewModel << "\n";
        std::cout << "Player Model: " << playerModel << "\n";
        std::cout << "World Model: " << worldModel << "\n";
        std::cout << "Sound: " << fireSound << "\n";
        std::cout << "Sound: " << clipinSound << "\n";
        std::cout << "Sound: " << clipoutSound << "\n";
        std::cout << "Sound: " << noammoSound << "\n";
    }
};