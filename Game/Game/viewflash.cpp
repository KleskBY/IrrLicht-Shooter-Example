#include "viewflash.h"


struct ViewFlash {
    core::vector3df color; // RGB 0-1
    float alpha;           // 0-1
    float fadeTime;        // seconds to fade
    float currentTime;     // elapsed
    bool active;
} viewFlash;

void TriggerViewFlash(float r, float g, float b, float a, float fadeTime)
{
    viewFlash.color = core::vector3df(r, g, b);
    viewFlash.alpha = a;
    viewFlash.fadeTime = fadeTime;
    viewFlash.currentTime = 0.f;
    viewFlash.active = true;
}


void ViewflashUpdate()
{
    if (viewFlash.active)
    {
        viewFlash.currentTime += DeltaTime;
        float t = viewFlash.currentTime / viewFlash.fadeTime;
        if (t >= 1.0f)
        {
            viewFlash.active = false;
            t = 1.0f;
        }

        float alpha = viewFlash.alpha * (1.0f - t);

        // Render full-screen quad with color * alpha
        core::dimension2du screenSize = driver->getScreenSize();
        u32 screenWidth = screenSize.Width;
        u32 screenHeight = screenSize.Height;


        driver->draw2DRectangle(
            video::SColor((u32)(alpha * 255),
                (u32)(viewFlash.color.X * 255),
                (u32)(viewFlash.color.Y * 255),
                (u32)(viewFlash.color.Z * 255)),
            core::rect<s32>(0, 0, screenWidth, screenHeight));
    }

}