#pragma once
#define NOMINMAX
#include "main.h"


class InputEventReceiver : public IEventReceiver
{
public:
    // We'll create a struct to record info on the mouse state
    struct SMouseState
    {
        core::position2di Position;
        bool LeftButtonDown;
        SMouseState() : LeftButtonDown(false) { }
    } MouseState;

    virtual bool OnEvent(const SEvent& event);
    //const SEvent::SJoystickEvent& GetJoystickState(void) const;
    //const SMouseState& GetMouseState(void) const;
    //InputEventReceiver()
    //{
    //}
private:
    SEvent::SJoystickEvent JoystickState;
};

extern InputEventReceiver InputRecevier;


namespace input
{
    extern core::vector3df Movement;
    extern core::vector3df Recoil;
    extern core::vector3df PunchAngle;
    extern bool Crouch;
    extern void ProcessMovement();
    extern void MouseLook();
    extern bool GetAttackInput();
    extern bool GetReloadInput();
    extern bool GetAttackInputSemi();
    extern int GetWeaponSelectInput();
    extern int GetMouseScrollInput();
    extern float m_yaw;
    extern float m_pitch;
}