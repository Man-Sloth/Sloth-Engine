#pragma once

#include "Sloth/Core/KeyCodes.h"
#include "Sloth/Core/MouseCodes.h"

#include <glm/glm.hpp>

namespace Sloth {

	class Input
	{
	public:
		static bool IsKeyPressed(KeyCode key);

		static bool IsMouseButtonPressed(MouseCode button);
		static glm::vec2 GetMousePosition();
		static float GetMouseX();
		static float GetMouseY();
	};
}
