#include "EditorLayer.h"
#include "Sloth/Scene/SceneSerializer.h"
#include "Sloth/Utils/PlatformUtils.h"
#include "Sloth/Math/Math.h"
#include "Sloth/Core/Input.h"
#include "Sloth/Renderer/AssetManager.h"


#include <imgui/imgui.h>
#include <GLFW/include/GLFW/glfw3.h>
#include "Sloth/Core/Application.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "ImGuizmo.h"
#include <iostream>
#include <format>


using namespace std;

namespace Sloth {

	extern const std::filesystem::path g_AssetPath;
	extern const AssetManager g_AssetManager;

	std::map<std::string, Ref<Texture2D>> AssetManager::m_LoadedTextures = std::map<std::string, Ref<Texture2D>>();
	std::map<std::string, int> AssetManager::m_AmountUsed = std::map<std::string, int>();

	bool compareObjects(Entity e1, Entity e2)
	{
		return(e1.GetComponent<TransformComponent>().Translation.y < e2.GetComponent<TransformComponent>().Translation.y);
	}

	EditorLayer::EditorLayer()
		: Layer("EditorLayer"), m_CameraController(1280.0f / 720.0f), m_SquareColor({ 0.2f, 0.3f, 0.8f, 1.0f })
	{
		//m_AnimationPanel.SetAnimatorPanel(m_AnimatorPanel);
	}

	void EditorLayer::OnAttach()
	{
		SLTH_PROFILE_FUNCTION();

		m_IconPlay = Texture2D::Create("Resources/Icons/Toolbar/PlayButton.png");
		m_IconStop = Texture2D::Create("Resources/Icons/Toolbar/StopButton.png");
		m_Button2D = Texture2D::Create("Resources/Icons/Toolbar/2DButton.png");
		m_Button2DOff = Texture2D::Create("Resources/Icons/Toolbar/2DButtonGrey.png");
		m_GreenBox = Texture2D::Create("Resources/Icons/Toolbar/GreenBox.png");
		m_GreyBox = Texture2D::Create("Resources/Icons/Toolbar/GreyBox.png");
		m_IconSimulate = Texture2D::Create("Resources/Icons/SimulateButton.png");
		m_Grid = Texture2D::Create("Resources/Icons/Toolbar/Grid.png");
		m_GridGrey = Texture2D::Create("Resources/Icons/Toolbar/GridGrey.png");
		m_DiamondWhite = Texture2D::Create("Resources/WhiteDiamond.png");
		m_CurrentTile = NULL;

		//GLFWcursor* cursor = Input::SetCursor();
		cursorBrush = CreateCursor(0);
		cursorEraser = CreateCursor(1);
		cursorDrop = CreateCursor(2);
		cursorSelect = CreateCursor(3);
		cursorDrag = CreateCursor(5);
		cursorArrow = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
		cursorHResize = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
		cursorVResize = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
		cursorIBeam = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);

		cursorID = 0;

		FramebufferSpecification fbSpec;
		fbSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		m_Framebuffer = Framebuffer::Create(fbSpec);

		m_EditorScene = CreateRef<Scene>();
		m_ActiveScene = m_EditorScene;
		m_Memento = Memento(m_ActiveScene);
		
		auto commandLineArgs = Application::Get().GetSpecification().CommandLineArgs;
		if (commandLineArgs.Count > 1)
		{
			auto sceneFilePath = commandLineArgs[1];
			//SceneSerializer serializer(m_ActiveScene, m_Objects, m_ObjectCoords, m_TileInspectorPanel.GetLayerNames(), 
			//	m_TileInspectorPanel.GetBuffers());
			SceneSerializer serializer(
				m_ActiveScene, 
				m_TileInspectorPanel.GetLayerNames(), 
				m_TileInspectorPanel.GetBuffers(),
				m_Objects,
				m_ObjectCoords,
				m_TileInspectorPanel.GetLayerCells());

			serializer.Deserialize(sceneFilePath);
		}

		m_EditorCamera = EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f);
		Renderer2D::SetLineWidth(4.0f);

		NewScene();
	}

	void EditorLayer::OnDetach()
	{
		SLTH_PROFILE_FUNCTION();
		glfwDestroyCursor(cursorBrush);
		glfwDestroyCursor(cursorEraser);
		glfwDestroyCursor(cursorDrop);
		glfwDestroyCursor(cursorSelect);
		glfwDestroyCursor(cursorArrow);
		glfwDestroyCursor(cursorHResize);
		glfwDestroyCursor(cursorVResize);
		glfwDestroyCursor(cursorIBeam);

	}

	void EditorLayer::OnUpdate(Timestep ts)
	{
		SLTH_PROFILE_FUNCTION();
		
		// Resize
		if (FramebufferSpecification spec = m_Framebuffer->GetSpecification();
			m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && // zero sized framebuffer is invalid
			(spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
		{
			m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_CameraController.OnResize(m_ViewportSize.x, m_ViewportSize.y);
			m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
			m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		}

		// Render
		Renderer2D::ResetStats();
		m_Framebuffer->Bind();
		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		RenderCommand::Clear();
		// Clear our entity ID attachment to -1
		m_Framebuffer->ClearAttachment(1, -1);

		switch (m_SceneState)
		{
			case SceneState::Edit:
			{
				if (m_ViewportFocused)
					m_CameraController.OnUpdate(ts);

				m_EditorCamera.OnUpdate(ts, m_Editor2D);

				m_ActiveScene->OnUpdateEditor(ts, m_EditorCamera);

				if (playAnimations)
				{
					for (int i = 0; i < m_Objects.size(); i++)
					{
						for (int j = 0; j < m_Objects[i].size(); j++)
						{
							if (m_Objects[i][j].HasComponent<AnimatorComponent>())
							{
								AnimatorComponent& ac = m_Objects[i][j].GetComponent<AnimatorComponent>();
								if (ac.animator.GetAnimations().size() > 0)
								{
									if(m_Objects[i][j] != m_SceneHierarchyPanel.GetSelectedEntity())
										ac.animator.Update();

								
									if (m_Objects[i][j].HasComponent<SpriteRendererComponent>())
									{
										SpriteRendererComponent& src = m_Objects[i][j].GetComponent<SpriteRendererComponent>();
										src.Texture = ac.animator.GetCurrentAnimation().GetFrame(ac.animator.GetCurrentAnimation().GetCurrentFrame());
										src.CenterPoint = ac.animator.GetCurrentAnimation().GetCenterPoint(ac.animator.GetCurrentAnimation().GetCurrentFrame());
									}
								}
							}
						}
					}
				}

				break;
			}
			case SceneState::Simulate:
			{
				m_EditorCamera.OnUpdate(ts, m_Editor2D);

				m_ActiveScene->OnUpdateSimulation(ts, m_EditorCamera);
				break;
			}
			case SceneState::Play:
			{
				m_ActiveScene->OnUpdateRuntime(ts);
				break;
			}
		}

		auto[mx, my] = ImGui::GetMousePos();
		mx -= m_ViewportBounds[0].x;
		my -= m_ViewportBounds[0].y;
		glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
		my = viewportSize.y - my;
		int mouseX = (int)mx;
		int mouseY = (int)my;

		//if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y)
		//{
			//int pixelData = m_Framebuffer->ReadPixel(1, mouseX, mouseY);
			//m_HoveredEntity = pixelData == -1 ? Entity() : Entity((entt::entity)pixelData, m_ActiveScene.get());
		//}
		
		OnOverlayRender();

		m_Framebuffer->Unbind();
	}

	void EditorLayer::OnImGuiRender()
	{
		SLTH_PROFILE_FUNCTION();
		//ImGui::SetMou
		AssetManager::GetTextureCount();
		// Note: Switch this to true to enable dockspace
		static bool dockspaceOpen = true;
		static bool opt_fullscreen_persistant = true;
		bool opt_fullscreen = opt_fullscreen_persistant;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
		
		m_ActiveScene->SetVisibleLayers(m_TileInspectorPanel.GetVisibleLayers());

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive, 
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise 
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
		ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// DockSpace
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		if (!setMouseConfig) {
			m_DefaultIO.ConfigFlags = io.ConfigFlags;
			setMouseConfig = true;
		}
		
		float minWinSizeX = style.WindowMinSize.x;
		//style.WindowMinSize.x = 370.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		//style.WindowMinSize.x = minWinSizeX;

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				// Disabling fullscreen would allow the window to be moved to the front of other windows, 
				// which we can't undo at the moment without finer window depth/z control.
				//ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen_persistant);1
				if (ImGui::MenuItem("New", "Ctrl+N"))
				{
					m_WantToSaveWindow = true;
					//NewScene();
				}

				if (ImGui::MenuItem("Open...", "Ctrl+O"))
					OpenScene();

				if (ImGui::MenuItem("Save", "Ctrl+S"))
					SaveScene();

				if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
					SaveSceneAs();

				if (ImGui::MenuItem("Exit")) Application::Get().Close();
					ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Tools")) { ImGui::EndMenu(); }
			if (ImGui::BeginMenu("Windows")) 
			{ 
				if (ImGui::MenuItem("Tiles"))
					m_TilemapPanel.SetShow(true);

				if (ImGui::MenuItem("Tilemap Inspector"))
					m_TileInspectorPanel.SetShow(true);

				if (ImGui::MenuItem("Hierarchy"))
					m_SceneHierarchyPanel.SetShowHierarchyPanel(true);

				if (ImGui::MenuItem("Properties"))
					m_SceneHierarchyPanel.SetShowPropertiesPanel(true);

				if (ImGui::MenuItem("Animation"))
					m_AnimationPanel.SetShow(true);

				if (ImGui::MenuItem("Animator Controller"))
					m_AnimatorPanel.SetShow(true);

				if (ImGui::MenuItem("Stats"))
					showStatsPanel = true;

				if (ImGui::MenuItem("Settings"))
					showSettingsPanel = true;

				ImGui::EndMenu(); 
			}
			if (ImGui::BeginMenu("Help")) { ImGui::EndMenu(); }

			ImGui::EndMenuBar();
		}


		if (showStatsPanel)
		{
			ImGui::Begin("Stats", &showStatsPanel);

			std::string name = "None";
			if (m_HoveredEntity)
				name = m_HoveredEntity.GetComponent<TagComponent>().Tag;
			ImGui::Text("Hovered Entity: %s", name.c_str());

			//if (m_ViewportHovered && ImGui::IsWindowFocused() && ImGui::IsMouseDown(ImGuiMouseButton_Left))
			auto stats = Renderer2D::GetStats();
			ImGui::Text("Renderer2D Stats:");

			ImGui::Text("Draw Calls: %d", stats.DrawCalls);
			ImGui::Text("Quads: %d", stats.QuadCount);
			ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
			ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
			ImGui::Text("Current Tile: %s", m_CurrentTile);
			ImGui::Text("Viewport Hovered: %d", m_ViewportHovered);
			ImGui::Text("Window Focused: %d", ImGui::IsWindowFocused());
			ImGui::Text("Mouse Released: %d", ImGui::IsMouseReleased(ImGuiMouseButton_Left));
			ImGui::Text("Mouse Down: %d", ImGui::IsMouseDown(ImGuiMouseButton_Left));
			ImGui::Text("m_Objects size: %d", m_Objects[0].size());

			ImGui::Separator();
			int size = AssetManager::GetTextureCount();
			std::string sizeText = "Texture Count: " + std::to_string(size);
			ImGui::Text(sizeText.c_str());
			std::string fpsText = "FPS: " + std::to_string(fps_display);
			ImGui::Text(fpsText.c_str());
			int layer = m_TileInspectorPanel.GetSelectedLayer();
			std::string layerText = "Selected Layer : " + std::to_string(layer);
			ImGui::Text(layerText.c_str());
			std::string countText = "Count: " + std::to_string(m_Count);
			ImGui::Text(countText.c_str());

			ImGui::End();
		}

		if (showSettingsPanel)
		{
			ImGui::Begin("Settings", &showSettingsPanel);
			ImGui::Checkbox("Show physics colliders", &m_ShowPhysicsColliders);
			ImGui::Checkbox("Show all animations", &playAnimations);
			ImGui::End();
		}

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGui::Begin("Viewport");

		//glm::vec3 mousePos = MouseToZPlane();
		//ImVec2 mousePos = ImGui::GetMousePos();
		//ImGui::Text("Mouse X: %f", mousePos.x);
		//ImGui::Text("Mouse Y: %f", mousePos.y);

		auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
		auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
		auto viewportOffset = ImGui::GetWindowPos();
		m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
		m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

		

		m_ViewportFocused = ImGui::IsWindowFocused();
		m_ViewportHovered = ImGui::IsWindowHovered();
		Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportFocused && !m_ViewportHovered);

		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

		uint64_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
		ImGui::Image(reinterpret_cast<void*>(textureID), ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const wchar_t* path = (const wchar_t*)payload->Data;

				if ((std::filesystem::path(g_AssetPath) / path).extension().string() == ".sloth")
					OpenScene(std::filesystem::path(g_AssetPath) / path);
				else if ((std::filesystem::path(g_AssetPath) / path).extension().string() == ".png")
					PlaceSpriteObject(std::filesystem::path(g_AssetPath) / path);
			}
			ImGui::EndDragDropTarget();
		}

		// Paint,erase,eyedrop, volume drag current tile selected
		if (m_ViewportHovered && ImGui::IsMouseDown(ImGuiMouseButton_Left) && m_Editor2D)
		{
			if (!m_MouseDownLastFrame)
			{
				// if painting starts, start saving every tile changed in that stroke
				m_CellStates.clear();
				m_Cells = std::vector<std::string>();
			}

			if (!m_EditorCamera.IsDragging())
			{
				PlaceSpriteObject(m_CurrentTile);
				Renderer2D::DrawQuad(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(1, 1), m_DiamondWhite);
				m_clickedInViewPort = true;
			}

		}
		else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
		{
			if (m_clickedInViewPort)
				if (m_CurrentTile || (m_CurrentAnimations.size() >0))
				{
					m_HistoryIndex++;
					m_clickedInViewPort = false;

					/*for (int i = 0; i < m_CellStates.size(); i++)
					{
						std::vector< Animation > acv = m_CellStates[i].animatorC.animator.GetAnimations();
						for (int j = 0; j < acv.size(); j++)
						{
							for (int k = 0; k < acv[j].)
						}
					}*/
					if (m_TileInspectorPanel.GetDrawMode() == 0 || m_TileInspectorPanel.GetDrawMode() == 1)
						m_Memento.SaveCell(m_Cells, m_CellStates);
				}
			//cout << "Released";
			//SetCursorPos(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);
		}

		if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
			m_MouseDownLastFrame = true;
		else
			m_MouseDownLastFrame = false;

		if (m_ViewportHovered)
			m_ViewportLastFrame = true;
		else
			m_ViewportLastFrame = false;


		// Change cursor to reflect tool selected
		if (ImGui::IsWindowHovered())
		{
			//ImGui::SetMouseCursor(ImGuiMouseCursor_None);
			auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
			

			io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

			if (m_EditorCamera.IsDragging())
			{
				SetCursor(cursorDrag);
				cursorID = -1;
			}
			else
			{
				// BRUSH = 0, ERASE = 1, VOLUME = 2, SELECT = 3
				if (m_TileInspectorPanel.GetDrawMode() == 0 && m_Editor2D && !ImGui::GetDragDropPayload())
				{
					if (cursorID != 1)
					{

						//cursorID Probably not needed anymore
						cursorID = 1;
						SetCursor(cursorBrush);

					}


				}
				else if (m_TileInspectorPanel.GetDrawMode() == 1 && m_Editor2D && !ImGui::GetDragDropPayload())
				{
					if (cursorID != 2)
					{
						cursorID = 2;
						SetCursor(cursorEraser);
					}

				}
				else if (m_TileInspectorPanel.GetDrawMode() == 3 && m_Editor2D && !ImGui::GetDragDropPayload())
				{
					if (cursorID != 3)
					{
						cursorID = 3;
						SetCursor(cursorDrop);
					}

				}

				else if (m_TileInspectorPanel.GetDrawMode() == 4 && m_Editor2D && !ImGui::GetDragDropPayload())
				{
					if (cursorID != 4)
					{
						cursorID = 4;
						SetCursor(cursorSelect);
					}

				}
			}
		}
		else
		{
			if (cursorID != -1)
			{
				cursorID = -1;
			}
				io.ConfigFlags = m_DefaultIO.ConfigFlags;
		}
		

		// Gizmos
		Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
		if (selectedEntity && m_GizmoType != -1)
		{
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();

			ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y, m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y);

			// Camera
			
			// Runtime camera from entity
			// auto cameraEntity = m_ActiveScene->GetPrimaryCameraEntity();
			// const auto& camera = cameraEntity.GetComponent<CameraComponent>().Camera;
			// const glm::mat4& cameraProjection = camera.GetProjection();
			// glm::mat4 cameraView = glm::inverse(cameraEntity.GetComponent<TransformComponent>().GetTransform());

			// Editor camera
			const glm::mat4& cameraProjection = m_EditorCamera.GetProjection();
			glm::mat4 cameraView = m_EditorCamera.GetViewMatrix();

			// Entity transform
			auto& tc = selectedEntity.GetComponent<TransformComponent>();
			glm::mat4 transform = tc.GetTransform();

			// Snapping
			bool snap = Input::IsKeyPressed(Key::LeftControl);
			float snapValue = 0.5f; // Snap to 0.5m for translation/scale
			// Snap to 45 degrees for rotation
			if (m_GizmoType == ImGuizmo::OPERATION::ROTATE)
				snapValue = 45.0f;

			float snapValues[3] = { snapValue, snapValue, snapValue };

			ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
				(ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform),
				nullptr, snap ? snapValues : nullptr);

			if (ImGuizmo::IsUsing())
			{
				glm::vec3 translation, rotation, scale;
				Math::DecomposeTransform(transform, translation, rotation, scale);

				glm::vec3 deltaRotation = rotation - tc.Rotation;
				tc.Translation = translation;
				tc.Rotation += deltaRotation;
				tc.Scale = scale;
			}
		}

		m_ViewportPosition.x = ImGui::GetWindowPos().x + (m_ViewportSize.x * 0.5);
		m_ViewportPosition.y = ImGui::GetWindowPos().y + (m_ViewportSize.y * 0.5);

		ImGui::End();
		ImGui::PopStyleVar();

		UI_Toolbar();

		m_SceneHierarchyPanel.SetTilemapData(m_TileInspectorPanel.GetLayerNames());

		m_SceneHierarchyPanel.OnImGuiRender();
		//m_ContentBrowserPanel.OnImGuiRender();
		m_TilemapPanel.OnImGuiRender(m_CurrentAnimations, m_CurrentTiles, m_TileInspectorPanel);
		m_TileInspectorPanel.OnImGuiRender(m_CurrentTile, m_CurrentTiles, m_CurrentAnimations, m_ObjectCoords, m_Objects, m_ActiveScene);
		m_AnimationPanel.OnImGuiRender();
		m_AnimatorPanel.OnImGuiRender();

		// New scene verification pop up
		if (m_WantToSaveWindow)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, ImVec2(0.5f, 0.0f));
			ImGuiWindowFlags window_flags_save = 0;
			window_flags_save |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse;
			ImGui::Begin("Do you Want to Save?", NULL, window_flags_save);
			ImGui::PopStyleVar();
			ImVec2 vp_Position; vp_Position.x = m_ViewportPosition.x; vp_Position.y = m_ViewportPosition.y;
			vp_Position.x -= (ImGui::GetWindowWidth() * 0.5f);
			vp_Position.y -= (ImGui::GetWindowHeight() * 0.5f);
			ImGui::SetWindowPos(vp_Position);
			float sizeY = ImGui::GetWindowHeight() / 3.0f;
			float sizeX = sizeY * 3;
			
			ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (sizeX * 0.5f));
			ImGui::SetCursorPosY((ImGui::GetWindowContentRegionMax().y * 0.5f) - (sizeY * 0.1f));

			if (ImGui::Button("OK", ImVec2(sizeX, sizeY)))
			{
				m_WantToSaveWindow = false;
				NewScene();
			}
			ImGui::End();
		}

		end = std::chrono::steady_clock::now();
		elapsed += std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
		if (elapsed >= 1000)
		{
			elapsed = 0;
			fps_display = fps;
			fps = 0;
		}
		begin = std::chrono::steady_clock::now();
		fps++;

		ImGui::End();
	}

	void EditorLayer::UI_Toolbar()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		auto& colors = ImGui::GetStyle().Colors;
		const auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(buttonHovered.x, buttonHovered.y, buttonHovered.z, 0.5f));
		const auto& buttonActive = colors[ImGuiCol_ButtonActive];
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(buttonActive.x, buttonActive.y, buttonActive.z, 0.5f));
		const auto& background = colors[ImGuiCol_TitleBg];
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(background.x, background.y, background.z, background.w));

		ImGui::Begin("##toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

		bool toolbarEnabled = (bool)m_ActiveScene;

		ImVec4 tintColor = ImVec4(1, 1, 1, 1);
		if (!toolbarEnabled)
			tintColor.w = 0.5f;

		float size = ImGui::GetWindowHeight() - 4.0f;
		{
			Ref<Texture2D> icon = m_Editor2D == true ? m_Button2D : m_Button2DOff;
			ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 2));
			if (ImGui::ImageButton((ImTextureID)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 1), ImVec2(1, 0), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) && toolbarEnabled)
			{
				if (!m_Editor2D)
					m_Editor2D = true;
				else
					m_Editor2D = false;
			}
		}
		ImGui::SameLine();
		{
			Ref<Texture2D> icon = (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Simulate) ? m_IconPlay : m_IconStop;
			ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));
			if (ImGui::ImageButton((ImTextureID)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 1), ImVec2(1, 0), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) && toolbarEnabled)
			{
				if (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Simulate)
					OnScenePlay();
				else if (m_SceneState == SceneState::Play)
					OnSceneStop();
			}
		}
		ImGui::SameLine();
		{
			Ref<Texture2D> icon = (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Play) ? m_IconSimulate : m_IconStop;		//ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));
			if (ImGui::ImageButton((ImTextureID)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 1), ImVec2(1, 0), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) && toolbarEnabled)
			{
				if (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Play)
					OnSceneSimulate();
				else if (m_SceneState == SceneState::Simulate)
					OnSceneStop();
			}
		}
		ImGui::SameLine();
		{
			Ref<Texture2D> icon = m_ShowPhysicsColliders == true ? m_GreenBox : m_GreyBox;
			ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) + (size * 2.0f));
			if (ImGui::ImageButton((ImTextureID)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 1), ImVec2(1, 0), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) && toolbarEnabled)
			{
				if (!m_ShowPhysicsColliders)
					m_ShowPhysicsColliders = true;
				else
					m_ShowPhysicsColliders = false;
			}
		}
		ImGui::SameLine();
		{
			Ref<Texture2D> icon = m_ShowGrid == true ? m_Grid : m_GridGrey;
			ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) + (size * 3.4f));
			if (ImGui::ImageButton((ImTextureID)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 1), ImVec2(1, 0), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) && toolbarEnabled)
			{
				if (!m_ShowGrid)
					m_ShowGrid = true;
				else
					m_ShowGrid = false;
			}
		}

		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(4);
		ImGui::End();
	}

	void EditorLayer::OnEvent(Event& e)
	{
		m_CameraController.OnEvent(e);
		if (m_SceneState == SceneState::Edit)
		{
			m_EditorCamera.OnEvent(e, m_ViewportHovered);
		}

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(SLTH_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
		dispatcher.Dispatch<MouseButtonPressedEvent>(SLTH_BIND_EVENT_FN(EditorLayer::OnMouseButtonPressed));
	}

	bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
	{
		// Shortcuts
		if (e.IsRepeat())
			return false;

		bool control = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		bool shift = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);

		switch (e.GetKeyCode())
		{
			case Key::N:
			{
				if (control) {
					//Promt user and ask if they are sure?
					m_WantToSaveWindow = true;
				}

				break;
			}
			case Key::O:
			{
				if (control)
					OpenScene();

				break;
			}
			case Key::S:
			{
				if (control)
				{
					if (shift)
						SaveSceneAs();
					else
						SaveScene();
				}

				break;
			}

			// Scene Commands
			case Key::D:
			{
				if (control)
					OnDuplicateEntity();

				break;
			}

			case Key::Z:
			{
				if (control)
				{
					float x = 1.5;
					int y = x * 2;
					int z = 0;
					m_SceneHierarchyPanel.DeselectEntity();
					m_Memento.UndoCell(m_Objects, m_ObjectCoords, m_TileInspectorPanel.GetCellSize());
				}
			}

			// Gizmos
			case Key::Q:
			{
				if (!ImGuizmo::IsUsing())
					m_GizmoType = -1;
				break;
			}
			case Key::W:
			{
				if (!ImGuizmo::IsUsing())
					m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
				break;
			}
			case Key::E:
			{
				if (!ImGuizmo::IsUsing())
					m_GizmoType = ImGuizmo::OPERATION::ROTATE;
				break;
			}
			case Key::R:
			{
				if (!ImGuizmo::IsUsing())
					m_GizmoType = ImGuizmo::OPERATION::SCALE;
				break;
			}
		}
	}

	bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
	{
		//select tile by left clicking
		bool tileMap = false;

		if (!tileMap)
		{
			if (e.GetMouseButton() == Mouse::ButtonLeft)
			{
				//if (m_ViewportHovered && !ImGuizmo::IsOver() && !Input::IsKeyPressed(Key::LeftAlt))
				//	m_SceneHierarchyPanel.SetSelectedEntity(m_HoveredEntity);
			}
		}
		
		return false;
	}

	void EditorLayer::OnOverlayRender()
	{
		if (m_SceneState == SceneState::Play)
		{
			Entity camera = m_ActiveScene->GetPrimaryCameraEntity();
			if (!camera)
				return;

			Renderer2D::BeginScene(camera.GetComponent<CameraComponent>().Camera, camera.GetComponent<TransformComponent>().GetTransform());
		}
		else
		{
			Renderer2D::BeginScene(m_EditorCamera);
		}

		if (m_ShowPhysicsColliders)
		{
			// Box Colliders
			{
				auto view = m_ActiveScene->GetAllEntitiesWith<TransformComponent, BoxCollider2DComponent>();
				for (auto entity : view)
				{
					auto [tc, bc2d] = view.get<TransformComponent, BoxCollider2DComponent>(entity);

					glm::vec3 translation = tc.Translation + glm::vec3(bc2d.Offset, 0.001f);
					glm::vec3 scale = tc.Scale * glm::vec3(bc2d.Size * 2.0f, 1.0f);

					glm::mat4 transform = glm::translate(glm::mat4(1.0f), translation)
						* glm::rotate(glm::mat4(1.0f), tc.Rotation.z, glm::vec3(0.0f, 0.0f, 1.0f))
						* glm::scale(glm::mat4(1.0f), scale);

					Renderer2D::DrawRect(transform, glm::vec4(0, 1, 0, 1));
				}
			}

			// Circle Colliders
			{
				auto view = m_ActiveScene->GetAllEntitiesWith<TransformComponent, CircleCollider2DComponent>();
				for (auto entity : view)
				{
					auto [tc, cc2d] = view.get<TransformComponent, CircleCollider2DComponent>(entity);

					glm::vec3 translation = tc.Translation + glm::vec3(cc2d.Offset, 0.001f);
					glm::vec3 scale = tc.Scale * glm::vec3(cc2d.Radius * 2.0f);

					glm::mat4 transform = glm::translate(glm::mat4(1.0f), translation)
						* glm::scale(glm::mat4(1.0f), scale);

					Renderer2D::DrawCircle(transform, glm::vec4(0, 1, 0, 1), 0.01f);
				}
			}
		}

		// Draw selected entity outline 
		if (Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity())
		{
			const TransformComponent& transform = selectedEntity.GetComponent<TransformComponent>();
			Renderer2D::DrawRect(transform.GetTransform(), glm::vec4(1.0f, 0.5f, 0.0f, 1.0f));
		}

		glm::vec2 cellSize = m_TileInspectorPanel.GetCellSize();
		int layer = m_TileInspectorPanel.GetSelectedLayer();
		m_LinkedGrids = m_TileInspectorPanel.IsGridLinked();
		// Move existing objects to new grid size
		if (m_LinkedGrids)
		{
			if (m_LinkedGrids != m_PrevLinkedGrids)
			{
				std::vector<glm::vec2> cells = m_TileInspectorPanel.GetLayerCells();
				for (int i = 0; i < m_Objects.size(); i++)
				{
					if (m_Objects[i].size() > 0)
					{
						for (int j = 0; j < m_Objects[i].size(); j++)
						{
							float cellNumX = m_Objects[i][j].GetComponent<TransformComponent>().Translation.x / cells[i].x;
							float cellNumY = m_Objects[i][j].GetComponent<TransformComponent>().Translation.y / cells[i].y;

							float newPosX = cellSize.x * cellNumX;
							float newPosY = cellSize.y * cellNumY;

							m_Objects[i][j].GetComponent<TransformComponent>().Translation.x = newPosX;
							m_Objects[i][j].GetComponent<TransformComponent>().Translation.y = newPosY;

							m_ObjectCoords[i][j] = glm::vec2(newPosX, newPosY);
						}
					}
				}
				m_TileInspectorPanel.SyncLayers();
			}

			if (m_PrevCellSize != cellSize)
			{
				for (int i = 0; i < m_Objects.size(); i++)
				{
					if (m_Objects[i].size() > 0)
					{
						for (int j = 0; j < m_Objects[i].size(); j++)
						{
							float cellNumX = m_Objects[i][j].GetComponent<TransformComponent>().Translation.x / m_PrevCellSize.x;
							float cellNumY = m_Objects[i][j].GetComponent<TransformComponent>().Translation.y / m_PrevCellSize.y;

							float newPosX = cellSize.x * cellNumX;
							float newPosY = cellSize.y * cellNumY;

							m_Objects[i][j].GetComponent<TransformComponent>().Translation.x = newPosX;
							m_Objects[i][j].GetComponent<TransformComponent>().Translation.y = newPosY;

							m_ObjectCoords[i][j] = glm::vec2(newPosX, newPosY);
						}
					}
				}
				m_TileInspectorPanel.SyncLayers();
			}
		}
		else
		{
			int selectedLayer = m_TileInspectorPanel.GetSelectedLayer();
			if (selectedLayer == m_PrevLayer)
			{
				if (m_Objects[selectedLayer].size() > 0)
				{
					for (int i = 0; i < m_Objects[selectedLayer].size(); i++)
					{
						float cellNumX = m_Objects[selectedLayer][i].GetComponent<TransformComponent>().Translation.x / m_PrevCellSize.x;
						float cellNumY = m_Objects[selectedLayer][i].GetComponent<TransformComponent>().Translation.y / m_PrevCellSize.y;

						float newPosX = cellSize.x * cellNumX;
						float newPosY = cellSize.y * cellNumY;

						m_Objects[selectedLayer][i].GetComponent<TransformComponent>().Translation.x = newPosX;
						m_Objects[selectedLayer][i].GetComponent<TransformComponent>().Translation.y = newPosY;

						m_ObjectCoords[selectedLayer][i] = glm::vec2(newPosX, newPosY);
					}
				}
			}
		}

		m_SnapX = cellSize.x;
		m_SnapY = cellSize.y;

		// Draw Grid
		if (m_ShowGrid && m_Editor2D)
		{
			/// Start of Grid Code /////////////////////////////////////////////////////////////
			//m_TileInspectorPanel.SetCellSize(m_SnapX, m_SnapY);

			glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];

			glm::mat4 proj = glm::inverse(m_EditorCamera.GetProjection());
			glm::mat4 view = glm::inverse(m_EditorCamera.GetViewMatrix());
			glm::mat4 viewProj = view * proj;

			float currentX = - 1.0f;
			glm::vec4 tmp = glm::vec4(currentX, 0, 0, 1);
			tmp = tmp * viewProj;
			currentX = tmp.x;

			float currentY = - 1.0f;
			tmp = glm::vec4(0, currentY, 0, 1);
			tmp = tmp * viewProj;
			currentY = tmp.y;

			glm::vec3 worldMouse = glm::vec3(currentX + m_EditorCamera.GetPosition().x, currentY + m_EditorCamera.GetPosition().y, m_EditorCamera.GetPosition().z);
			glm::vec3 mouseDir = glm::normalize(glm::vec3(currentX * (m_EditorCamera.GetPosition().z + 1), currentY * (m_EditorCamera.GetPosition().z + 1), m_EditorCamera.GetPosition().z));
			glm::vec3 rayDelta = mouseDir * 100.0f;
			glm::vec3 planeDelta = glm::vec3(0.0f, 0.0f, 0.0f) - worldMouse;
			glm::vec3 planeNorm = glm::vec3(0.0f, 0.0f, 1.0f);

			float wp = glm::dot(planeDelta, planeNorm);
			float vp = glm::dot(rayDelta, planeNorm);
			float k = wp / vp;

			glm::vec3 planePos = worldMouse + (rayDelta * k);

			glm::vec3 gridStart = glm::vec3(
				-planePos.x + m_EditorCamera.GetPosition().x * 2.0f,
				-planePos.y + m_EditorCamera.GetPosition().y * 2.0f,
				0.0f);

			float finalX = 1.0f;
			tmp = glm::vec4(finalX, 0, 0, 1);
			tmp = tmp * viewProj;
			finalX = tmp.x;

			float finalY = 1.0f;
			tmp = glm::vec4(0, finalY, 0, 1);
			tmp = tmp * viewProj;
			finalY = tmp.y;

			worldMouse = glm::vec3(finalX + m_EditorCamera.GetPosition().x, finalY + m_EditorCamera.GetPosition().y, m_EditorCamera.GetPosition().z);
			mouseDir = glm::normalize(glm::vec3(finalX * (m_EditorCamera.GetPosition().z + 1), finalY * (m_EditorCamera.GetPosition().z + 1), m_EditorCamera.GetPosition().z));
			rayDelta = mouseDir * 100.0f;
			planeDelta = glm::vec3(0.0f, 0.0f, 0.0f) - worldMouse;
			planeNorm = glm::vec3(0.0f, 0.0f, 1.0f);

			wp = glm::dot(planeDelta, planeNorm);
			vp = glm::dot(rayDelta, planeNorm);
			k = wp / vp;

			planePos = worldMouse + (rayDelta * k);

			glm::vec3 gridFinal = glm::vec3(
				-planePos.x + m_EditorCamera.GetPosition().x * 2.0f,
				-planePos.y + m_EditorCamera.GetPosition().y * 2.0f,
				0.0f);

			float snapValue = m_SnapY;

			float snapX = m_SnapX;
			float snapY = snapValue / 2.0f;
			float snapPosX = round(gridStart.x / (snapX)) * (snapX) - (snapX * 0.5);
			float snapPosY = round(gridStart.y / (snapY)) * (snapY) - (snapValue * 0.5);

			float yMod;
			if (snapPosY >= 0.1f)
				yMod = fmodf(snapPosY + 0.0001f, snapValue);
			else
				yMod = fmodf(snapPosY - 0.0001f, snapValue);

			if (yMod >= -0.1f && yMod <= 0.1f)
			{
				if (gridStart.x >= snapPosX)
					snapPosX += snapX / 2;
				else
					snapPosX -= snapX / 2;
			}

			int xLength = gridFinal.x;
			int yLength = gridFinal.y;


			for (int i = 0; (snapPosX+(snapX *i)) <= (xLength + snapX*2); i++)
			{
				for (int j = 0; (snapPosY + (snapValue * j)) <= (yLength + snapValue*2); j++)
				{
					Renderer2D::DrawQuad(
						glm::vec3(snapPosX + (snapX * i),
							snapPosY + (snapValue * j),
							0.0f),
						glm::vec2(snapX, snapValue),
						m_DiamondWhite);
				}
			}

			/// End Grid Code //////////////////////////////////////
		}

		m_PrevCellSize = cellSize;
		m_PrevLayer = m_TileInspectorPanel.GetSelectedLayer();
		m_PrevLinkedGrids = m_TileInspectorPanel.IsGridLinked();

		if(m_ViewportHovered && m_Editor2D && !m_EditorCamera.IsDragging() && !ImGui::GetDragDropPayload())
			ObjectPreview();


		Renderer2D::EndScene();
	}

	void EditorLayer::NewScene()
	{
		
		m_ActiveScene = CreateRef<Scene>();
		m_Memento = Memento(m_ActiveScene);
		//m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
		
		m_EditorScenePath = std::filesystem::path();
		for (int i = 0; i < m_Objects.size(); i++)
		{
			m_ObjectCoords[i].clear();
			m_Objects[i].clear();
		}
		m_History.clear();
		m_ObjectCoords.resize(1);
		m_Objects.resize(1);
		m_TileInspectorPanel.ClearLayers();
		m_TileInspectorPanel.SetCellSize(2.56, 1.28);
		m_PrevCellSize = m_TileInspectorPanel.GetCellSize();
		m_TileInspectorPanel.SetLinked(true);
		//AssetManager::ResetTextureCount();
	}

	void EditorLayer::OpenScene()
	{
		std::string filepath = FileDialogs::OpenFile("Sloth Scene (*.sloth)\0*.sloth\0");
		if (!filepath.empty())
			OpenScene(filepath);
	}

	void EditorLayer::OpenScene(const std::filesystem::path& path)
	{
		if (m_SceneState != SceneState::Edit)
			OnSceneStop();

		if (path.extension().string() != ".sloth")
		{
			SLTH_WARN("Could not load {0} - not a scene file", path.filename().string());
			return;
		}
		
		Ref<Scene> newScene = CreateRef<Scene>();
		//SceneSerializer serializer(newScene, m_Objects, m_ObjectCoords, m_TileInspectorPanel.GetLayerNames(),
		//	m_TileInspectorPanel.GetBuffers());
		NewScene();
		SceneSerializer serializer(
			newScene, 
			m_TileInspectorPanel.GetLayerNames(), 
			m_TileInspectorPanel.GetBuffers(),
			m_Objects,
			m_ObjectCoords,
			m_TileInspectorPanel.GetLayerCells());
		if (serializer.Deserialize(path.string()))
		{
			m_EditorScene = newScene;
			m_EditorScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_SceneHierarchyPanel.SetContext(m_EditorScene);

			m_ActiveScene = m_EditorScene;
			m_Memento = Memento(m_ActiveScene);
			m_EditorScenePath = path;
			m_TileInspectorPanel.SetLayerNames(serializer.GetLayers());
			m_TileInspectorPanel.SetBuffers(serializer.GetBuffers());
			m_Objects = serializer.GetObjects();
			m_ObjectCoords = serializer.GetObjectCoords();
			m_TileInspectorPanel.SetLayerCells(serializer.GetCellSizes());
			m_TileInspectorPanel.SetLinked(serializer.GetGridLocked());
			m_TileInspectorPanel.SetVisibleLayers(serializer.GetVisibleLayers());
			m_ActiveScene->SetVisibleLayers(m_TileInspectorPanel.GetVisibleLayers());
			m_PrevCellSize = m_TileInspectorPanel.GetCellSize();
		}
	}

	void EditorLayer::SaveScene()
	{
		if (!m_EditorScenePath.empty())
			SerializeScene(m_ActiveScene, m_EditorScenePath);
		else
			SaveSceneAs();
	}

	void EditorLayer::SaveSceneAs()
	{
		std::string filepath = FileDialogs::SaveFile("Sloth Scene (*.sloth)\0*.sloth\0");
		if (!filepath.empty())
		{
			SerializeScene(m_ActiveScene, filepath);
			m_EditorScenePath = filepath;
		}
	}

	void EditorLayer::SerializeScene(Ref<Scene> scene, const std::filesystem::path& path)
	{
		//SceneSerializer serializer(scene, m_Objects, m_ObjectCoords, m_TileInspectorPanel.GetLayerNames(), m_TileInspectorPanel.GetBuffers());
		SceneSerializer serializer(
			scene, 
			m_TileInspectorPanel.GetLayerNames(), 
			m_TileInspectorPanel.GetBuffers(),
			m_Objects,
			m_ObjectCoords,
			m_TileInspectorPanel.GetLayerCells());
		serializer.SetGridLocked(m_LinkedGrids);
		serializer.Serialize(path.string());
	}

	void EditorLayer::OnScenePlay()
	{
		if (m_SceneState == SceneState::Simulate)
			OnSceneStop();

		m_SceneState = SceneState::Play;

		m_ActiveScene = Scene::Copy(m_EditorScene);
		m_Memento = Memento(m_ActiveScene);
		m_ActiveScene->OnRuntimeStart();

		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}

	void EditorLayer::OnSceneSimulate()
	{
		if (m_SceneState == SceneState::Play)
			OnSceneStop();

		m_SceneState = SceneState::Simulate;

		m_ActiveScene = Scene::Copy(m_EditorScene);
		m_Memento = Memento(m_ActiveScene);
		m_ActiveScene->OnSimulationStart();

		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}

	void EditorLayer::OnSceneStop()
	{
		SLTH_CORE_ASSERT(m_SceneState == SceneState::Play || m_SceneState == SceneState::Simulate);

		if (m_SceneState == SceneState::Play)
			m_ActiveScene->OnRuntimeStop();
		else if (m_SceneState == SceneState::Simulate)
			m_ActiveScene->OnSimulationStop();

		m_SceneState = SceneState::Edit;

		m_ActiveScene = m_EditorScene;
		m_Memento = Memento(m_ActiveScene);

		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}

	void EditorLayer::OnDuplicateEntity()
	{
		if (m_SceneState != SceneState::Edit)
			return;

		Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
		if (selectedEntity)
			m_EditorScene->DuplicateEntity(selectedEntity);
	}

	Entity EditorLayer::PlaceSpriteObject(const std::filesystem::path& path)
	{
		std::string filename = path.filename().string();
		filename.erase(filename.length() - 4);
		auto gameObject = m_ActiveScene->CreateEntity(filename);

		glm::vec3 objectPos = MouseToZPlane();

		gameObject.GetComponent<TransformComponent>().Translation = glm::vec4{ objectPos.x, objectPos.y, 0.0f, 1.0f };
		gameObject.AddComponent<SpriteRendererComponent>(glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f });
		gameObject.GetComponent<SpriteRendererComponent>().Texture = AssetManager::LoadTexture(path.string());

		//Ref<Texture2D> tex = gameObject.GetComponent<SpriteRendererComponent>().Texture;
		//float ratio = tex.

		return gameObject;
	}

	Entity EditorLayer::PlaceSpriteObject(Ref<Texture2D> image)
	{
		std::string filename;
		if (m_CurrentTile != NULL)
		{
			std::string s = image.get()->GetPath();
			std::wstring ws = std::wstring(s.begin(), s.end());
			const wchar_t* path = ws.c_str();
			const std::filesystem::path p = std::filesystem::path(g_AssetPath) / path;
			filename = p.filename().string();
			filename.erase(filename.length() - 4);
		}

		Entity gameObject;
		// gameObject = m_ActiveScene->CreateEntity("Tile");

		Ref<Texture2D> sprite = image;
		//auto gameObject = m_ActiveScene->CreateEntity("Tile");

		glm::vec3 objectPos = MouseToZPlane();

		float snapValue = m_SnapY;

		float snapX = m_SnapX;
		float snapY = snapValue / 2.0f;
		float snapPosX = m_SnapPos.x;
		float snapPosY = m_SnapPos.y;

		bool found = false;
		int indexFound = -1;

		int layer = m_TileInspectorPanel.GetSelectedLayer();

		if (m_ObjectCoords[layer].size() > 0) { // Already objects on the map
			for (int i = 0; i < m_ObjectCoords[layer].size(); i++)
			{
				if (m_ObjectCoords[layer][i].x <= snapPosX + (snapX/2) && m_ObjectCoords[layer][i].x >= snapPosX - (snapX/2))
					if (m_ObjectCoords[layer][i].y <= snapPosY + (snapY/2) && m_ObjectCoords[layer][i].y >= snapPosY - (snapY/2))
					{
						found = true;
						indexFound = i;
						
					}
			}

			if (!found) // a tile is not already in the current cell
			{
				// BRUSH = 0, ERASE = 1, VOLUME = 2, SELECT = 3
				state cellState;
				cellState.isEmpty = true;
				cellState.layer = layer;
				float xCellIndex = snapPosX / m_TileInspectorPanel.GetCellSize().x;
				float yCellIndex = snapPosY / m_TileInspectorPanel.GetCellSize().y;

				float xCellRound, yCellRound;

				float value = (int)(xCellIndex * 100 + .5);
				xCellRound = (float)value / 100;


				value = (int)(yCellIndex * 100 + .5);
				yCellRound = (float)value / 100;

				int xCellInt = xCellRound * 2;
				int yCellInt = yCellRound * 2;

				if (xCellInt == -0)
					xCellInt = 0;
				if (yCellInt == -0)
					yCellInt = 0;

				//Catch edge case that causes rounding negative numbers to be off by 1
				if (snapPosX < -0.001)
					xCellInt--;
				if (snapPosY < -0.001)
					yCellInt--;
				

				glm::ivec2 cellIndex = glm::ivec2(xCellInt, yCellInt);

				std::string cellString = to_string(cellIndex.x) + "," + to_string(cellIndex.y);

				int mode = m_TileInspectorPanel.GetDrawMode();

				if (mode == 0)
				{
					if (std::find(m_Cells.begin(), m_Cells.end(), cellString) != m_Cells.end())
					{

					}
					else
					{
						m_Cells.push_back(cellString);
						m_CellStates.push_back(cellState);
					}
				}

				if (mode == 0 && m_CurrentTile != NULL && !ImGui::GetDragDropPayload())
				{
					m_SceneHierarchyPanel.DeselectEntity();
					gameObject = m_ActiveScene->CreateEntity(filename);
					glm::vec2 coords = { snapPosX, snapPosY };
					
					AssetManager::LoadTexture(sprite.get()->GetPath());

					gameObject.GetComponent<TransformComponent>().Translation = glm::vec4{ snapPosX, snapPosY, 0.0f, 1.0f };
					gameObject.AddComponent<SpriteRendererComponent>(glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f });
					gameObject.GetComponent<SpriteRendererComponent>().Texture = sprite;
					gameObject.GetComponent<SpriteRendererComponent>().Layer = layer;
					gameObject.GetComponent<TransformComponent>().Snap = true;
					gameObject.SetLayer(layer);

					SceneSerializer ss(gameObject.GetComponent<SpriteRendererComponent>());
					ss.DeserializeImageData();
					gameObject.GetComponent<SpriteRendererComponent>().CenterPoint = ss.GetSpriteRenderer().CenterPoint;
					//gameObject.AddComponent<LayerComponent>(layer);

					m_ObjectCoords[layer].push_back(coords);
					m_Objects[layer].push_back(gameObject);
					
				}
				else if (mode == 0 && m_CurrentAnimations.size() > 0 && !ImGui::GetDragDropPayload())
				{
					m_SceneHierarchyPanel.DeselectEntity();
					gameObject = m_ActiveScene->CreateEntity(filename);
					glm::vec2 coords = { snapPosX, snapPosY };

					//AssetManager::LoadTexture(sprite.get()->GetPath());

					gameObject.GetComponent<TransformComponent>().Translation = glm::vec4{ snapPosX, snapPosY, 0.0f, 1.0f };
					gameObject.AddComponent<SpriteRendererComponent>(glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f });
					//gameObject.GetComponent<SpriteRendererComponent>().Texture = sprite;
					gameObject.GetComponent<SpriteRendererComponent>().Layer = layer;
					gameObject.GetComponent<TransformComponent>().Snap = true;
					gameObject.SetLayer(layer);


					gameObject.AddComponent<AnimatorComponent>();
					gameObject.GetComponent<AnimatorComponent>().animator.AddAnimation(m_CurrentAnimations[0]);
					gameObject.GetComponent<SpriteRendererComponent>().Texture = m_CurrentAnimations[0].GetFrame(m_CurrentAnimations[0].GetCurrentFrame());
					gameObject.GetComponent<SpriteRendererComponent>().CenterPoint = m_CurrentAnimations[0].GetCenterPoint(0);

					m_ObjectCoords[layer].push_back(coords);
					m_Objects[layer].push_back(gameObject);
				}
				else if (mode == 4 && !ImGui::GetDragDropPayload())
				{
					m_SceneHierarchyPanel.DeselectEntity();
				}
				else
				{
					//m_ActiveScene->DestroyEntity(gameObject);
				}
			}
			else // replacing a tile with a new painted tile
			{
				// BRUSH = 0, ERASE = 1, VOLUME = 2, EYE DROPPER = 3, SELECT = 4
				state cellState;
				cellState.name = m_Objects[layer][indexFound].GetName();
				cellState.transformC = m_Objects[layer][indexFound].GetComponent<TransformComponent>();
				int mode = m_TileInspectorPanel.GetDrawMode();
				if (mode == 0 && m_CurrentTile != NULL && !ImGui::GetDragDropPayload())
				{
					m_SceneHierarchyPanel.DeselectEntity();

					if (m_Objects[layer][indexFound].HasComponent<AnimatorComponent>())
					{
						cellState.hasAC = true;
						cellState.animatorC = m_Objects[layer][indexFound].GetComponent<AnimatorComponent>();
						m_Objects[layer][indexFound].RemoveComponent<AnimatorComponent>();
					}

					if (m_Objects[layer][indexFound].HasComponent<SpriteRendererComponent>()) 
					{
						// Save some attributes of old object for undoing function (ctr+z)
						cellState.hasSRC = true;
						cellState.spriteRendererC = m_Objects[layer][indexFound].GetComponent<SpriteRendererComponent>();


						Ref<Texture2D> oldTexture = m_Objects[layer][indexFound].GetComponent<SpriteRendererComponent>().Texture;
						//glm::vec2 oldCenter = m_Objects[layer][indexFound].GetComponent<SpriteRendererComponent>().CenterPoint;
						m_Objects[layer][indexFound].GetComponent<SpriteRendererComponent>().Texture = sprite;
						SceneSerializer ss(m_CurrentTile);
						ss.DeserializeImageData();
						m_Objects[layer][indexFound].GetComponent<SpriteRendererComponent>().CenterPoint = ss.GetSpriteRenderer().CenterPoint;
						UUID id = m_Objects[layer][indexFound].GetComponent<IDComponent>().ID;

						std::string oldPath = oldTexture.get()->GetPath();
						AssetManager::DeleteTexture(oldPath);
						std::string newPath = sprite.get()->GetPath();
						AssetManager::LoadTexture(newPath);

						if (oldPath.compare(newPath) != 0)
						{
							cellState.isEmpty = false;
							cellState.layer = layer;

							float xCellIndex = snapPosX / m_TileInspectorPanel.GetCellSize().x;
							float yCellIndex = snapPosY / m_TileInspectorPanel.GetCellSize().y;
							
							float value = (int)(xCellIndex * 100 + .5);
							float xCellRound = (float)value / 100;

							value = (int)(yCellIndex * 100 + .5);
							float yCellRound = (float)value / 100;

							int xCellInt = xCellRound * 2;
							int yCellInt = yCellRound * 2;

							if (xCellInt == -0)
								xCellInt = 0;
							if (yCellInt == -0)
								yCellInt = 0;

							//Catch edge case that causes rounding negative numbers to be off by 1
							if (snapPosX < -0.001)
								xCellInt--;
							if (snapPosY < -0.001)
								yCellInt--;

							glm::ivec2 cellIndex = glm::ivec2(xCellInt, yCellInt);

							std::string cellString = to_string(cellIndex.x) + "," + to_string(cellIndex.y);
							m_Cells.push_back(cellString);
							m_CellStates.push_back(cellState);

						}
					}
				}
				else if (mode == 0 && m_CurrentAnimations.size() >= 1 && !ImGui::GetDragDropPayload()) //painting animations
				{
					if (m_Objects[layer][indexFound].HasComponent<SpriteRendererComponent>())
					{
						cellState.hasSRC = true;
						cellState.spriteRendererC = m_Objects[layer][indexFound].GetComponent<SpriteRendererComponent>();
						if (!m_Objects[layer][indexFound].HasComponent<AnimatorComponent>())
							AssetManager::DeleteTexture(m_Objects[layer][indexFound].GetComponent<SpriteRendererComponent>().Texture.get()->GetPath());
						m_Objects[layer][indexFound].GetComponent<SpriteRendererComponent>().Texture = m_CurrentAnimations[0].GetFrame(0);
					}
					else
					{
						m_Objects[layer][indexFound].AddComponent<SpriteRendererComponent>(glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f });
						m_Objects[layer][indexFound].GetComponent<SpriteRendererComponent>().Texture = m_CurrentAnimations[0].GetFrame(0);
					}

					
					if (m_Objects[layer][indexFound].HasComponent<AnimatorComponent>())
					{
						cellState.hasAC = true;
						cellState.animatorC = m_Objects[layer][indexFound].GetComponent<AnimatorComponent>();
						m_Objects[layer][indexFound].GetComponent<AnimatorComponent>().animator.ClearAnimations();
						for (int i = 0; i < m_CurrentAnimations.size(); i++)
						{
							m_Objects[layer][indexFound].GetComponent<AnimatorComponent>().animator.AddAnimation(m_CurrentAnimations[i]);
						}
					}
					else
					{
						m_Objects[layer][indexFound].AddComponent<AnimatorComponent>();
						for (int i = 0; i < m_CurrentAnimations.size(); i++)
						{
							m_Objects[layer][indexFound].GetComponent<AnimatorComponent>().animator.AddAnimation(m_CurrentAnimations[i]);
						}
					}
					
					float xCellIndex = snapPosX / m_TileInspectorPanel.GetCellSize().x;
					float yCellIndex = snapPosY / m_TileInspectorPanel.GetCellSize().y;

					float value = (int)(xCellIndex * 100 + .5);
					float xCellRound = (float)value / 100;

					value = (int)(yCellIndex * 100 + .5);
					float yCellRound = (float)value / 100;

					int xCellInt = xCellRound * 2;
					int yCellInt = yCellRound * 2;

					if (xCellInt == -0)
						xCellInt = 0;
					if (yCellInt == -0)
						yCellInt = 0;

					//Catch edge case that causes rounding negative numbers to be off by 1
					if (snapPosX < -0.001)
						xCellInt--;
					if (snapPosY < -0.001)
						yCellInt--;

					glm::ivec2 cellIndex = glm::ivec2(xCellInt, yCellInt);

					std::string cellString = to_string(cellIndex.x) + "," + to_string(cellIndex.y);

				
					cellState.isEmpty = false;
					cellState.layer = layer;
					
					if (std::find(m_Cells.begin(), m_Cells.end(), cellString) != m_Cells.end())
					{
						
					}
					else
					{
						m_Cells.push_back(cellString);
						m_CellStates.push_back(cellState);
					}
					
				}
				else if (mode == 1 && !ImGui::GetDragDropPayload()) // Erasing tile
				{
					m_SceneHierarchyPanel.DeselectEntity(); //deselect object before erasing to prevent crash
					
					if (m_Objects[layer][indexFound].HasComponent<SpriteRendererComponent>())
					{
						cellState.hasSRC = true;
						cellState.spriteRendererC = m_Objects[layer][indexFound].GetComponent<SpriteRendererComponent>();
						if (!m_Objects[layer][indexFound].HasComponent<AnimatorComponent>())
							AssetManager::DeleteTexture(m_Objects[layer][indexFound].GetComponent<SpriteRendererComponent>().Texture.get()->GetPath());
					
					}

					if (m_Objects[layer][indexFound].HasComponent<AnimatorComponent>())
					{
						cellState.hasAC = true;
						cellState.animatorC = m_Objects[layer][indexFound].GetComponent<AnimatorComponent>();
					}
					
					std::string name = m_Objects[layer][indexFound].GetName();
					cellState.name = name;
					glm::vec2 position = m_Objects[layer][indexFound].GetComponent<TransformComponent>().Translation;
					cellState.transformC = m_Objects[layer][indexFound].GetComponent<TransformComponent>();
					UUID id = m_Objects[layer][indexFound].GetComponent<IDComponent>().ID;
					cellState.isEmpty = false;
					cellState.layer = layer;
				
					float xCellIndex = snapPosX / m_TileInspectorPanel.GetCellSize().x;
					float yCellIndex = snapPosY / m_TileInspectorPanel.GetCellSize().y;

					float value = (int)(xCellIndex * 100 + .5);
					float xCellRound = (float)value / 100;

					value = (int)(yCellIndex * 100 + .5);
					float yCellRound = (float)value / 100;

					int xCellInt = xCellRound * 2;
					int yCellInt = yCellRound * 2;

					if (xCellInt == -0)
						xCellInt = 0;
					if (yCellInt == -0)
						yCellInt = 0;

					//Catch edge case that causes rounding negative numbers to be off by 1
					if (snapPosX < -0.001)
						xCellInt--;
					if (snapPosY < -0.001)
						yCellInt--;

					glm::ivec2 cellIndex = glm::ivec2(xCellInt, yCellInt);

					std::string cellString = to_string(cellIndex.x) + "," + to_string(cellIndex.y);

					m_Cells.push_back(cellString);
					m_CellStates.push_back(cellState);
					m_ActiveScene->DestroyEntity(m_Objects[layer][indexFound]);
					m_Objects[layer].erase(m_Objects[layer].begin() + indexFound);
					m_ObjectCoords[layer].erase(m_ObjectCoords[layer].begin() + indexFound);

				}
				else if (mode == 3 && !ImGui::GetDragDropPayload())
				{
					m_CurrentTiles.clear();
					m_CurrentTile = m_Objects[layer][indexFound].GetComponent<SpriteRendererComponent>().Texture;
					m_CurrentTiles.push_back(m_CurrentTile);
					SceneSerializer ss = SceneSerializer(m_CurrentTile);
					ss.DeserializeImageData();
					glm::vec2 cp = ss.GetSpriteRenderer().CenterPoint;
					m_TileInspectorPanel.SetCenterPoint(cp);
					m_TileInspectorPanel.SetDrawMode(0);
				}
				else if (mode == 4 && !ImGui::GetDragDropPayload())
				{
					m_SceneHierarchyPanel.SetSelectedEntity(m_Objects[layer][indexFound]);
				}
				else if (mode == 5)
				{
					
				}

			}
		}
		else // First object being painted to the map
		{
			// BRUSH = 0, ERASE = 1, VOLUME = 2, SELECT = 3
			int mode = m_TileInspectorPanel.GetDrawMode();
			if (mode == 0)
			{
				state cellState;
				cellState.isEmpty = true;
				cellState.layer = layer;
				float xCellIndex = snapPosX / m_TileInspectorPanel.GetCellSize().x;
				float yCellIndex = snapPosY / m_TileInspectorPanel.GetCellSize().y;

				float value = (int)(xCellIndex * 100 + .5);
				float xCellRound = (float)value / 100;

				value = (int)(yCellIndex * 100 + .5);
				float yCellRound = (float)value / 100;

				int xCellInt = xCellRound * 2;
				int yCellInt = yCellRound * 2;

				if (xCellInt == -0)
					xCellInt = 0;
				if (yCellInt == -0)
					yCellInt = 0;

				//Catch edge case that causes rounding negative numbers to be off by 1
				if (snapPosX < -0.001)
					xCellInt--;
				if (snapPosY < -0.001)
					yCellInt--;

				glm::ivec2 cellIndex = glm::ivec2(xCellInt, yCellInt);

				std::string cellString = to_string(cellIndex.x) + "," + to_string(cellIndex.y);
				m_Cells.push_back(cellString);
				m_CellStates.push_back(cellState);
			}

			if (mode == 0 && m_CurrentTile != NULL && !ImGui::GetDragDropPayload())
			{
				m_SceneHierarchyPanel.DeselectEntity();
				gameObject = m_ActiveScene->CreateEntity(filename);
				glm::vec3 coords = { snapPosX, snapPosY, gameObject.GetUUID() };
				
				gameObject.GetComponent<TransformComponent>().Translation = glm::vec4{ snapPosX, snapPosY, 0.0f, 1.0f };
				gameObject.AddComponent<SpriteRendererComponent>(glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f });
				gameObject.GetComponent<SpriteRendererComponent>().Texture = sprite;
				gameObject.GetComponent<SpriteRendererComponent>().Layer = layer;
				gameObject.GetComponent<TransformComponent>().Snap = true;
				UUID id = gameObject.GetComponent<IDComponent>().ID;

				AssetManager::LoadTexture(sprite.get()->GetPath());

				SceneSerializer ss(gameObject.GetComponent<SpriteRendererComponent>());
				ss.DeserializeImageData();
				gameObject.GetComponent<SpriteRendererComponent>().CenterPoint = ss.GetSpriteRenderer().CenterPoint;
				//gameObject.AddComponent<LayerComponent>(layer);

				m_ObjectCoords[layer].push_back(coords);
				m_Objects[layer].push_back(gameObject);

				sort(m_Objects[layer].begin(), m_Objects[layer].end(), compareObjects);

			}
			else if (mode == 0 && m_CurrentAnimations.size() > 0 && !ImGui::GetDragDropPayload())
			{
				m_SceneHierarchyPanel.DeselectEntity();

				gameObject = m_ActiveScene->CreateEntity(filename);
				glm::vec2 coords = { snapPosX, snapPosY };

				//AssetManager::LoadTexture(sprite.get()->GetPath());

				gameObject.GetComponent<TransformComponent>().Translation = glm::vec4{ snapPosX, snapPosY, 0.0f, 1.0f };
				gameObject.AddComponent<SpriteRendererComponent>(glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f });
				//gameObject.GetComponent<SpriteRendererComponent>().Texture = sprite;
				gameObject.GetComponent<SpriteRendererComponent>().Layer = layer;
				gameObject.GetComponent<TransformComponent>().Snap = true;
				gameObject.SetLayer(layer);

				//SceneSerializer ss(gameObject.GetComponent<SpriteRendererComponent>());
				//ss.DeserializeImageData();
				//gameObject.GetComponent<SpriteRendererComponent>().CenterPoint = ss.GetSpriteRenderer().CenterPoint;

				gameObject.AddComponent<AnimatorComponent>();
				gameObject.GetComponent<AnimatorComponent>().animator.AddAnimation(m_CurrentAnimations[0]);
				gameObject.GetComponent<SpriteRendererComponent>().Texture = m_CurrentAnimations[0].GetFrame(m_CurrentAnimations[0].GetCurrentFrame());
				gameObject.GetComponent<SpriteRendererComponent>().CenterPoint = m_CurrentAnimations[0].GetCenterPoint(0);

				m_ObjectCoords[layer].push_back(coords);
				m_Objects[layer].push_back(gameObject);
			}

			else
			{
				//m_ActiveScene->DestroyEntity(gameObject);
			}
		}
		return gameObject;
	}

	void EditorLayer::ObjectPreview()
	{
		glm::vec3 objectPos = MouseToZPlane();

		float snapValue = m_SnapY;

		float snapX = m_SnapX;
		float snapY = snapValue / 1.0f;
		float snapPosX = round(objectPos.x / (snapX)) * (snapX);
		float snapPosY = round(objectPos.y / (snapY)) * (snapY);

		//float localXZero = snapPosX - (snapX * 0.5f);
		//float localYZero = snapPosY - (snapY * 0.5f);

		// if mouse is in top left of grid diamond
		if (objectPos.x < snapPosX && objectPos.y > snapPosY)
		{
			glm::vec2 p1 = glm::vec2(snapPosX, snapPosY + (snapValue * 0.5f));
			glm::vec2 p2 = glm::vec2(snapPosX - (snapX * 0.5f), snapPosY);
			
			//Renderer2D::DrawLine(glm::vec3(p1.x, p1.y, 0.0f), glm::vec3(p2.x, p2.y, 0.0f), glm::vec4(1, 0, 0, 1));
			// slope intercept equation of the top left line of the diamond
			float slope = ((p1.y - p2.y) / (p1.x - p2.x));
			float b = snapPosY - (slope * (snapPosX - (snapX * 0.5)));
			float y = (slope * objectPos.x) + b;

			// Check if point is in diamond
			if (y > objectPos.y)
			{
				// In diamond
			}
			else
			{
				// Not in diamond
				//move up/left diagnal if we leave the diamond in that direction
				snapPosX -= snapX * 0.5f;
				snapPosY += snapValue * 0.5f;
			}
		}
		// if mouse is in top right of grid diamond
		else if (objectPos.x > snapPosX && objectPos.y > snapPosY)
		{
			glm::vec2 p1 = glm::vec2(snapPosX + (snapX * 0.5f), snapPosY);
			glm::vec2 p2 = glm::vec2(snapPosX, snapPosY + (snapValue * 0.5));

			//Renderer2D::DrawLine(glm::vec3(p1.x, p1.y, 0.0f), glm::vec3(p2.x, p2.y, 0.0f), glm::vec4(1, 0, 0, 1));
			// slope intercept equation of the top left line of the diamond
			float slope = ((p1.y - p2.y) / (p1.x - p2.x));
			float b = snapPosY - (slope * (snapPosX + (snapX * 0.5)));
			float y = (slope * objectPos.x) + b;

			// Check if point is in diamond
			if (y > objectPos.y)
			{
				// In diamond
			}
			else
			{
				// Not in diamond
				//move up/right diagnal if we leave the diamond in that direction
				snapPosX += snapX / 2;
				snapPosY += snapValue / 2;
			}
		}
		// if mouse is in bottom left of diamond
		else if (objectPos.x < snapPosX && objectPos.y < snapPosY)
		{
			glm::vec2 p1 = glm::vec2(snapPosX, snapPosY - (snapValue * 0.5f));
			glm::vec2 p2 = glm::vec2(snapPosX - (snapX * 0.5f), snapPosY);

			//Renderer2D::DrawLine(glm::vec3(p1.x, p1.y, 0.0f), glm::vec3(p2.x, p2.y, 0.0f), glm::vec4(1, 0, 0, 1));
			// slope intercept equation of the top left line of the diamond
			float slope = ((p1.y - p2.y) / (p1.x - p2.x));
			float b = snapPosY - (slope * (snapPosX - (snapX * 0.5)));
			float y = (slope * objectPos.x) + b;

			// Check if point is in diamond
			if (y < objectPos.y)
			{
				// In diamond
			}
			else
			{
				// Not in diamond
				//move down/left diagnal if we leave the diamond in that direction
				snapPosX -= snapX * 0.5f;
				snapPosY -= snapValue * 0.5f;
			}
		}
		// if mouse is in bottom right of diamond
		else if (objectPos.x > snapPosX && objectPos.y < snapPosY)
		{
			glm::vec2 p1 = glm::vec2(snapPosX + (snapX * 0.5f), snapPosY);
			glm::vec2 p2 = glm::vec2(snapPosX, snapPosY - (snapValue * 0.5));

			//Renderer2D::DrawLine(glm::vec3(p1.x, p1.y, 0.0f), glm::vec3(p2.x, p2.y, 0.0f), glm::vec4(1, 0, 0, 1));
			// slope intercept equation of the top left line of the diamond
			float slope = ((p1.y - p2.y) / (p1.x - p2.x));
			float b = snapPosY - (slope * (snapPosX + (snapX * 0.5)));
			float y = (slope * objectPos.x) + b;

			// Check if point is in diamond
			if (y < objectPos.y)
			{
				// In diamond
			}
			else
			{
				// Not in diamond
				//move down/left diagnal if we leave the diamond in that direction
				snapPosX += snapX * 0.5f;
				snapPosY -= snapValue * 0.5f;

			}
		}
		
		m_SnapPos = glm::vec2(snapPosX, snapPosY);
		
		if (m_CurrentTile != NULL && cursorID == 1)
		{
			SceneSerializer ss(m_CurrentTile);
			ss.DeserializeImageData();
			glm::vec2 cp = ss.GetSpriteRenderer().CenterPoint;
			Renderer2D::DrawQuad(glm::vec2(snapPosX -cp.x, snapPosY + cp.y), glm::vec2(1.0f, 1.0f), m_CurrentTile, true, 1.0f, glm::vec4(1, 1, 1, 0.4));
		}

		else if (m_CurrentAnimations.size() > 0 && cursorID == 1)
		{
			m_CurrentAnimations[0].GetCenterPoint(m_CurrentAnimations[0].GetCurrentFrame());
			m_CurrentAnimations[0].GetFrame(m_CurrentAnimations[0].GetCurrentFrame());
			Renderer2D::DrawQuad(glm::vec2(snapPosX - m_CurrentAnimations[0].GetCenterPoint(m_CurrentAnimations[0].GetCurrentFrame()).x, 
				snapPosY + m_CurrentAnimations[0].GetCenterPoint(m_CurrentAnimations[0].GetCurrentFrame()).y), glm::vec2(1.0f, 1.0f),
				m_CurrentAnimations[0].GetFrame(m_CurrentAnimations[0].GetCurrentFrame()), true, 1.0f, glm::vec4(1, 1, 1, 0.4));
		}
		Renderer2D::DrawQuad(glm::vec3(snapPosX, snapPosY, 0.0f), glm::vec2(snapX, snapValue), m_DiamondWhite,1.0f, glm::vec4(1,1,0,1));
		
	}

	glm::vec3 EditorLayer::MouseToZPlane()
	{
		auto [mx, my] = ImGui::GetMousePos();
		mx -= m_ViewportBounds[0].x;
		my -= m_ViewportBounds[0].y;
		glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
		my = viewportSize.y - my;
		int mouseX = (int)mx;
		int mouseY = (int)my;
		glm::vec2 mouse = glm::vec2(mouseX, mouseY);

		glm::mat4 proj = glm::inverse(m_EditorCamera.GetProjection());
		glm::mat4 view = glm::inverse(m_EditorCamera.GetViewMatrix());
		glm::mat4 viewProj = view * proj;

		float currentX = (mouse.x / viewportSize.x) * 2.0f - 1.0f;
		glm::vec4 tmp = glm::vec4(currentX, 0, 0, 1);
		tmp = tmp * viewProj;
		currentX = tmp.x;

		float currentY = (mouse.y / viewportSize.y) * 2.0f - 1.0f;
		tmp = glm::vec4(0, currentY, 0, 1);
		tmp = tmp * viewProj;
		currentY = tmp.y;

		glm::vec3 worldMouse = glm::vec3(currentX + m_EditorCamera.GetPosition().x, currentY + m_EditorCamera.GetPosition().y, m_EditorCamera.GetPosition().z);
		glm::vec3 mouseDir = glm::normalize(glm::vec3(currentX * (m_EditorCamera.GetPosition().z + 1), currentY * (m_EditorCamera.GetPosition().z + 1), m_EditorCamera.GetPosition().z));
		glm::vec3 rayDelta = mouseDir * 100.0f;
		glm::vec3 planeDelta = glm::vec3(0.0f, 0.0f, 0.0f) - worldMouse;
		glm::vec3 planeNorm = glm::vec3(0.0f, 0.0f, 1.0f);

		float wp = glm::dot(planeDelta, planeNorm);
		float vp = glm::dot(rayDelta, planeNorm);
		float k = wp / vp;

		glm::vec3 planePos = worldMouse + (rayDelta * k);

		return glm::vec3(-planePos.x + m_EditorCamera.GetPosition().x * 2.0f, -planePos.y + m_EditorCamera.GetPosition().y * 2.0f, 0.0f);

	}

	GLFWcursor* EditorLayer::CreateCursor(int tool)
	{
		auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
	
		unsigned char pixels[32 * 32 * 4];
		//memset(pixels, 0xff, sizeof(pixels));
		
		std::ifstream image;
		if (tool == 0)
			image.open("Resources/Icons/Tilemap/Brush.bmp", std::ios_base::binary);
		else if (tool == 1)
			image.open("Resources/Icons/Tilemap/Eraser.bmp", std::ios_base::binary);
		else if (tool == 2)
			image.open("Resources/Icons/Tilemap/Dropper.bmp", std::ios_base::binary);
		else if (tool == 3)
			image.open("Resources/Icons/Tilemap/Select.bmp", std::ios_base::binary);
		else if (tool == 5)
			image.open("Resources/Icons/Drag.bmp", std::ios_base::binary);

		image.seekg(0, std::ios::end);
		int n = image.tellg();
		image.seekg(0, std::ios::beg);

		char* res = new char[n];
		//for (int i = 0; i < n; i++)
		//	res[i] = '5';

		bool bit = image.eof();
		image.seekg(138);
		image.read(res, sizeof(pixels));

		int end = 4096;
		
		//memcpy(pixels, res, sizeof(pixels));

		//upside down, wrong color
		//for (int i = 0; i < 4096; i++)
		//{
		//	pixels[i] = res[i];
		//}


		// right side up wrong color
		/*for (int i = 0; i < 32; i++)
		{
			for (int j = 0; j < 128; j++)
			{
				int value = end - ((i + 1) * 128) + j;
				pixels[(i*128)+j] = res[value];
			}
		}*/

		// Flips BMP verically to correct orientation and flips BGRA format to RGBA format
		for (int i = 0; i < 32; i++)
		{
			for (int j = 0; j < 128; j+=4)
			{
				int value = end - ((i + 1) * 128) + j;
				pixels[(i * 128) + j] = res[value + 2];
				pixels[(i * 128) + j + 1] = res[value + 1];
				pixels[(i * 128) + j + 2] = res[value];
				pixels[(i * 128) + j + 3] = res[value + 3];
			}
		}
		
		GLFWimage icon;
		icon.width = 32;
		icon.height = 32;
		icon.pixels = pixels;

		GLFWcursor* cursor;
		if(tool == 3)
			cursor = glfwCreateCursor(&icon, 16, 0);
		else
			cursor = glfwCreateCursor(&icon, 0, 32);

		return cursor;
	}

	void EditorLayer::SetCursor(GLFWcursor* cursor)
	{
		auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		glfwSetCursor(window, cursor);
	}

	

}
