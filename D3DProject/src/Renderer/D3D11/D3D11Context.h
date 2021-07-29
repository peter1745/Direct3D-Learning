#pragma once

#include "Renderer/RendererContext.h"

#include <d3d11.h>
#include <d3d11_1.h>
#include <d3d11_2.h>
#include <dxgi.h>
#include <d3dcompiler.h>

namespace Renderer {

	class D3D11Context : public RendererContext
	{
	public:
		D3D11Context(HWND windowHandle);
		virtual ~D3D11Context();

		virtual void Init() override;

	private:
		HWND m_WindowHandle;
		ID3D11Device1* m_Device;
		ID3D11DeviceContext1* m_DeviceContext;

	};

}
