#include "slthpch.h"
#include "Sloth/Renderer/RenderCommand.h"

namespace Sloth {

	Scope<RendererAPI> RenderCommand::s_RendererAPI = RendererAPI::Create();

}