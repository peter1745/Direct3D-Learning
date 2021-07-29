#pragma once

namespace Renderer {

	class RendererContext
	{
	public:
		virtual ~RendererContext() = default;

		virtual void Init() = 0;

		// virtual Shared<Device> GetDevice() const = 0;
	};

}
