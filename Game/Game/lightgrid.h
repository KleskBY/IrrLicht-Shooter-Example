#pragma once
#include "main.h"

struct LightGrid {
    core::aabbox3df bounds;
    core::vector3di size;      // number of cells (x,y,z)
    std::vector<video::SColor> data; // size.x * size.y * size.z
    float cellSize;
};

extern LightGrid g_LightGrid;

struct LightSample {
    irr::video::SColor color;
    float brightness;
};
LightSample GetLightColorAtPoint(const core::vector3df& pos, IMesh* mesh);
void BakeLightGrid(IMesh* mesh, float cellSize);
video::SColor SampleLightGrid(const core::vector3df& pos);
video::SColor GetLightmapColorAtRayHit(vector3df start);
void CacheLightmaps(scene::IMesh* mesh);