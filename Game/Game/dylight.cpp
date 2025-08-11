#include "dylight.h"


std::vector<Q3Light> g_Lights;
gui::IGUIFont* font;
s32 GetLight(IQ3LevelMesh* mesh, s32 startposIndex)
{
	if (0 == mesh) return 0;

	irr::scene::quake3::tQ3EntityList& entityList = mesh->getEntityList();
	irr::scene::quake3::IEntity search;
	search.name = "light";

	// find all entities in the multi-list
	s32 lastIndex;
	s32 index = entityList.binary_search_multi(search, lastIndex);
	if (index < 0) return 0;

	index += core::clamp(startposIndex, 0, lastIndex - index);

	u32 parsepos;

	const irr::scene::quake3::SVarGroup* group;
	group = entityList[index].getGroup(1);

	parsepos = 0;
	vector3df pos = irr::scene::quake3::getAsVector3df(group->get("origin"), parsepos);
	pos = pos * 0.025f;

	parsepos = 0;
	f32 rad = irr::scene::quake3::getAsFloat(group->get("light"), parsepos);
	if (rad == 0.0f) rad = 300.f;
	rad = rad * 0.025f;

	parsepos = 0;
	vector3df color = irr::scene::quake3::getAsVector3df(group->get("_color"), parsepos);
	if (color.X == 0 && color.Y == 0 && color.Z == 0) color = vector3df(1.f, 1.f, 1.f);

	Q3Light light;
	light.position = pos;
	light.radius = rad;
	light.color.set(color.X, color.Z, color.Y); // G and B are actually Z and Y

	g_Lights.push_back(light);
	auto cube = smgr->addCubeSceneNode(0.1f, 0,-1, pos);
	cube->getMaterial(0).Lighting = false;
	smgr->getMeshManipulator()->setVertexColors(cube->getMesh(), light.color.toSColor());
	auto lt = smgr->addLightSceneNode(0, pos, light.color.toSColor(), rad);

	//gui::IGUIFont* font = device->getGUIEnvironment()->getFont("../../media/fontlucida.png");
	//std::string cl = std::to_string(lt->getLightData().DiffuseColor.r) + ", " + std::to_string(lt->getLightData().DiffuseColor.g) + ", " + std::to_string(lt->getLightData().DiffuseColor.b);
	//core::stringw name(cl.c_str());
	//auto node = smgr->addBillboardTextSceneNode(font, name.c_str(), lt, core::dimension2d<f32>(2.0f, 0.20f), core::vector3df(0, 0, 0));

	return lastIndex - index + 1;
}


void ParseLights(IQ3LevelMesh* q3mesh)
{
	g_Lights.clear();
	s32 remaining = GetLight(q3mesh, 0);
	s32 i = 1;
	while (remaining > 1)
	{
		remaining = GetLight(q3mesh, i);
		i++;
	}
	std::cout << "LOADED AMOUNT OF LIGHTS: " << g_Lights.size() << std::endl;
}


video::SColorf GetDynamicLightColorAtPosition(const core::vector3df& pos)
{
	video::SColorf total(0, 0, 0);

	for (auto& light : g_Lights)
	{
		core::vector3df diff = light.position - pos;
		float dist = diff.getLength();

		if (dist < light.radius) // within influence
		{
			float factor = 1.0f - (dist / light.radius); // linear falloff
			total.r += light.color.r * factor;
			total.g += light.color.g * factor;
			total.b += light.color.b * factor;
		}
	}

	// Clamp to [0,1]
	total.r = core::clamp(total.r, 0.f, 1.f);
	total.g = core::clamp(total.g, 0.f, 1.f);
	total.b = core::clamp(total.b, 0.f, 1.f);
	//std::cout << total.r << " " << total.g << " " << total.b << std::endl;
	return total;
}
