#pragma once
#include "main.h"

struct Q3Light {
	core::vector3df position;
	float radius;
	video::SColorf color;
};
extern std::vector<Q3Light> g_Lights;

void ParseLights(IQ3LevelMesh* q3mesh);

video::SColorf GetDynamicLightColorAtPosition(const core::vector3df& pos);