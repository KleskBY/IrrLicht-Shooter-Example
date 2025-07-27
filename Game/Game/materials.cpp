#include "materials.h"
video::SMaterial DebugMaterial;

void SetDefaultMaterial(ISceneNode* node)
{
	//node->setMaterialFlag(video::EMF_LIGHTING, true);
	node->setMaterialFlag(video::EMF_TRILINEAR_FILTER, true);
	//node->setMaterialFlag(video::EMF_BILINEAR_FILTER, false); //use this for pixels
	node->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
	node->setMaterialFlag(video::EMF_USE_MIP_MAPS, true);
	node->setMaterialFlag(video::EMF_ANISOTROPIC_FILTER, true);
	node->setMaterialFlag(video::EMF_FOG_ENABLE, true);
}

void SetTransparentMaterial(ISceneNode* node)
{
	node->setMaterialType(video::EMT_TRANSPARENT_ALPHA_CHANNEL);
	node->setMaterialFlag(video::EMF_LIGHTING, true);
	node->setMaterialFlag(video::EMF_BILINEAR_FILTER, false);
	node->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
}



void InitDebugMaterial()
{
	DebugMaterial.setTexture(0, 0);
	DebugMaterial.Lighting = false;
	DebugMaterial.ZBuffer = false;
	DebugMaterial.Wireframe = true;
}