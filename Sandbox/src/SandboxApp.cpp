#include <Sloth.h>
#include <Sloth/Core/EntryPoint.h>

#include "Sandbox2D.h"
#include "ExampleLayer.h"

class Sandbox : public Sloth::Application
{
public:
	Sandbox(const Sloth::ApplicationSpecification& specification)
		: Sloth::Application(specification)
	{
		// PushLayer(new ExampleLayer());
		PushLayer(new Sandbox2D());
	}

	~Sandbox()
	{
	}
};

Sloth::Application* Sloth::CreateApplication(Sloth::ApplicationCommandLineArgs args)
{
	ApplicationSpecification spec;
	spec.Name = "Sandbox";
	spec.WorkingDirectory = "../Sloth-Editor";
	spec.CommandLineArgs = args;

	return new Sandbox(spec);
}
