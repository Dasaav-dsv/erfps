#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <d3d12.h>
#include <d3d11.h>
#include <d3d11on12.h>
#include <dxgi1_4.h>
#include <fstream>
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler")
#include <DirectXMath.h>
#include <wrl/client.h>
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <vector>
#include <comdef.h>

#include "ID3DRenderer.h"
#include "IRenderCallback.h"
#include "Logger.h"

// D3D11 renderer with support for D3D12 using D3D11On12
class Renderer : public ID3DRenderer
{
public:
	void OnPresent(IDXGISwapChain* pThis, UINT syncInterval, UINT flags);
	void OnResizeBuffers(IDXGISwapChain* pThis, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT newFormat, UINT swapChainFlags);
	void AddRenderCallback(IRenderCallback* object);
	void SetCommandQueue(ID3D12CommandQueue* commandQueue);
	void SetGetCommandQueueCallback(void (*callback)());

private:
	Logger logger{"Renderer"};
	HWND window = NULL;

	std::vector<IRenderCallback*> callbacks = {};

	void (*callbackGetCommandQueue)();
	bool mustInitializeD3DResources = true;
	bool firstTimeInitPerformed = false;
	bool isDeviceRetrieved = false;
	bool isRunningD3D12 = false;
	bool getCommandQueueCalled = false;
	int windowWidth = 0;
	int windowHeight = 0;
	UINT bufferIndex = 0;
	UINT bufferCount = 0;

	Microsoft::WRL::ComPtr<ID3D12Device> d3d12Device = nullptr;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3d11Context = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Device> d3d11Device = nullptr;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
	Microsoft::WRL::ComPtr<ID3D11On12Device> d3d11On12Device = nullptr;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> d3d12RenderTargets;
	std::vector<Microsoft::WRL::ComPtr<ID3D11Resource>> d3d11WrappedBackBuffers;
	std::vector<Microsoft::WRL::ComPtr<ID3D11RenderTargetView>> d3d11RenderTargetViews;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain = nullptr;
	Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain3 = nullptr;
	std::shared_ptr<DirectX::SpriteBatch> spriteBatch = nullptr;
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	D3D11_VIEWPORT viewport;

	// Load the shaders from disk at compile time into a string.
	const char* shaderData = 
	{
		#include "Shaders.hlsl"
	};

	std::vector<Microsoft::WRL::ComPtr<ID3D11PixelShader>> shaders = {};

	enum PS_Shader : int32_t
	{
		GLOW_GAUSS = 0,
		SATURATE = 1,
		INTENSITY = 2,
	};

	bool InitD3DResources(IDXGISwapChain* swapChain);
	bool RetrieveD3DDeviceFromSwapChain();
	void GetSwapChainDescription();
	void GetBufferCount();
	void GetSwapchainWindowInfo();
	void CreateViewport();
	void InitD3D();
	void InitD3D11();
	void CreateD3D11Context();
	void CreateSpriteBatch();
	void CreateD3D11RenderTargetView();
	void InitD3D12();
	void CreateD3D11On12Device();
	void CreateD3D12Buffers();
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateD3D12RtvHeap();
	void CreateD3D12RenderTargetView(UINT bufferIndex, D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle);
	void CreateD3D11WrappedBackBuffer(UINT bufferIndex);
	void CreateD3D11RenderTargetViewWithWrappedBackBuffer(UINT bufferIndex);
	bool WaitForCommandQueueIfRunningD3D12();
	void Render();
	void PreRender();
	void RenderCallbacks();
	void PostRender();
	void LoadShader(const char* shader, std::string targetShaderVersion, int32_t shaderID);
	void ReleaseViewsBuffersAndContext();
	bool CheckSuccess(HRESULT hr);
};