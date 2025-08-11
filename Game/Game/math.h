#pragma once
#include "main.h"

core::vector3df FromImperial(const core::vector3df& quakePos);
core::vector3df ToImperial(const core::vector3df& irrPos);
vector3df stov(const char* str);
core::vector3df ClampVector(const core::vector3df& value, const core::vector3df& min, const core::vector3df& max);