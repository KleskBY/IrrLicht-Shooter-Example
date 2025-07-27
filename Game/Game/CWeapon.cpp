#include "CWeapon.h"


void CWeapon::AddRecoil(float x, float y)
{
    input::Recoil.X = input::Recoil.X + x;
    input::Recoil.Y = input::Recoil.Y + y;
    input::PunchAngle.X = x * 10.f;
    input::PunchAngle.Y = y * 10.f;
}


void CWeapon::FireBullet(vector3df spread, float dist, float dmg)
{
    core::vector3df direction = (MainCamera->getTarget() - MainCamera->getPosition()).normalize();
    if (spread != core::vector3df(0, 0, 0))
    {
        direction.X += Random(-spread.X, spread.X);
        direction.Y += Random(-spread.Y, spread.Y);
        direction.Z += Random(-spread.X, spread.X);
        direction.normalize();
    }

    core::line3d<f32> ray;
    ray.start = MainCamera->getPosition();
    ray.end = ray.start + direction * dist;

    vector3df intersection;
    triangle3df hitTriangle;

    ISceneNode* selectedSceneNode = collMan->getSceneNodeAndCollisionPointFromRay(ray, intersection, hitTriangle, 0, 0);
    IAnimatedMeshSceneNode* selectedMeshSceneNode = static_cast<IAnimatedMeshSceneNode*>(selectedSceneNode);

    vector3df normal = hitTriangle.getNormal().normalize();
    AddBulletHole(intersection, normal);
    Muzzleflash();

    IParticleSystemSceneNode* partSys = createParticleSystem(intersection, normal / 100, 8, 8,
        SColor(0, 255, 255, 255), SColor(0, 255, 255, 255),
        500.0f, 500.0f,
        0,
        dimension2df(0.1f, 0.1f), dimension2df(0.2f, 0.2f),
        "data/smoke2.jpg",
        350.0f,
        true,
        EMT_TRANSPARENT_ADD_COLOR,
        true);
}


void CWeapon::WeaponThink()
{
    if (nextthink > Time) return;
    if (state == WEAPON_STATE_IDLE || state == WEAPON_STATE_ATTACK)
    {
        if (semi || clip <= 0)
        {
            if (input::GetAttackInputSemi())
            {
                StartAttack();
            }
        }
        else
        {
            if (input::GetAttackInput())
            {
                StartAttack();
            }
        }

        if (input::GetReloadInput())
        {
            StartReload();
        }
    }


    if (state == WEAPON_STATE_ATTACK)
    {
        frame = frame + 1;
        nextthink = Time + framerate;
        if (frame >= attackEndFrame) state = WEAPON_STATE_IDLE;
    }
    else if (state == WEAPON_STATE_RELOAD)
    {
        frame = frame + 1;
        nextthink = Time + framerate;
        Reload();
    }
    else if (state == WEAPON_STATE_HIDE)
    {
        weaponOffset = lerp(weaponOffset, vector3df(0, -1, 0), 10.f * DeltaTime);
        //ViewmodelNode->setPosition(lerp(ViewmodelNode->getPosition(), vector3df(0, -1, 0), 10.f * DeltaTime));
    }
    else if (state == WEAPON_STATE_GET)
    {
        weaponOffset = lerp(weaponOffset, vector3df(0, 0, 0), 10.f * DeltaTime);
        if (weaponOffset.getDistanceFrom(vector3df(0, 0, 0)) < 0.05f) state = WEAPON_STATE_IDLE;

        //ViewmodelNode->setPosition(lerp(ViewmodelNode->getPosition(), vector3df(0, 0, 0), 10.f * DeltaTime));
        //if (ViewmodelNode->getPosition().getDistanceFrom(vector3df(0, 0, 0)) < 0.05f) state = WEAPON_STATE_IDLE;
    }

    ViewmodelFrame = frame;
    ViewmodelModel = viewModel;
}



void CWeapon::StartAttack()
{
    if (lastAttack + rate > Time) return;

    lastAttack = Time;
    if (clip <= 0)
    {
        sound::Play2D(noammoSound.c_str());
        return;
    }

    state = WEAPON_STATE_ATTACK;
    frame = attackStartFrame;
    nextthink = Time + framerate;
    Fire();
}

void CWeapon::StartReload()
{
    if (ammo <= 0)
    {
        //CycleWeaponReverseCommand();
        return;
    }
    if (clip < maxClip + 1)
    {
        frame = reloadStartFrame;
        nextthink = Time + framerate;
        state = WEAPON_STATE_RELOAD;
    }
}

void CWeapon::FinishReload()
{
    state = WEAPON_STATE_IDLE;
    frame = 0;

    if (clip > 0) clip = maxClip + 1;
    else clip = maxClip;

    /* if (self.ammoType >= clipSize)
     {
         self.ammoType = self.ammoType - (clipSize - self.clip);
         if (self.clip > 0) self.clip = clipSize + 1;
         else self.clip = clipSize;
         self.currentClip = self.clip;
     }
     else
     {
         self.clip = self.ammoType;
         self.currentClip = self.clip;
         self.ammoType = 0;
     }*/
     //W_SetCurrentAmmo();
};

void CWeapon::Hide()
{
    state = WEAPON_STATE_HIDE;
}

void CWeapon::Get()
{
    ViewmodelNode->setPosition(vector3df(0, -1, 0));
    state = WEAPON_STATE_GET;
}