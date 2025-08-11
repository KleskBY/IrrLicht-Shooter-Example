#pragma once
#include "main.h"

namespace hud
{
	void ShowMessage(const std::wstring& msg);
	void DrawHUD(int currentClip);
	void InitHUD();
	extern float recoilAmount;
};
