#include "AttackIndicator.h"

using namespace OF;

void AttackIndicator::Setup()
{
	InitFramework(device, spriteBatch, window);

	if (!textureID)
	{
		indicatorType = iniSet.indicatorType->Get();

		if (indicatorType <= 0)
		{
			textureID = LoadTexture(IDR_IND1024);

			boxSizeX = 512;
			boxSizeY = 1024;
		}
		else
		{
			textureID = LoadTexture(IDR_IND0256);

			boxSizeX = 128;
			boxSizeY = 256;
		}
	}

	for (int i = 0; i < IND_TOTAL; i++)
	{
		indicatorBox[i] = CreateBox(0, 0, boxSizeX, boxSizeY);
	}
}

void AttackIndicator::UpdateAll()
{
	RECT hwndRect;
	GetClientRect(ofWindow, &hwndRect);

	ofWindowWidth = hwndRect.right - hwndRect.left;
	ofWindowHeight = hwndRect.bottom - hwndRect.top;

	for (int i = 0; i < IND_TOTAL; i++)
	{
		IndicatorData* curr = &indicator[i];

		if (curr->EnemyIns != -1)
		{
			if (curr->YAXT.time > 0.0f)
			{
				curr->YAXT.time -= *TraverseSafePtr<float>(dT_ptr);
				curr->YAXT.alpha = std::clamp(curr->YAXT.time / fadeoutTime, 0.0f, 1.0f);
			}
			else
			{
				*reinterpret_cast<__m128*>(&curr->YAXT) = _mm_setzero_ps();
				curr->EnemyIns |= -1;
			}
		}
	}
}

void AttackIndicator::Render()
{
	if (iniSet.pIniBool->isShowAttacks)
	{
		UpdateAll();

		for (int i = 0; i < IND_TOTAL; i++)
		{
			IndicatorData* currInd = &indicator[i];

			if (currInd->EnemyIns == -1) continue;

			Box* currBox = indicatorBox[i];

			const float k = std::clamp(static_cast<float>(i), 0.0f, 6.0f);
			const float scale = (20.0f - k) / (20.0f + k);
			float WHf = static_cast<float>(ofWindowHeight) * scale;

			int widthOffset = ofWindowWidth;
			int heightOffset = ofWindowHeight;

			switch (indicatorType)
			{
			case 1:
				WHf *= camData.aaScale > 1.0f ? camData.aaScale : 1.0f;
			default:
			case 0:
				widthOffset /= 2;
				heightOffset /= 2;
				break;
			case 2:
				WHf *= 2.1f;
				widthOffset /= 10;
				heightOffset = (heightOffset * 843) / 1000;
				break;
			}

			currBox->scale = WHf * 0.00065f;
			currBox->rotation = -atan2f(currInd->YAXT.yOffset, currInd->YAXT.xOffset);

			WHf *= static_cast<float>(boxSizeY) / 1024.0f;

			currBox->x = widthOffset + static_cast<int>(MapFloatToRange(currInd->YAXT.xOffset, -1.0f, 1.0f, -WHf, WHf) * 0.3f) - boxSizeX / 2;
			currBox->y = heightOffset + static_cast<int>(MapFloatToRange(-currInd->YAXT.yOffset, -1.0f, 1.0f, -WHf, WHf) * 0.3f) - boxSizeY / 2;

			DrawBox(currBox, textureID, _mm_set1_ps(currInd->YAXT.alpha));
		}
	}
}

void __vectorcall AttackIndicator::DrawIndicator(int64_t EnemyIns, __m128 indicatorXY)
{
	IndicatorData* freeSlot = nullptr;

	for (int i = 0; i < IND_TOTAL; i++)
	{
		IndicatorData* currSlot = &indicator[i];

		if (currSlot->EnemyIns != -1)
		{
			if (currSlot->EnemyIns == EnemyIns)
			{
				_mm_store_ps(reinterpret_cast<float*>(&currSlot->YAXT), _mm_insert_ps(indicatorXY, _mm_set_ss(refreshTime), 0b00110000));
				return;
			}
		}
		else
		{
			freeSlot = currSlot;
		}
	}

	if (freeSlot != nullptr)
	{
		freeSlot->EnemyIns = EnemyIns;
		_mm_store_ps(reinterpret_cast<float*>(&freeSlot->YAXT), _mm_insert_ps(indicatorXY, _mm_set_ss(refreshTime), 0b00110000));
	}
}