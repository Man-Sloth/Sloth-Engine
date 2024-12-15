#pragma once

#include "Sloth/Core/Layer.h"

#include "Sloth/Events/ApplicationEvent.h"
#include "Sloth/Events/KeyEvent.h"
#include "Sloth/Events/MouseEvent.h"

namespace Sloth {

	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(Event& e) override;

		void Begin();
		void End();

		void BlockEvents(bool block) { m_BlockEvents = block; }
		
		void SetDarkThemeColors();
	private:
		bool m_BlockEvents = true;
	};

}
