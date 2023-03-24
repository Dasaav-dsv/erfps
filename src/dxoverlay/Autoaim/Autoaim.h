#pragma once

#include <cstdint>
#include <algorithm>
#include <iostream>

#include "../../Defs.h"
#include "../../../resource/resource.h"

#include "../../../include/DirectXHook/IRenderCallback.h"
#include "../../../include/DirectXHook/OverlayFramework.h"

class Autoaim : public IRenderCallback
{
public:
	void Setup();
	void Render();

	void Update();
	void DrawAA();

private:
	int aaTextureID = 0;

	const int boxSizeX = 512;
	const int boxSizeY = 512;

	float aaTime = 0.0f;
	const float aaTimeMax = 0.177f;

	float aaFov = iniSet.fpFOV;
	
	float aaRot = 0.0f;

	float aaRotSpeed = 0.0f;

	float aaShaderTime = 0.0f;

	OF::Box* aaBox = nullptr;
};

inline Autoaim* s_Autoaim = nullptr;