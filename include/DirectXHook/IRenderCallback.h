#pragma once

#include <string>
#include <d3d11.h>
#include <wrl/client.h>
#include <SpriteBatch.h>
#include <SpriteFont.h>

class IRenderCallback
{
public:
	virtual void Setup() {};
	virtual void Render() {};

	void Init(Microsoft::WRL::ComPtr<ID3D11Device> device, Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, std::shared_ptr<DirectX::SpriteBatch> spriteBatch, HWND window)
	{
		this->device = device;
		this->context = context;
		this->spriteBatch = spriteBatch;
		this->window = window;
		this->initialized = true;
	}

	bool IsInit() { return this->initialized; };

	void SetShaderID(int32_t shaderID) { this->shaderID = shaderID; }

	auto GetShaderID() { return this->shaderID; }

	bool SetShaderBuffer(DirectX::XMVECTOR data)
	{
		if (!this->shaderBuffer)
		{
			D3D11_BUFFER_DESC bufferDesc = { 0 };
			bufferDesc.ByteWidth = sizeof(data);
			bufferDesc.Usage = D3D11_USAGE_DEFAULT;
			bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

			this->device->CreateBuffer(&bufferDesc, nullptr, this->shaderBuffer.GetAddressOf());
			this->context->UpdateSubresource(this->shaderBuffer.Get(), 0, nullptr, &data, sizeof(data), 0);

			return false;
		}
		else
		{
			this->context->UpdateSubresource(this->shaderBuffer.Get(), 0, nullptr, &data, sizeof(data), 0);

			return true;
		}
	}

	auto GetShaderBuffer() { return this->shaderBuffer; }

protected:
	Microsoft::WRL::ComPtr<ID3D11Device> device = nullptr;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context = nullptr;
	std::shared_ptr<DirectX::SpriteBatch> spriteBatch = nullptr;
	int32_t shaderID = -1;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> shader = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Buffer> shaderBuffer = nullptr;
	HWND window;
	bool initialized = false;
};