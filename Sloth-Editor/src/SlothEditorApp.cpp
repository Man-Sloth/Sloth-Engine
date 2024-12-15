#include <Sloth.h>
#include <Sloth/Core/EntryPoint.h>

#include "EditorLayer.h"

namespace Sloth {

	class SlothEditor : public Application
	{
	public:
		SlothEditor(const ApplicationSpecification& spec)
			: Application(spec)
		{
			PushLayer(new EditorLayer());
		}
	};

	Application* CreateApplication(ApplicationCommandLineArgs args)
	{
		ApplicationSpecification spec;
		spec.Name = "Sloth-Editor";
		spec.CommandLineArgs = args;

		return new SlothEditor(spec);
	}

}
