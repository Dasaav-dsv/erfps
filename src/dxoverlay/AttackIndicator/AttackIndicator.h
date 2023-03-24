#pragma once

#include <cstdint>
#include <algorithm>
#include <iostream>

#include "../../Defs.h"
#include "../../../resource/resource.h"

#include "../../../include/DirectXHook/IRenderCallback.h"
#include "../../../include/DirectXHook/OverlayFramework.h"

#define IND_TOTAL 3

class AttackIndicator : public IRenderCallback
{
public:
	void Setup();
	void Render();

	void UpdateAll();
	void __vectorcall DrawIndicator(int64_t EnemyIns, __m128 indicatorXY);

private:
	struct IndicatorData
	{
		struct alignas(16)
		{
			float yOffset = 0.0f;
			float alpha = 0.0f;
			float xOffset = 0.0f;
			float time = 0.0f;
		} YAXT;

		int64_t EnemyIns = -1;
		uint64_t : 64;
	};

	int indicatorType = 0;

	const float refreshTime = 0.4f;
	const float fadeoutTime = 0.3f;

	int boxSizeX;
	int boxSizeY;

	int textureID = 0;

	OF::Box* indicatorBox[IND_TOTAL] = {};
	IndicatorData indicator[IND_TOTAL] = {};
};

inline AttackIndicator* s_AttackIndicator = nullptr;