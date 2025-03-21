#include "slthpch.h"
#include "Sloth/Renderer/GraphicsContext.h"

#include "Sloth/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLContext.h"

namespace Sloth {

	Scope<GraphicsContext> GraphicsContext::Create(void* window)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:    SLTH_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:  return CreateScope<OpenGLContext>(static_cast<GLFWwindow*>(window));
		}

		SLTH_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}