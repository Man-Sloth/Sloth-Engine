#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <map>
#include "Sloth/Scene/Components.h"
#include "Sloth/Scene/Scene.h"
#include "Sloth.h"

namespace Sloth {

	struct state {
		std::string name = "";
		bool isEmpty = true;
		bool hasSRC = false;
		bool hasAC = false;
		
		TransformComponent transformC;
		SpriteRendererComponent spriteRendererC;
		AnimatorComponent animatorC;
		int layer = 0;
	};

	class Memento {
	public:
		Memento() {}
		Memento(Ref<Scene> scene);
		~Memento() {}
		
		// Save a paint stroke or attribute change to later undo
		void SaveCell(std::vector<std::string>& cells, std::vector<state>& s);

		// Perform undo action to previous state
		void UndoCell(std::vector<std::vector<Entity>>& objects, std::vector<std::vector<glm::vec2>>& oCoords, glm::vec2 cellSize);

	private:
		std::map < std::string, std::vector<state> > cellMap; // map to show each cell's change history
		std::vector< std::vector< std::string >> cellChunks; // List of a list of each cell changed per Save
		Ref<Scene> m_Scene; // Current scene to be altered
	
	};
}
