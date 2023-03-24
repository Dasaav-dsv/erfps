#pragma once

#include <cstdint>
#include <algorithm>
#include <iostream>

#include "../../Defs.h"
#include "../../../resource/resource.h"

#include "../../../include/DirectXHook/IRenderCallback.h"
#include "../../../include/DirectXHook/OverlayFramework.h"

class Crosshair : public IRenderCallback
{
public:
	void Setup();
	void Render();

	void Update();
	void Draw();

private:
	int textureID = 0;
	int crosshairType = 0;

	int boxSizeX;
	int boxSizeY;

	float time = 0.0f;
	float spread = 1.0f;

	float crouchTime = 0.0f;
	const float crouchTimeMax = 0.6f;

	OF::Box* box[4] = {};

	enum CrosshairType
	{
		ANGLED = 0,
		BASIC = 1,
		CIRCLE = 2,
		DOT = 3
	};
};

inline Crosshair* s_Crosshair = nullptr;