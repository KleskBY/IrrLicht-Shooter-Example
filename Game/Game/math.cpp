#include "math.h"

core::vector3df FromImperial(const core::vector3df& quakePos)
{
    return core::vector3df(
        quakePos.X,
        -quakePos.Z, // Quake Z up to Irrlicht Y
        quakePos.Y
    );
}

// Convert from Irrlicht coordinates (Y up) to Quake 3 (Z up)
core::vector3df ToImperial(const core::vector3df& irrPos)
{
    return core::vector3df(
        irrPos.X,
        irrPos.Z,   // Irrlicht Y-up to Quake Y
        -irrPos.Y   // Irrlicht Y becomes -Z
    );
}


// Convert string to vector3df
vector3df stov(const char* str)
{
	float x = 0.f, y = 0.f, z = 0.f;

	// Replace commas with spaces for easier parsing
	std::string clean = str;
	for (char& c : clean)
		if (c == ',') c = ' ';

	std::stringstream ss(clean);
	ss >> x >> y >> z;

	return vector3df(x, y, z);
}

core::vector3df ClampVector(const core::vector3df& value, const core::vector3df& min, const core::vector3df& max)
{
	return core::vector3df(core::clamp(value.X, min.X, max.X), core::clamp(value.Y, min.Y, max.Y), core::clamp(value.Z, min.Z, max.Z));
}