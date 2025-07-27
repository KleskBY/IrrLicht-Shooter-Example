#include "input.h"

namespace input
{
    bool keys[KEY_KEY_CODES_COUNT];
    float m_pitch;
    float m_yaw;
    float Sensitivity = 1.0f;
    vector3df Movement = vector3df(0, 0, 0);
    vector3df Recoil = vector3df(0,0,0);
    vector3df PunchAngle = vector3df(0, 0, 0);
    int mouseScroll = 0;
    void ProcessMovement()
    {
        Movement = core::vector3df(0, 0, 0);

        if (keys[irr::EKEY_CODE::KEY_KEY_W])
        {
            Movement.X = 1;
        }
        if (keys[irr::EKEY_CODE::KEY_KEY_S])
        {
            Movement.X = -1;
        }
        if (keys[irr::EKEY_CODE::KEY_KEY_A])
        {
            Movement.Z = 1;
        }
        if (keys[irr::EKEY_CODE::KEY_KEY_D])
        {
            Movement.Z = -1;
        }
        if (keys[irr::EKEY_CODE::KEY_SPACE])
        {
            Movement.Y = 1;
        }
        if (keys[irr::EKEY_CODE::KEY_LCONTROL])
        {
            Movement.Y = -1;
        }
    }

    constexpr float XM_PI = 3.141592654f;
    constexpr float XM_2PI = 6.283185307f;
    constexpr float XM_1DIVPI = 0.318309886f;
    constexpr float XM_1DIV2PI = 0.159154943f;
    constexpr float XM_PIDIV2 = 1.570796327f;
    constexpr float XM_PIDIV4 = 0.785398163f;
    void ClampInputAngles()
    {
        // limit pitch to straight up or straight down
        constexpr float limit = XM_PIDIV2 - 0.02f;
        m_pitch = std::max(-limit, m_pitch);
        m_pitch = std::min(+limit, m_pitch);

        // keep longitude in sane range by wrapping
        if (m_yaw > XM_PI) m_yaw -= XM_2PI;
        else if (m_yaw < -XM_PI) m_yaw += XM_2PI;
    }

    core::quaternion QuaternionFromYawPitch(float yaw, float pitch) 
    {
        // Convert degrees to radians
        yaw = core::DEGTORAD * yaw;
        pitch = core::DEGTORAD * pitch;

        // Calculate half angles
        float halfYaw = yaw * 0.5f;
        float halfPitch = pitch * 0.5f;

        // Calculate sin and cos of half angles
        float sinYaw = sin(halfYaw);
        float cosYaw = cos(halfYaw);
        float sinPitch = sin(halfPitch);
        float cosPitch = cos(halfPitch);

        // Calculate quaternion components
        float x = cosYaw * sinPitch;
        float y = sinYaw * cosPitch;
        float z = cosYaw * cosPitch;
        float w = sinYaw * sinPitch;

        return core::quaternion(x, y, z, w);
    }

    void MouseLook()
    {
        Recoil = lerp<vector3df>(Recoil, vector3df(0, 0, 0), 25 * DeltaTime);
        m_yaw = m_yaw - Recoil.X * DeltaTime;
        m_pitch = m_pitch - Recoil.Y * DeltaTime;

        ClampInputAngles();
        float y = sinf(m_pitch);
        float r = cosf(m_pitch);
        float z = r * cosf(m_yaw);
        float x = r * sinf(m_yaw);
        MainCamera->setTarget(MainCamera->getPosition() + core::vector3df(x, y, z));
        PunchAngle = lerp<vector3df>(PunchAngle, vector3df(0, 0, 0), 10 * DeltaTime);
        MainCamera->setRotation(lerp(MainCamera->getRotation(), MainCamera->getRotation() + PunchAngle, 20 * DeltaTime));
    }

    bool GetReloadInput()
    {
        return keys[irr::EKEY_CODE::KEY_KEY_R];
    }

    bool GetAttackInput()
    {
        return keys[irr::EKEY_CODE::KEY_LBUTTON];
    }

    bool pressedLastTime = false;
    bool GetAttackInputSemi()
    {
        if (keys[irr::EKEY_CODE::KEY_LBUTTON] )
        {
            if (!pressedLastTime)
            {
                pressedLastTime = true;
                return true;
            }
        }
        else pressedLastTime = false;

        return false;
    }

    int GetWeaponSelectInput()
    {
        for (int i = 1; i < 9; i++)
        {
            if (keys[irr::EKEY_CODE::KEY_KEY_0 + i]) return i;
        }
        return 0;
    }

    int GetMouseScrollInput()
    {
        int ret = mouseScroll;
        mouseScroll = 0;
        return ret;
    }
}




bool InputEventReceiver::OnEvent(const SEvent& event)
{
    core::vector3df delta;
    // Remember the mouse state
    const irr::core::dimension2du& screenSize = device->getVideoDriver()->getScreenSize();

    if (event.EventType == irr::EET_MOUSE_INPUT_EVENT)
    {
        switch (event.MouseInput.Event)
        {
        case EMIE_LMOUSE_PRESSED_DOWN:
            MouseState.LeftButtonDown = true;
            input::keys[irr::EKEY_CODE::KEY_LBUTTON] = true;
            break;
        case EMIE_LMOUSE_LEFT_UP:
            MouseState.LeftButtonDown = false;
            input::keys[irr::EKEY_CODE::KEY_LBUTTON] = false;
            break;
        case EMIE_MOUSE_MOVED:
            if (device->isWindowActive() && device->isWindowFocused())
            {
                delta = core::vector3df(float(device->getCursorControl()->getRelativePosition().X - 0.5f), float(device->getCursorControl()->getRelativePosition().Y - 0.5f), 0.f);
                input::m_pitch -= delta.Y * input::Sensitivity;
                input::m_yaw -= -delta.X * input::Sensitivity;
                device->getCursorControl()->setPosition(0.5f, 0.5f);
            }
            break;
        case EMIE_MOUSE_WHEEL:
            input::mouseScroll = event.MouseInput.Wheel;
            //std::cout << event.MouseInput.Wheel << std::endl;
        default:
            device->getCursorControl()->setPosition(0.5f, 0.5f);
            // We won't use the wheel
            break;
        }
    }

    // The state of each connected joystick is sent to us
    // once every run() of the Irrlicht device.  Store the
    // state of the first joystick, ignoring other joysticks.
    // This is currently only supported on Windows and Linux.
    if (event.EventType == irr::EET_JOYSTICK_INPUT_EVENT && event.JoystickEvent.Joystick == 0)
    {
        JoystickState = event.JoystickEvent;
    }


    if (event.EventType == irr::EET_KEY_INPUT_EVENT)
    {
        input::keys[event.KeyInput.Key] = event.KeyInput.PressedDown;
    }
        //printf("%c\n", event.KeyInput.Char);


    return false;
}

InputEventReceiver InputRecevier;

    //const SEvent::SJoystickEvent& InputEventReceiver::GetJoystickState(void) const
    //{
    //    return JoystickState;
    //}

    //const SEvent::SMouseState& InputEventReceiver::GetMouseState(void) const
    //{
    //    return MouseState;
    //}
