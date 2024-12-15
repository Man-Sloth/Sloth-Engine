#pragma once
#define OEMRESOURCE

#include "Sloth.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"
#include "Panels/TilemapPanel.h"
#include "Panels/TileInspectorPanel.h"
#include "Panels/AnimationPanel.h"
#include "Panels/AnimatorPanel.h"
#include "Sloth/Core/Memento.h"
#include <imgui/imgui.h>

#include "Sloth/Renderer/EditorCamera.h"
#include <GLFW/include/GLFW/glfw3.h>

namespace Sloth {

	typedef enum { PAINT, PAINTOVER, ERASE, FLOAT } TYPE;

	struct History
	{
		
		int layer;
		TYPE type;
		std::vector <std::string> names;
		//std::vector <glm::vec2> centerPoints;
		//std::vector <Ref<Texture2D>> OGSprites;
		std::vector <glm::vec2> positions;
		//std::vector<int> indexes;
		std::vector<Entity> objects;
		std::vector<UUID> ids;
		std::vector<TransformComponent> tc;
		std::vector<SpriteRendererComponent> src;
		std::vector<AnimatorComponent> ac;

		std::vector<SpriteRendererComponent> oldSRC;
		std::vector<AnimatorComponent> oldAC;

		std::vector<bool> hasSRC;
		std::vector<bool> hasAC;
		std::vector<bool> hadSRC;
		std::vector<bool> hadAC;
		std::vector<bool> replaceTile;

	};

	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		virtual ~EditorLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		void OnUpdate(Timestep ts) override;
		virtual void OnImGuiRender() override;
		void OnEvent(Event& e) override;
	private:
		bool OnKeyPressed(KeyPressedEvent& e);
		bool OnMouseButtonPressed(MouseButtonPressedEvent& e);
		
		void OnOverlayRender();

		void NewScene();
		void OpenScene();
		void OpenScene(const std::filesystem::path& path);
		void SaveScene();
		void SaveSceneAs();

		void SerializeScene(Ref<Scene> scene, const std::filesystem::path& path);

		void OnScenePlay();
		void OnSceneSimulate();
		void OnSceneStop();

		void OnDuplicateEntity();

		Entity PlaceSpriteObject(const std::filesystem::path& path);
		Entity PlaceSpriteObject(Ref<Texture2D> image);
		void ObjectPreview();
		glm::vec3 MouseToZPlane();

		GLFWcursor* CreateCursor(int tool);
		void SetCursor(GLFWcursor* cursor);

		// UI Panels
		void UI_Toolbar();
	private:
		Sloth::OrthographicCameraController m_CameraController;

		// Temp
		Ref<VertexArray> m_SquareVA;
		Ref<Shader> m_FlatColorShader;
		Ref<Framebuffer> m_Framebuffer;

		Ref<Scene> m_ActiveScene;
		Ref<Scene> m_EditorScene;
		std::filesystem::path m_EditorScenePath;
		Entity m_SquareEntity;
		Entity m_CameraEntity;
		Entity m_SecondCamera;
		
		Entity m_HoveredEntity;

		bool m_PrimaryCamera = true;

		EditorCamera m_EditorCamera;

		Ref<Texture2D> m_CheckerboardTexture;

		bool m_ViewportFocused = false, m_ViewportHovered = false;
		bool m_clickedInViewPort = false;
		glm::vec2 m_ViewportSize = { 0.0f, 0.0f };
		glm::vec2 m_ViewportBounds[2];
		glm::vec2 m_ViewportPosition;
		glm::vec2 m_SnapPos = { 0.0f, 0.0f };
		glm::vec2 m_PrevCellSize = { 0.0f, 0.0f };

		glm::vec4 m_SquareColor = { 0.2f, 0.3f, 0.8f, 1.0f };

		int m_GizmoType = -1;
		int m_PrevLayer = 0;
		float m_SnapX = 1.2f, m_SnapY = 0.65f;

		bool m_ShowPhysicsColliders = false;
		bool m_Editor2D = true;
		bool m_ShowGrid = false;
		bool m_MouseDownLastFrame = false;
		bool m_MouseReleased;
		bool m_ViewportLastFrame = false;
		bool m_WantToSaveWindow = false;
		bool m_LinkedGrids = true;
		bool m_PrevLinkedGrids = true;
		bool setMouseConfig = false;

		bool showSettingsPanel = false;
		bool showStatsPanel = true;

		bool playAnimations = true;

		enum class SceneState
		{
			Edit = 0, Play = 1, Simulate = 2
		};
		SceneState m_SceneState = SceneState::Edit;

		// Panels
		SceneHierarchyPanel m_SceneHierarchyPanel;
		ContentBrowserPanel m_ContentBrowserPanel;
		TilemapPanel m_TilemapPanel;
		TileInspectorPanel m_TileInspectorPanel;
		AnimationPanel m_AnimationPanel;
		AnimatorPanel m_AnimatorPanel = AnimatorPanel(m_AnimationPanel);

		// Editor resources
		Ref<Texture2D> m_IconPlay, m_IconStop, m_Button2D, m_Button2DOff, m_GreenBox, m_GreyBox, m_IconSimulate;
		Ref<Texture2D> m_Grid, m_GridGrey, m_CurrentTile, m_DiamondWhite;

		std::vector<Ref<Texture2D>> m_CurrentTiles;
		std::vector<Animation> m_CurrentAnimations;
		
		
		HICON m_BrushIcon, m_EraserIcon, m_ArrowIcon;
		HCURSOR m_BrushCursor, m_EraserCursor, m_ArrowCursor;
		ICONINFO m_Ico;

		ImGuiIO m_DefaultIO;

		GLFWcursor* cursorBrush;
		GLFWcursor* cursorEraser;
		GLFWcursor* cursorDrop;
		GLFWcursor* cursorSelect;
		GLFWcursor* cursorArrow;
		GLFWcursor* cursorHResize;
		GLFWcursor* cursorVResize;
		GLFWcursor* cursorIBeam;
		GLFWcursor* cursorDrag;

		int cursorID;
		int m_HistoryIndex = 0;
		int m_Count = 0;
		//std::vector<glm::vec2> m_ObjectCoords;
		//std::vector<Entity> m_Objects;
		std::vector < std::vector<glm::vec2> > m_ObjectCoords = std::vector< std::vector<glm::vec2> >(1);
		std::vector < std::vector<Entity> > m_Objects = std::vector < std::vector<Entity> >(1);

		//Undo variables
		Memento m_Memento;
		std::vector<std::string> m_Cells;
		std::vector<state> m_CellStates;

		std::vector<History> m_History;
		History m_curHistory;

		//fps variables
		float elapsed = 0.0f;
		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();;
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		int fps = 0;
		int fps_display = 0;

	};

}
