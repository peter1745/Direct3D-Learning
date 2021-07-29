#undef UNICODE
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <d3d11.h>
#include <d3d11_1.h>
#include <d3d11_2.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <wrl/client.h>

template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

ComPtr<ID3D11Device> g_D3DBaseDevice;
ComPtr<ID3D11DeviceContext> g_D3DBaseDeviceContext;
ComPtr<ID3D11Device1> g_D3DDevice;
ComPtr<ID3D11DeviceContext1> g_D3DDeviceContext;
ComPtr<IDXGISwapChain1> g_SwapChain;
ComPtr<ID3D11RenderTargetView> g_RenderTargetView;
ComPtr<ID3D11DepthStencilView> g_DepthBufferView;
ComPtr<ID3D11RasterizerState1> g_RasterizerState;
ComPtr<ID3D11SamplerState> g_SamplerState;
ComPtr<ID3D11DepthStencilState> g_DepthStencilState;
D3D11_VIEWPORT g_Viewport;

UINT g_Width = 1280;
UINT g_Height = 720;

HWND g_WindowHandle;

bool InitializeD3D11();

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	const char* CLASS_NAME = "D3DWindowClass";

	WNDCLASS wc = { };
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	g_WindowHandle = CreateWindowEx(
		0,
		CLASS_NAME,
		"D3D11 Program",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, g_Width, g_Height,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (g_WindowHandle == NULL)
		return 0;

	if (!InitializeD3D11())
	{
		if (MessageBox(NULL, "Failed to initialize Direct3D11!", "Error", MB_OK | MB_ICONERROR) == IDOK)
		{
			PostQuitMessage(0);
		}
	}

	ShowWindow(g_WindowHandle, nCmdShow);

	bool should_close = false;

	const float backgroundColor[4] = { 0.2f, 0.3f, 0.8f, 1.0f };

	MSG msg = {};
	while (!should_close)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		g_D3DDeviceContext->ClearRenderTargetView(g_RenderTargetView.Get(), backgroundColor);
		g_D3DDeviceContext->ClearDepthStencilView(g_DepthBufferView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

		g_D3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		g_D3DDeviceContext->RSSetViewports(1, &g_Viewport);
		g_D3DDeviceContext->RSSetState(g_RasterizerState.Get());

		g_D3DDeviceContext->OMSetRenderTargets(1, g_RenderTargetView.GetAddressOf(), g_DepthBufferView.Get());
		g_D3DDeviceContext->OMSetDepthStencilState(g_DepthStencilState.Get(), 0);
		g_D3DDeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);

		g_SwapChain->Present(1, 0);

		if (msg.message == WM_QUIT)
			break;
	}

	return 0;
}

#define DX_CHECK(x) { if (!SUCCEEDED(x)) { __debugbreak(); } }

bool InitializeD3D11()
{
	D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };

	DX_CHECK(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_BGRA_SUPPORT, featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &g_D3DBaseDevice, nullptr, &g_D3DBaseDeviceContext));

	DX_CHECK(g_D3DBaseDevice->QueryInterface(__uuidof(ID3D11Device1), (void**)&g_D3DDevice));
	DX_CHECK(g_D3DBaseDeviceContext->QueryInterface(__uuidof(ID3D11DeviceContext1), (void**)&g_D3DDeviceContext));

	IDXGIDevice1* dxgiDevice;
	DX_CHECK(g_D3DDevice->QueryInterface(__uuidof(IDXGIDevice1), (void**)&dxgiDevice));
	IDXGIAdapter* dxgiAdapter;
	DX_CHECK(dxgiDevice->GetAdapter(&dxgiAdapter));

	IDXGIFactory2* dxgiFactory;
	dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), (void**)&dxgiFactory);

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
	swapChainDesc.Width = g_Width;
	swapChainDesc.Height = g_Height;
	swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
	swapChainDesc.Stereo = FALSE;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapChainDesc.Flags = 0;

	DX_CHECK(dxgiFactory->CreateSwapChainForHwnd(g_D3DDevice.Get(), g_WindowHandle, &swapChainDesc, nullptr, nullptr, &g_SwapChain));

	ID3D11Texture2D* frameBuffer;
	DX_CHECK(g_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&frameBuffer));
	DX_CHECK(g_D3DDevice->CreateRenderTargetView(frameBuffer, nullptr, &g_RenderTargetView));

	D3D11_TEXTURE2D_DESC depthBufferDesc;
	frameBuffer->GetDesc(&depthBufferDesc);
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	ID3D11Texture2D* depthBuffer;
	DX_CHECK(g_D3DDevice->CreateTexture2D(&depthBufferDesc, nullptr, &depthBuffer));
	DX_CHECK(g_D3DDevice->CreateDepthStencilView(depthBuffer, nullptr, &g_DepthBufferView));

	// Rasterizer
	D3D11_RASTERIZER_DESC1 rasterizerDesc = {};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	DX_CHECK(g_D3DDevice->CreateRasterizerState1(&rasterizerDesc, &g_RasterizerState));

	// Sampler
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	DX_CHECK(g_D3DDevice->CreateSamplerState(&samplerDesc, &g_SamplerState));

	// Depth Stencil State
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilDesc.StencilEnable = FALSE;
	DX_CHECK(g_D3DDevice->CreateDepthStencilState(&depthStencilDesc, &g_DepthStencilState));

	g_Viewport = { 0.0f, 0.0f, (float)g_Width, (float)g_Height, 0.0f, 1.0f };

	return true;
}
