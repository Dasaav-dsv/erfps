#include "Crosshair.h"

using namespace OF;

void Crosshair::Setup()
{
	InitFramework(device, spriteBatch, window);

	switch (iniSet.crosshairType)
	{
	default:
		iniSet.crosshairType = *"A";
	case *"A":
		textureID = LoadTexture(IDR_A256_32);
		boxSizeX = 64;
		boxSizeY = 32;
		crosshairType = ANGLED;
		break;
	case *"B":
		textureID = LoadTexture(IDR_B144_64);
		boxSizeX = 64;
		boxSizeY = 64;
		crosshairType = BASIC;
		break;
	case *"C":
		textureID = LoadTexture(IDR_C256_32);
		boxSizeX = 64;
		boxSizeY = 32;
		crosshairType = CIRCLE;
		break;
	case *"D":
		textureID = LoadTexture(IDR_D064_64);
		boxSizeX = 64;
		boxSizeY = 64;
		crosshairType = DOT;
		break;
	}

	for (int i = 0; i < 4; i++)
	{
		box[i] = CreateBox(0, 0, boxSizeX, boxSizeY);
	}
}

void Crosshair::Update()
{
	float dT;

	if (ReadUnsafePtr(TraverseUnsafePtr<float>(dT_ptr), dT))
	{
		if (!camData.pCamBool->isCrosshairEnable)
		{
			this->time = std::clamp(this->time - dT, 0.0f, 0.05f);
		}
		else if (camData.pCamBool->isCrouch)
		{
			this->crouchTime = std::clamp(this->crouchTime + dT, 0.0f, crouchTimeMax);
		}
		else
		{
			this->crouchTime = std::clamp(this->crouchTime - dT * 2.5f, 0.0f, crouchTimeMax);
		}
	}
	else
	{
		this->time = 0.0f;

		this->crouchTime = 0.0f;
	}

	this->spread = camData.crosshairSpread;

	RECT hwndRect;
	GetClientRect(ofWindow, &hwndRect);

	ofWindowWidth = hwndRect.right - hwndRect.left;
	ofWindowHeight = hwndRect.bottom - hwndRect.top;
}

void Crosshair::Render()
{
	if (camData.pCamBool->isCamInit && iniSet.pIniBool->isCrosshairEnable)
	{
		Update();

		if (this->time > 0.0f)
		{
			float WHf = static_cast<float>(ofWindowHeight) * 0.000231f * std::clamp(iniSet.crosshairSize, 0.1f, 2.0f);
			int spread = static_cast<int>(camData.crosshairSpread * 20.0f);
			int crouchFrame = static_cast<int>(crouchTime * 52.0f) & 31;

			RECT atlasPos = {};

			atlasPos.left = boxSizeX * (crouchFrame % 4);
			atlasPos.right = boxSizeX * (crouchFrame % 4 + 1);
			atlasPos.top = boxSizeY * (crouchFrame / 4);
			atlasPos.bottom = boxSizeY * (crouchFrame / 4 + 1);

			switch (crosshairType)
			{
			case ANGLED:
			{
				int boxSizeXScaled = static_cast<int>(static_cast<float>(boxSizeX) * WHf);
				int boxSizeYScaled = static_cast<int>(static_cast<float>(boxSizeY) * WHf);

				for (int i = 0; i < 4; i++)
				{
					box[i]->scale = WHf;

					int boxOffsetX = i & 1;
					int boxOffsetY = (i >> 1) & 1;

					box[i]->x = ofWindowWidth / 2 - boxSizeXScaled * boxOffsetX - boxSizeXScaled / 2 + spread * boxSizeXScaled / 300 * (1 - 2 * boxOffsetX);
					box[i]->y = ofWindowHeight / 2 + boxSizeYScaled * boxOffsetY - (boxSizeYScaled * 3) / 2;

					DrawBox(box[i], textureID, _mm_set1_ps(1.0f), &atlasPos, i);
				}

				break;
			}
			case BASIC:
			{
				int boxSizeXScaled = static_cast<int>(static_cast<float>(boxSizeX) * WHf);
				int boxSizeYScaled = static_cast<int>(static_cast<float>(boxSizeY) * WHf);

				for (int i = 0; i < 4; i++)
				{
					box[i]->scale = WHf;

					int boxOffsetX = i & 1;
					int boxOffsetY = (i >> 1) & 1;

					box[i]->x = ofWindowWidth / 2 - boxSizeXScaled * boxOffsetX - boxSizeXScaled / 2;
					box[i]->y = ofWindowHeight / 2 + boxSizeYScaled * boxOffsetY - boxSizeYScaled * 2;

					if (i > 1)
					{
						box[i]->x += spread * boxSizeXScaled / 300 * (1 - 2 * boxOffsetX);
					}
				}

				box[0]->rotation = M_PIf * 0.5f;
				box[1]->rotation = M_PIf * 0.5f;

				box[0]->x += boxSizeXScaled / 2;
				box[0]->y += boxSizeYScaled * 3 / 2 + spread * boxSizeXScaled / 300;

				box[1]->x += boxSizeXScaled * 3 / 2;
				box[1]->y += boxSizeYScaled / 2 - spread * boxSizeYScaled / 300;

				for (int i = 0; i < 4; i++)
				{
					atlasPos.left = 0;
					atlasPos.right = boxSizeY;
					atlasPos.top = 0;
					atlasPos.bottom = boxSizeY;

					if (i < 2)
					{
						atlasPos.left += crouchFrame * 3;
						atlasPos.right += crouchFrame * 3;
					}

					DrawBox(box[i], textureID, _mm_set1_ps(1.0f), &atlasPos, i);
				}

				break;
			}
			case CIRCLE:
			{
				if (crouchTime < crouchTimeMax - 0.1f)
				{
					WHf *= sqrtf(camData.crosshairSpread * 0.3f);
				}

				int boxSizeXScaled = static_cast<int>(static_cast<float>(boxSizeX) * WHf);
				int boxSizeYScaled = static_cast<int>(static_cast<float>(boxSizeY) * WHf);

				for (int i = 0; i < 4; i++)
				{
					box[i]->scale = WHf;

					int boxOffsetX = i & 1;
					int boxOffsetY = (i >> 1) & 1;

					box[i]->x = ofWindowWidth / 2 - boxSizeXScaled * boxOffsetX - boxSizeXScaled / 2;

					if (crouchTime >= crouchTimeMax - 0.1f)
					{
						box[i]->x += static_cast<int>(camData.crosshairSpread * 20.0f) * boxSizeXScaled / 300 * (1 - 2 * boxOffsetX);
					}

					box[i]->y = ofWindowHeight / 2 + boxSizeYScaled * boxOffsetY - (boxSizeYScaled * 3) / 2;

					DrawBox(box[i], textureID, _mm_set1_ps(1.0f), &atlasPos, i);
				}

				break;
			}
			case DOT:
			{
				box[0]->scale = WHf * 0.7f;

				box[0]->x = ofWindowWidth / 2 - boxSizeX / 2;
				box[0]->y = ofWindowHeight / 2 - boxSizeY / 2;

				DrawBox(box[0], textureID, _mm_set1_ps(1.0f));

				break;
			}
			}
		}
	}
}

void Crosshair::Draw()
{
	float dT;

	if (ReadUnsafePtr(TraverseUnsafePtr<float>(dT_ptr), dT))
	{
		this->time = std::clamp(this->time + dT, 0.0f, 0.05f);
	}
	else
	{
		this->time = 0.0f;
	}
}