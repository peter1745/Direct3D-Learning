#include "D3D11Context.h"

namespace Renderer {

	D3D11Context::D3D11Context(HWND windowHandle)
		: m_WindowHandle(windowHandle)
	{
	}

	D3D11Context::~D3D11Context()
	{
		m_DeviceContext->Release();
		m_Device->Release();
	}

	void D3D11Context::Init()
	{
		D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };

		ID3D11Device* device;
		ID3D11DeviceContext* deviceContext;

		D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_BGRA_SUPPORT, featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &device, nullptr, &deviceContext);

		device->QueryInterface(__uuidof(ID3D11Device1), (void**)&m_Device);
		deviceContext->QueryInterface(__uuidof(ID3D11DeviceContext1), (void**)&m_DeviceContext);

		// Create Swapchain
	}

}
