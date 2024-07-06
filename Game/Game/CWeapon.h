#pragma once
#include "main.h"
#include "bulletholes.h"
#include "viewmodel.h"
#include "input.h"
#include "sound.h"

enum AMMO_TYPE {
    AMMO_TYPE_NONE,
    AMMO_TYPE_PISTOL,
    AMMO_TYPE_COUNT
};

enum WEAPON_STATE {
    WEAPON_STATE_IDLE,
    WEAPON_STATE_ATTACK,
    WEAPON_STATE_RELOAD,
    WEAPON_STATE_CHANGE,
    WEAPON_STATE_HIDE,
    WEAPON_STATE_GET,
    WEAPON_STATE_COUNT
};

class CWeapon {
public:
    int state = 0;

    float damage;
    
    float lastAttack = 0;
    float rate = 0.15f;
    bool semi = false;
    float distance = 4096;

    int ammo;
    int maxClip;
    int clip;

    float framerate;
    int frame = 0;
    int reloadStartFrame;
    int reloadEndFrame;
    int drawStartFrame;
    int drawEndFrame;
    int attackStartFrame;
    int attackEndFrame;

    float nextthink = 0.f;

    float weaponModelLength = 1.0f;
    std::string viewModel;
    std::string playerModel;
    std::string worldModel;
    std::string fireSound;
    std::string clipinSound;
    std::string clipoutSound;
    std::string slide1Sound;
    std::string slide2Sound;
    std::string noammoSound;

  

    // Virtual methods to be implemented by derived classes
    virtual void Fire() = 0;
    virtual void Reload() = 0;
    virtual void Switch() = 0;
    virtual void Select() = 0;
    virtual void Precache() = 0;

    void AddRecoil(float x, float y);
    void FireBullet(vector3df spread = vector3df(0, 0, 0), float dist = 4096.0f, float dmg = 10.f);
    void WeaponThink();

    void StartAttack();
    void StartReload();
    void FinishReload();


    void Hide();
    void Get();

};