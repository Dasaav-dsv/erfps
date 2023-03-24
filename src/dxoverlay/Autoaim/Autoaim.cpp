#include "Autoaim.h"

using namespace OF;

void Autoaim::Setup()
{
	InitFramework(device, spriteBatch, window);

	aaTextureID = LoadTexture(IDR_AIM0512);

	aaBox = CreateBox(0, 0, boxSizeX, boxSizeY);
}

void Autoaim::Update()
{
	float dT;
	bool lockState;

	if (ReadUnsafePtr(TraverseUnsafePtr(lockModeEnabled_ptr), lockState) && ReadUnsafePtr(TraverseUnsafePtr<float>(dT_ptr), dT))
	{
		if (!(camData.pCamBool->isCrosshairEnable && lockState))
		{
			aaTime = std::clamp(aaTime - dT, 0.0f, aaTimeMax + 0.1f);
		}
	}
	else
	{
		aaTime = 0.0f;
	}

	float fov;

	if (ReadUnsafePtr(TraverseUnsafePtr<float>(fov_ptr), fov))
	{
		aaFov = fov;
	}

	RECT hwndRect;
	GetClientRect(ofWindow, &hwndRect);

	ofWindowWidth = hwndRect.right - hwndRect.left;
	ofWindowHeight = hwndRect.bottom - hwndRect.top;
}

void Autoaim::Render()
{
	if (camData.pCamBool->isCamInit && iniSet.pIniBool->isShowAutoaim)
	{
		Update();

		if (aaTime > 0.0f)
		{
			if (*TraverseSafePtr(lockModeEnemy_ptr) == 1)
			{
				aaRotSpeed = 0.0f;

				aaShaderTime += *TraverseSafePtr<float>(dT_ptr) * 4.0f;
			}
			else if (aaTime < aaTimeMax)
			{
				aaRotSpeed = 0.05f;

				if (*TraverseSafePtr(lockModeEnabled_ptr) == 0)
				{
					aaRotSpeed *= -1.0f;
				}
			}
			else
			{
				aaRotSpeed = 0.0f;

				aaShaderTime = std::clamp(aaShaderTime - *TraverseSafePtr<float>(dT_ptr) * 2.0f, 0.0f, 2.0f);
			}

			aaRot += aaRotSpeed;

			if (aaRot >= M_PIf * 2.0f)
			{
				aaRot -= M_PIf * 2.0f;
			}

			float scale = std::clamp(aaTime, 0.0f, aaTimeMax) * tanf(iniSet.aimAssistAng * M_PI_180f) / (aaTimeMax * tanf(aaFov * 0.5f));
			const float WHf = static_cast<float>(ofWindowHeight) * scale;

			camData.aaScale = scale * 7.0f;

			aaBox->scale = WHf * 0.002f;
			aaBox->rotation = aaRot;

			aaBox->x = ofWindowWidth / 2 - boxSizeX / 2;
			aaBox->y = ofWindowHeight / 2 - boxSizeY / 2;

			SetShaderID(2);

			float shaderIntensity = std::clamp(aaShaderTime, 0.5f, 3.0f);

			if (SetShaderBuffer(_mm_set_ps(shaderIntensity + (shaderIntensity - 0.5f) * (0.5f + sinf(aaShaderTime * 0.7f)), 0.0f, 0.0f, 0.0f)))
			{
				float colorIntensity = std::clamp(aaShaderTime, 0.3f, 1.0f);

				DrawBox(aaBox, aaTextureID, _mm_set1_ps(colorIntensity));
			}
		}
	}
}

void Autoaim::DrawAA()
{
	float dT;

	if (ReadUnsafePtr(TraverseUnsafePtr<float>(dT_ptr), dT))
	{
		aaTime = std::clamp(aaTime + dT, 0.0f, aaTimeMax + 0.1f);
	}
	else
	{
		aaTime = 0.0f;
	}
}