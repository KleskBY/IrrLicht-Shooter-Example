#include "hud.h"
#include "main.h"

namespace hud
{
    struct HUDData {
        int health = 100;
        int armor = 0;
        int ammoClip = 30;
        int ammoReserve = 90;
        int weaponIconID = -1; // optional texture index
        std::wstring message;  // pickup messages
        float messageTimer = 0.f;
    };

    float crosshairGap = 8.0f;      // base distance from center
    float crosshairLength = 6.0f;   // length of each line
    float crosshairThickness = 1.5f;
    float recoilDecay = 1.0f;      // how fast it returns to normal
    float recoilAmount = 0.0f;      // dynamic recoil spread
    const video::SColor CROSSHAIR_COLOR = video::SColor(220, 50, 250, 50);

    HUDData hud;

    video::ITexture* texHealth;
    video::ITexture* texArmor;
    video::ITexture* texWeaponIcons[5];
    gui::IGUIFont* HudFont;

    const video::SColor HUD_COLOR = video::SColor(150, 255, 160, 0);
    const float BASE_WIDTH = 800.0f;
    const float BASE_HEIGHT = 600.0f;


    void InitHUD()
    {
        HudFont = device->getGUIEnvironment()->getFont("data/gfx/lcd.xml");

        texHealth = driver->getTexture("hud/health.tga");
        texArmor = driver->getTexture("hud/armor.tga");
        texWeaponIcons[5]; // e.g., AK, pistol, knife
        texWeaponIcons[0] = driver->getTexture("hud/ak74.tga");
    }

    void DrawRect(const core::rect<s32>& rect, video::SColor color)
    {
        driver->draw2DRectangle(color, rect);
    }

    void DrawImage(video::ITexture* tex, int x, int y)
    {
        driver->draw2DImage(tex, core::position2d<s32>(x, y));
    }

    void ShowMessage(const std::wstring& msg)
    {
        hud.message = msg;
        hud.messageTimer = 2.f; // 2 seconds display
    }

    void UpdateHUD()
    {
        if (hud.messageTimer > 0.f)
        {
            hud.messageTimer -= DeltaTime;
            if (hud.messageTimer < 0.f) hud.messageTimer = 0.f;
        }
    }

    inline int HUDX(float x, float hudScale) { return (int)(x * hudScale); }
    inline int HUDY(float y, float hudScale) { return (int)(y * hudScale); }

    void DrawCrosshair()
    {
        if (recoilAmount > 0.0f) 
        {
            recoilAmount -= recoilDecay * DeltaTime;
            if (recoilAmount < 0.0f) recoilAmount = 0.0f;
        }
        if (recoilAmount > 50.f) recoilAmount = 50.f;

        core::dimension2du screen = driver->getScreenSize();
        int cx = screen.Width / 2;
        int cy = screen.Height / 2;

        float gap = crosshairGap + recoilAmount * 20.f;
        // Horizontal lines
        driver->draw2DLine(core::position2d<s32>(cx - gap - crosshairLength, cy), core::position2d<s32>(cx - gap, cy), CROSSHAIR_COLOR);
        driver->draw2DLine(core::position2d<s32>(cx + gap, cy), core::position2d<s32>(cx + gap + crosshairLength, cy), CROSSHAIR_COLOR);

        // Vertical lines
        driver->draw2DLine(core::position2d<s32>(cx, cy - gap - crosshairLength), core::position2d<s32>(cx, cy - gap), CROSSHAIR_COLOR);
        driver->draw2DLine(core::position2d<s32>(cx, cy + gap), core::position2d<s32>(cx, cy + gap + crosshairLength), CROSSHAIR_COLOR);
    }


    void DrawHUD(int currentClip)
    {
        if (!HudFont) return;

        UpdateHUD();

        core::dimension2du screen = driver->getScreenSize();
        int sw = screen.Width;
        int sh = screen.Height;

        float scaleX = screen.Width / BASE_WIDTH;
        float scaleY = screen.Height / BASE_HEIGHT;
        float hudScale = min(scaleX, scaleY); // uniform scaling

        // === Health & Armor (bottom left) ===
        DrawImage(texHealth, 20, sh - 80);
        HudFont->draw((L"+" + std::to_wstring(hud.health)).c_str(), core::rect<s32>(HUDX(20, hudScale), sh - HUDY(75, hudScale), HUDX(120, hudScale), sh - HUDY(50, hudScale)), HUD_COLOR);

        //DrawImage(driver, texArmor, 20, sh - 50);
        //font->draw(std::to_wstring(hud.armor).c_str(), core::rect<s32>(60, sh - 45, 120, sh - 20), video::SColor(150, 255, 240, 240));

        // === Ammo (bottom right) ===
        if (currentClip < 9999)
        {
            std::wstring ammoText = std::to_wstring(currentClip) + L"/" + std::to_wstring(hud.ammoReserve);
            if (currentClip <= 9) ammoText = L" " + ammoText;
            HudFont->draw(ammoText.c_str(), core::rect<s32>(sw - HUDX(180, hudScale), sh - HUDY(75, hudScale), sw - HUDX(20, hudScale), sh - HUDY(50, hudScale)), HUD_COLOR, false, false);
        }

        // Weapon icon above ammo
        if (hud.weaponIconID >= 0) DrawImage(texWeaponIcons[hud.weaponIconID], sw - 100, sh - 120);

        // === Crosshair (center) ===
        DrawCrosshair();

        // === Pickup message (center-top) ===
        if (hud.messageTimer > 0.f)
        {
            HudFont->draw(hud.message.c_str(), core::rect<s32>(0, 50, sw, 100), HUD_COLOR, true, true);
        }
    }



}