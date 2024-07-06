#include "materials.h"


void SetDefaultMaterial(ISceneNode* node)
{
	node->setMaterialFlag(video::EMF_LIGHTING, true);
	node->setMaterialFlag(video::EMF_BILINEAR_FILTER, false);
	node->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
}

void SetTransparentMaterial(ISceneNode* node)
{
	node->setMaterialType(video::EMT_TRANSPARENT_ALPHA_CHANNEL);
	node->setMaterialFlag(video::EMF_LIGHTING, true);
	node->setMaterialFlag(video::EMF_BILINEAR_FILTER, false);
	node->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
}

