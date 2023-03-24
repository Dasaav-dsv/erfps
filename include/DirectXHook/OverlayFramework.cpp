#include "OverlayFramework.h"

namespace OF
{
	extern int LoadBlank()
	{
		if (ofDevice == nullptr)
		{
			ofLogger.Log("Could not load texture, ofDevice is nullptr! Run InitFramework before attempting to load textures!");
			return -1;
		}

		ID3D11Texture2D* pTexture = nullptr;
		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = 1;
		desc.Height = 1;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		uint8_t pixelData[] = { 255, 255, 255, 255 };
		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = pixelData;
		initData.SysMemPitch = sizeof(uint8_t) * 4;
		initData.SysMemSlicePitch = 0;

		HRESULT hr = ofDevice->CreateTexture2D(&desc, &initData, &pTexture);

		if (FAILED(hr))
		{
			ofLogger.Log("Error %#010x when creating blank texture", hr);

			ofLogger.Log("Texture loading failed: blank");
			return -1;
		}

		HRESULT hrCOM = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pSRV = nullptr;
		hr = ofDevice->CreateShaderResourceView(pTexture, nullptr, pSRV.GetAddressOf());

		if (hrCOM == 0)
		{
			CoUninitialize();
		}

		if (FAILED(hr))
		{
			ofLogger.Log("Error %#010x when creating blank texture shader resource view", hr);

			ofLogger.Log("Texture loading failed: blank");
			return -1;
		}

		ofLogger.Log("Texture loaded: blank");

		ofTextures.push_back(pSRV);

		return ofTextures.size() - 1;
	}

	extern int LoadTexture(DWORD resourceID)
	{
		if (ofDevice == nullptr)
		{
			ofLogger.Log("Could not load texture, ofDevice is nullptr! Run InitFramework before attempting to load textures!");
			return -1;
		}

		if (!resourceID)
		{
			return LoadBlank();
		}

		HMODULE hModule = GetModuleHandleA("erfps.dll");
		HRSRC hResource = FindResourceA(hModule, MAKEINTRESOURCEA(resourceID), "ZDDS");
		HGLOBAL hResourceData = LoadResource(hModule, hResource);
		LPVOID pResourceData = LockResource(hResourceData);
		DWORD dwResourceSize = SizeofResource(hModule, hResource);

		WCHAR tmpPath[L_tmpnam_s];
		errno_t err = _wtmpnam_s(tmpPath, L_tmpnam_s);

		if (err)
		{
			ofLogger.Log("Texture loading failed: failed to create temporary file name");
			return -1;
		}

		size_t compressedSize = dwResourceSize;
		size_t maxDecompressedSize = 0x100000;
		uint8_t* decompressedData = new uint8_t[maxDecompressedSize];

		if (zng_uncompress(decompressedData, &maxDecompressedSize, reinterpret_cast<uint8_t*>(pResourceData), compressedSize))
		{
			ofLogger.Log("Texture loading failed: failed to decompress texture");

			delete[] decompressedData;
			return -1;
		}

		FILE* tmpFile;

		err = _wfopen_s(&tmpFile, tmpPath, L"wb");

		if (err)
		{
			ofLogger.Log("Texture loading failed: failed to create temporary file");
			return -1;
		}

		fwrite(decompressedData, maxDecompressedSize, 1, tmpFile);
		fclose(tmpFile);
		delete[] decompressedData;

		HRESULT hrCOM = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture = nullptr;
		HRESULT hr = DirectX::CreateDDSTextureFromFile(ofDevice, tmpPath, nullptr, texture.GetAddressOf());

		DeleteFileW(tmpPath);

		if (hrCOM == S_OK)
		{
			CoUninitialize();
		}

		if (FAILED(hr))
		{
			ofLogger.Log("Error %#010x when loading texture", hr);

			ofLogger.Log("Texture loading failed: ID %d", resourceID);
			return -1;
		}

		ofLogger.Log("Texture loaded: ID %d", resourceID);

		ofTextures.push_back(texture);

		return ofTextures.size() - 1;
	}
}