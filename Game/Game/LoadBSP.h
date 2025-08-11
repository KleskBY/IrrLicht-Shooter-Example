#pragma once
#include "main.h"
void ScaleMeshVertices(scene::IMesh* mesh, float scale);
std::vector<scene::SMesh*> SplitMeshBuffer(const scene::IMeshBuffer* buffer, u32 maxVerts = 65535);
s32 Q3StartPosition(IQ3LevelMesh* mesh, s32 startposIndex, float scale);
bool LoadLevelBsp8(std::string levelName);