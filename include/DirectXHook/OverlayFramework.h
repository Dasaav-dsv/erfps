#pragma once

#include <d3d11.h>
#include <vector>
#include <fstream>
#include <DDSTextureLoader.h>
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <wincodec.h>
#include <comdef.h>
#include <combaseapi.h>
#include <wrl/client.h>
#include <iostream>
#include <chrono>
#include <string>
#include "Logger.h"

#include "../../resource/resource.h"

#include "../zlib-ng-win-x86-64/include/zlib-ng.h"
#pragma comment(lib, "zlibstatic-ng")

#undef DrawText

namespace OF
{
	struct Box
	{
		int x = 0;
		int y = 0;
		float z = 0;
		int width = 0;
		int height = 0;
		float scale = 1.0f;
		float rotation = 0.0f;
		bool pressed = false; // Whether the box is currently being pressed (left mouse button held down)
		bool clicked = false; // Whether the box has been clicked this frame (left mouse button pressed and then released)
		bool hover = false; // Whether the cursor is currently hovering over this box
		bool draggable = true;
		bool visible = false; // Managed by the framework, do not change 
		Box* parentBox = nullptr;
	};

	inline Logger ofLogger{ "OverlayFramework" };
	inline HWND ofWindow = 0;
	inline int ofWindowWidth = 0;
	inline int ofWindowHeight = 0;
	inline std::vector<Box*> ofBoxes = std::vector<Box*>();
	inline std::vector<int> ofBoxOrder = std::vector<int>();

	inline ID3D11Device* ofDevice = nullptr;
	inline std::shared_ptr<DirectX::SpriteBatch> ofSpriteBatch = nullptr;
	inline std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> ofTextures = std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>>();
	inline std::vector<std::shared_ptr<DirectX::SpriteFont>> ofFonts = std::vector<std::shared_ptr<DirectX::SpriteFont>>();
	inline std::shared_ptr<DirectX::SpriteFont> ofActiveFont = nullptr;

	inline bool ofInitialized = false;
	inline bool ofFailedToLoadBlank = false;

	// Gives the framework the required DirectX objects to draw
	inline void InitFramework(Microsoft::WRL::ComPtr<ID3D11Device> device, std::shared_ptr<DirectX::SpriteBatch> spriteBatch, HWND window)
	{
		if (!ofInitialized)
		{
			ofLogger.Log("Initialized");
			ofDevice = device.Get();
			ofLogger.Log("ofDevice: %p", ofDevice);
			ofSpriteBatch = spriteBatch;
			ofWindow = window;

			RECT hwndRect;
			GetClientRect(ofWindow, &hwndRect);
			ofWindowWidth = hwndRect.right - hwndRect.left;
			ofWindowHeight = hwndRect.bottom - hwndRect.top;

			ofInitialized = true;
		}
	}

	inline int MapIntToRange(int number, int inputStart, int inputEnd, int outputStart, int outputEnd)
	{
		return outputStart + (outputEnd - outputStart) * (number - inputStart) / (inputEnd - inputStart);
	}

	inline float MapFloatToRange(float number, float inputStart, float inputEnd, float outputStart, float outputEnd)
	{
		return outputStart + (outputEnd - outputStart) * (number - inputStart) / (inputEnd - inputStart);
	}

	extern int LoadBlank();

	extern int LoadTexture(DWORD resourceID = 0);

	inline void PlaceOnTop(Box* boxOnTop)
	{
		size_t boxIndex = 0;
		for (int i = 0; i < ofBoxes.size(); i++)
		{
			if (ofBoxes[i] == boxOnTop)
			{
				boxIndex = i;
				break;
			}
		}

		ofBoxOrder.push_back(boxIndex);
		for (int i = 0; i < ofBoxOrder.size() - 1; i++)
		{
			if (ofBoxes[ofBoxOrder[i]] == ofBoxes[ofBoxOrder.back()])
			{
				ofBoxOrder.erase(ofBoxOrder.begin() + i);
			}
		}

		for (int i = 0; i < ofBoxOrder.size(); i++)
		{
			ofBoxes[ofBoxOrder[i]]->z = 1.0f / (1.0f + (static_cast<float>(i) / 1000.0f));
		}
	}

	inline POINT GetAbsolutePosition(Box* box)
	{
		if (box == nullptr)
		{
			return { 0, 0 };
		}

		POINT absolutePosition = { box->x, box->y };
		Box* parentBox = box->parentBox;
		while (parentBox != nullptr)
		{
			if (parentBox->parentBox == box)
			{
				break;
			}

			absolutePosition.x += parentBox->x;
			absolutePosition.y += parentBox->y;

			parentBox = parentBox->parentBox;
		}

		return absolutePosition;
	}

	inline Box* CreateBox(Box* parentBox, int x, int y, int width, int height)
	{
		Box* box = new Box;
		box->x = x;
		box->y = y;
		box->width = width;
		box->height = height;
		box->parentBox = parentBox;

		if (parentBox != nullptr)
		{
			box->draggable = false;
		}

		ofBoxes.push_back(box);
		PlaceOnTop(box);
		return ofBoxes.back();
	}

	inline Box* CreateBox(int x, int y, int width, int height)
	{
		return CreateBox(nullptr, x, y, width, height);
	}

	inline void _DrawBox(Box* box, DirectX::XMVECTOR color, int textureID, const RECT* source, uint32_t spriteEffects)
	{
		if (box == nullptr)
		{
			ofLogger.Log("Attempted to render a nullptr Box!");
			return;
		}

		if (ofSpriteBatch == nullptr)
		{
			ofLogger.Log("Attempted to render with ofSpriteBatch as nullptr! Run InitFramework before attempting to draw!");
			return;
		}

		if (ofTextures.size() < 1)
		{
			if (ofFailedToLoadBlank == false)
			{
				if (LoadBlank() != 0)
				{
					ofFailedToLoadBlank = true;
					return;
				}
			}
			else
			{
				return;
			}
		}

		if (textureID < 0 || textureID > ofTextures.size() - 1)
		{
			ofLogger.Log("'%i' is an invalid texture ID!", textureID);
			return;
		}

		POINT position = GetAbsolutePosition(box);

		const float heightf = static_cast<float>(box->height) * box->scale;
		const float widthf = static_cast<float>(box->width) * box->scale;

		const int height = static_cast<int>(heightf);
		const int width = static_cast<int>(widthf);

		RECT rect = {};

		if (!source)
		{
			rect.top = position.y + box->height / 2 + box->height % 2;
			rect.left = position.x + box->width / 2 + box->width % 2;
			rect.bottom = rect.top + height;
			rect.right = rect.left + width;

			box->visible = true;
			ofSpriteBatch->Draw(ofTextures[textureID].Get(), rect, nullptr, color, box->rotation, DirectX::XMFLOAT2(static_cast<float>(box->width) * 0.5f, static_cast<float>(box->height) * 0.5f), DirectX::SpriteEffects(spriteEffects), box->z);
		}
		else
		{
			rect.top = position.y + height / 2 + height % 2;
			rect.left = position.x + width / 2 + width % 2;
			rect.bottom = rect.top + height;
			rect.right = rect.left + width;

			box->visible = true;
			ofSpriteBatch->Draw(ofTextures[textureID].Get(), rect, source, color, box->rotation, DirectX::XMFLOAT2(), DirectX::SpriteEffects(spriteEffects), box->z);
		}
	}

	inline void DrawBox(Box* box, int textureID, DirectX::XMVECTOR color = _mm_set1_ps(1.0f), const RECT* source = nullptr, uint32_t spriteEffects = 0)
	{
		_DrawBox(box, color, textureID, source, spriteEffects);
	}
}