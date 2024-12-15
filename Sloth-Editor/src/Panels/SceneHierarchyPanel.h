#pragma once

#include "Sloth/Core/Base.h"
#include "Sloth/Scene/Scene.h"
#include "Sloth/Scene/Entity.h"

namespace Sloth {

	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const Ref<Scene>& scene);

		void SetContext(const Ref<Scene>& scene);

		void OnImGuiRender();

		Entity GetSelectedEntity() const { return m_SelectionContext; }
		void SetSelectedEntity(Entity entity);
		void DeselectEntity();
		void SetTilemapData(std::vector<std::string> tmap);

		void SetShowHierarchyPanel(bool show);
		void SetShowPropertiesPanel(bool show);

	private:
		template<typename T>
		void DisplayAddComponentEntry(const std::string& entryName);
	
		void DrawEntityNode(Entity entity);
		void DrawTmapNodes();
		void DrawComponents(Entity entity);

	private:
		Ref<Scene> m_Context;
		Entity m_SelectionContext;
		Entity m_NextSelectedEntity;
		std::vector<std::string> m_Tilemap;
		bool m_CenterLastFrame = false;
		bool changedSelected = false;
		bool showHierarchyPanel = true;
		bool showPropertiesPanel = true;
	};

}
