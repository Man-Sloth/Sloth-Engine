#include "SceneHierarchyPanel.h"
#include "Sloth/Scene/Components.h"
#include "Sloth/Scene/SceneSerializer.h"
#include "Sloth/Renderer/AssetManager.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <glm/gtc/type_ptr.hpp>


#include <cstring>
#include <iostream>

/* The Microsoft C++ compiler is non-compliant with the C++ standard and needs
 * the following definition to disable a security warning on std::strncpy().
 */
#ifdef _MSVC_LANG
  #define _CRT_SECURE_NO_WARNINGS
#endif

namespace Sloth {

	extern const std::filesystem::path g_AssetPath;

	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context)
	{
		SetContext(context);
	}

	void SceneHierarchyPanel::SetContext(const Ref<Scene>& context)
	{
		if (m_SelectionContext)
			if (m_SelectionContext.HasComponent<SpriteRendererComponent>())
				AssetManager::DeleteTexture(m_SelectionContext.GetComponent<SpriteRendererComponent>().Texture.get()->GetPath());
		m_Context = context;
		m_NextSelectedEntity = {};
		m_SelectionContext = {};
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		if (showHierarchyPanel)
		{
			ImGui::Begin("Scene Hierarchy", &showHierarchyPanel);

			if (m_Tilemap.size() > 0)
			{
				DrawTmapNodes();
			}

			if (m_Context)
			{
				m_Context->m_Registry.each([&](auto entityID)
					{
						Entity entity{ entityID , m_Context.get() };
						if (!entity.HasComponent<SpriteRendererComponent>())
							DrawEntityNode(entity);
					});

				if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
					m_SelectionContext = {};

				//	// Right-click on blank space
				if (ImGui::BeginPopupContextWindow(0, 1, false))
				{
					if (ImGui::MenuItem("Create Empty Entity"))
						m_Context->CreateEntity("Empty Entity");

					ImGui::EndPopup();
				}

				//if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && ImGui::IsWindowHovered())
				//	ImGui::SetWindowFocus();

			}
			ImGui::End();
		}

		if (showPropertiesPanel)
		{
			ImGui::Begin("Properties", &showPropertiesPanel);

			// if we're not switching object selections this frame
			if (!changedSelected)
			{
				if (m_SelectionContext)
					DrawComponents(m_SelectionContext);
			}
			// when ImGui is done from last frame we switch object selection
			else
			{
				m_SelectionContext = m_NextSelectedEntity;
				changedSelected = false;
			}

			ImGui::End();
		}
	}

	void SceneHierarchyPanel::SetSelectedEntity(Entity entity)
	{
		//Let ImGui finish it's processes before changing selection to prevent stats from transfering to new object
		if (m_NextSelectedEntity)
		{
			//if something new is clicked and not the same object
			if (entity.GetUUID() != m_NextSelectedEntity.GetUUID())
			{
				changedSelected = true;
				m_NextSelectedEntity = entity;
			}
		}
		else
		{
			//if nothing is selected yet
			changedSelected = true;
			m_NextSelectedEntity = entity;		
		}
	}

	void SceneHierarchyPanel::DeselectEntity()
	{
		m_SelectionContext = {};
		m_NextSelectedEntity = {};
	}

	void SceneHierarchyPanel::SetTilemapData(std::vector<std::string> tmap)
	{
		m_Tilemap = tmap;
	}

	void SceneHierarchyPanel::SetShowHierarchyPanel(bool show)
	{
		showHierarchyPanel = show;
	}

	void SceneHierarchyPanel::SetShowPropertiesPanel(bool show)
	{
		showPropertiesPanel = show;
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		auto& tag = entity.GetComponent<TagComponent>().Tag;
		
		ImGuiTreeNodeFlags flags = ((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());
		if (ImGui::IsItemClicked())
		{
			m_SelectionContext = entity;
		}

		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete Entity"))
				entityDeleted = true;

			ImGui::EndPopup();
		}

		if (opened)
		{
			//ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
			//bool opened = ImGui::TreeNodeEx((void*)9817239, flags, tag.c_str());
			if (opened)
				ImGui::TreePop();
			//ImGui::TreePop();
		}

		if (entityDeleted)
		{
			m_Context->DestroyEntity(entity);
			if (m_SelectionContext == entity)
				m_SelectionContext = {};
		}
	}

	void SceneHierarchyPanel::DrawTmapNodes()
	{
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
		bool opened = ImGui::TreeNodeEx("Tile Map", flags, "Tilemap");
		if (ImGui::IsItemClicked())
		{
			//m_SelectionContext = entity;
		}

		if (opened)
		{

			////bool opened = ImGui::TreeNodeEx((void*)9817239, flags, tag.c_str());
			for (int i = 0; i < m_Tilemap.size(); i++)
			{
				ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
				bool opened = ImGui::TreeNodeEx((void*)(intptr_t)i, flags, m_Tilemap[i].c_str());

				if (opened)
				{
					if (m_Context)
					{
						m_Context->m_Registry.each([&](auto entityID)
							{
								Entity entity{ entityID , m_Context.get()};
								if (entity.HasComponent<SpriteRendererComponent>())
								{
									auto& layer = entity.GetComponent<SpriteRendererComponent>().Layer;
									if (layer == i)
										DrawEntityNode(entity);
								}
							});

						if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
							m_SelectionContext = {};

						// Right-click on blank space
						//if (ImGui::BeginPopupContextWindow(0, 1, false))
						//{
						//	if (ImGui::MenuItem("Create Empty Entity"))
						//		m_Context->CreateEntity("Empty Entity");

						//	ImGui::EndPopup();
						//}

						//if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && ImGui::IsWindowHovered())
						//	ImGui::SetWindowFocus();

					}
					ImGui::TreePop();
				}
			}
			ImGui::TreePop();
		}
	}

	static void DrawVec2Control(const std::string& label, glm::vec2& values, SpriteRendererComponent& s, float resetValue = 0.0f, float columnWidth = 100.0f)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(2, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize))
		{
			values.x = resetValue;
			if (s.UseCenterPoint)
			{
				std::cout << "Saved Image META data!\n";
				SceneSerializer ss(s);
				ss.SerializeImageData();
			}
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, 0.01f, 0.0f, 0.0f, "%.2f");

		if ((ImGui::IsItemDeactivatedAfterEdit()))
		{
			if (s.UseCenterPoint)
			{
				std::cout << "Saved Image META data!\n";
				SceneSerializer ss(s);
				ss.SerializeImageData();
			}
			
		}
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize))
		{
			values.y = resetValue;
			if (s.UseCenterPoint)
			{
				std::cout << "Saved Image META data!\n";
				SceneSerializer ss(s);
				ss.SerializeImageData();
			}
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, 0.01f, 0.0f, 0.0f, "%.2f");
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			if (s.UseCenterPoint)
			{
				std::cout << "Saved Image META data!\n";
				SceneSerializer ss(s);
				ss.SerializeImageData();

			}
		}
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();
	}

	static void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize))
			values.x = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize))
			values.y = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Z", buttonSize))
			values.z = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();
	}
	
	template<typename T, typename UIFunction>
	static void DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction, bool selectChange = false)
	{

		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

		if (entity.HasComponent<T>())
		{
			auto& component = entity.GetComponent<T>();
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImGui::Separator();
			bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
			ImGui::PopStyleVar(
			);
			std::string n = name.c_str();
			if (n.compare("Transform") != 0)
			{
				ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
				if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight }))
				{
					ImGui::OpenPopup("ComponentSettings");
				}
			}

			bool removeComponent = false;
			if (ImGui::BeginPopup("ComponentSettings"))
			{
				if (ImGui::MenuItem("Remove component"))
					removeComponent = true;

				ImGui::EndPopup();
			}

			if (open)
			{
				uiFunction(component);
				ImGui::TreePop();
			}

			if (removeComponent)
				entity.RemoveComponent<T>();
		}
	}

	void SceneHierarchyPanel::DrawComponents(Entity entity)
	{
		if (entity.HasComponent<TagComponent>())
		{
			auto& tag = entity.GetComponent<TagComponent>().Tag;

			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			std::strncpy(buffer, tag.c_str(), sizeof(buffer));
			if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
			{
				tag = std::string(buffer);
			}
		}

		if (entity.HasComponent<AnimatorComponent>())
			if (entity.HasComponent<SpriteRendererComponent>())
				entity.GetComponent<AnimatorComponent>().src = &entity.GetComponent<SpriteRendererComponent>();

		ImGui::SameLine();
		ImGui::PushItemWidth(-1);

		if (ImGui::Button("Add Component"))
			ImGui::OpenPopup("AddComponent");

		if (ImGui::BeginPopup("AddComponent"))
		{
			DisplayAddComponentEntry<CameraComponent>("Camera");
			DisplayAddComponentEntry<SpriteRendererComponent>("Sprite Renderer");
			DisplayAddComponentEntry<CircleRendererComponent>("Circle Renderer");
			DisplayAddComponentEntry<Rigidbody2DComponent>("Rigidbody 2D");
			DisplayAddComponentEntry<BoxCollider2DComponent>("Box Collider 2D");
			DisplayAddComponentEntry<CircleCollider2DComponent>("Circle Collider 2D");
			DisplayAddComponentEntry<AnimatorComponent>("Animator");

			ImGui::EndPopup();
		}

		ImGui::PopItemWidth();

		DrawComponent<TransformComponent>("Transform", entity, [](auto& component)
		{
			DrawVec3Control("Translation", component.Translation);
			glm::vec3 rotation = glm::degrees(component.Rotation);
			DrawVec3Control("Rotation", rotation);
			component.Rotation = glm::radians(rotation);
			DrawVec3Control("Scale", component.Scale, 1.0f);
		});

		DrawComponent<CameraComponent>("Camera", entity, [](auto& component)
		{
			auto& camera = component.Camera;

			ImGui::Checkbox("Primary", &component.Primary);

			const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
			const char* currentProjectionTypeString = projectionTypeStrings[(int)camera.GetProjectionType()];
			if (ImGui::BeginCombo("Projection", currentProjectionTypeString))
			{
				for (int i = 0; i < 2; i++)
				{
					bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];
					if (ImGui::Selectable(projectionTypeStrings[i], isSelected))
					{
						currentProjectionTypeString = projectionTypeStrings[i];
						camera.SetProjectionType((SceneCamera::ProjectionType)i);
					}

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}

			if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
			{
				float perspectiveVerticalFov = glm::degrees(camera.GetPerspectiveVerticalFOV());
				if (ImGui::DragFloat("Vertical FOV", &perspectiveVerticalFov))
					camera.SetPerspectiveVerticalFOV(glm::radians(perspectiveVerticalFov));

				float perspectiveNear = camera.GetPerspectiveNearClip();
				if (ImGui::DragFloat("Near", &perspectiveNear))
					camera.SetPerspectiveNearClip(perspectiveNear);

				float perspectiveFar = camera.GetPerspectiveFarClip();
				if (ImGui::DragFloat("Far", &perspectiveFar))
					camera.SetPerspectiveFarClip(perspectiveFar);
			}

			if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
			{
				float orthoSize = camera.GetOrthographicSize();
				if (ImGui::DragFloat("Size", &orthoSize))
					camera.SetOrthographicSize(orthoSize);

				float orthoNear = camera.GetOrthographicNearClip();
				if (ImGui::DragFloat("Near", &orthoNear))
					camera.SetOrthographicNearClip(orthoNear);

				float orthoFar = camera.GetOrthographicFarClip();
				if (ImGui::DragFloat("Far", &orthoFar))
					camera.SetOrthographicFarClip(orthoFar);

				ImGui::Checkbox("Fixed Aspect Ratio", &component.FixedAspectRatio);
			}
		});

		DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [](auto& component)
		{
			ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));
			std::filesystem::path filePath = component.Texture.get()->GetPath();
			std::string fileName = filePath.filename().string();
			ImGui::Image((ImTextureID)component.Texture->GetRendererID(), ImVec2(25, 25), { 0, 1 }, { 1, 0 });
			ImGui::SameLine();
			ImGui::Button(fileName.c_str(), ImVec2(200.0f, 0.0f));
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const wchar_t* path = (const wchar_t*)payload->Data;
					std::filesystem::path texturePath = std::filesystem::path(g_AssetPath) / path;
					
					Ref<Texture2D> texture = AssetManager::LoadTexture(texturePath.string());
					if (texture->IsLoaded())
					{
						if (component.Texture->IsLoaded())
							AssetManager::DeleteTexture(component.Texture.get()->GetPath());
						component.Texture = texture;
					}
					else
						SLTH_WARN("Could not load texture {0}", texturePath.filename().string());
				}
				ImGui::EndDragDropTarget();
			}

			ImGui::DragFloat("Tiling Factor", &component.TilingFactor, 0.1f, 0.0f, 100.0f);

			
			DrawVec2Control("Center Point", component.CenterPoint, component);
			std::string layer = "Layer: " + std::to_string(component.Layer);
			ImGui::Text(layer.c_str());

			ImGui::Checkbox("Always use \" Center Point \" for this texture.",&component.UseCenterPoint);
			if (ImGui::IsItemDeactivatedAfterEdit())
			{
				if (component.UseCenterPoint)
				{
					std::cout << "Saved Image META data!\n";
					SceneSerializer ss(component);
					ss.SerializeImageData();
				}
				else
				{
				}
			}
		});

		DrawComponent<AnimatorComponent>("Animator", entity, [](auto& component)
		{
				
				std::string controllerName;
				if (component.animator.GetSize() == 0)
					controllerName = "No Controller Assigend";
				else
					controllerName = component.animator.GetName();

				ImGui::Columns(2);
				ImGui::SetColumnWidth(0, 140);
				ImGui::Text("Controller ");
				ImGui::NextColumn();
				ImGui::Button(controllerName.c_str(), ImVec2(200, 25));
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ANIMATORCONTROLLER_ITEM"))
					{
						const wchar_t* path = (const wchar_t*)payload->Data; 
						std::wstring ws(path);
						std::string pathString(ws.begin(), ws.end());

						int pos = 0;
						int count = 0;
						std::string target = ".anmtr";
						std::vector<int> positions;
						while ((pos = pathString.find(target, pos)) != std::string::npos) { //save how many times .anm shows up to separate paths
							count++;
							pos += target.length();
							positions.push_back(pos);
						}

						std::vector<std::string> subPaths;
						pos = 0;
						for (int i = 0; i < count; i++)
						{
							subPaths.push_back(pathString.substr(pos, positions[i] - pos)); // Save each path
							pos += subPaths[i].length();
						}
						SceneSerializer ss;
						ss.DeserializeAnimator(subPaths[0]);
						
						//delet old images
						component.animator.ClearAnimations();
						//AssetManager::DeleteTexture(component.src->Texture.get()->GetPath());
						

						component.animator = ss.GetAnimator();
						component.animator.SetPath(subPaths[0]);
						controllerName = subPaths[0];
						std::filesystem::path pathName = controllerName;
						std::string controllerName = pathName.filename().string();
						component.animator.SetName(controllerName);
						component.animator.SetTags(ss.GetTags());
						

						AssetManager::DeleteTexture(component.src->Texture.get()->GetPath());
						component.src->Texture = component.animator.GetAnimation(0).GetFrame(0);
						AssetManager::LoadTexture(component.src->Texture.get()->GetPath());
						component.src->CenterPoint = component.animator.GetAnimation(0).GetCenterPoint(0);
					}
				}

				ImGui::NextColumn();
				ImGui::Text("Current Animation ");
				ImGui::NextColumn();
				ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove;
				std::string animationName;

				if (component.animator.GetSize() > 0)
				{
					component.animator.Update();
					Animator a = component.animator;
					animationName = component.animator.GetCurrentAnimation().GetName();

					if (ImGui::Button(const_cast<char*>(animationName.c_str())))
						ImGui::OpenPopup("active_animator");

					if (ImGui::BeginPopup("active_animator", flags))
					{
						for (int i = 0; i < component.animator.GetSize(); i++)
						{
							int num = component.animator.GetSize();
							std::string id = component.animator.GetAnimation(i).GetName() + "##" + std::to_string(i);
							if (ImGui::Selectable(const_cast<char*>(id.c_str())))
								component.animator.SetAnimation(i);
						}
						ImGui::EndPopup();
					}
					AssetManager::DeleteTexture(component.src->Texture.get()->GetPath());
					component.src->Texture = component.animator.GetCurrentAnimation().GetFrame(component.animator.GetCurrentAnimation().GetCurrentFrame());
					AssetManager::LoadTexture(component.src->Texture.get()->GetPath());
					component.src->CenterPoint = component.animator.GetCurrentAnimation().GetCenterPoint(component.animator.GetCurrentAnimation().GetCurrentFrame());
				}
				else
				{
					if (ImGui::Button("NONE"))
						ImGui::OpenPopup("no_animator");

					if (ImGui::BeginPopup("no_animator", flags))
					{
						if (ImGui::Selectable("NONE"))
						{
						}
						ImGui::EndPopup();
					}
				}
		});

		DrawComponent<CircleRendererComponent>("Circle Renderer", entity, [](auto& component)
		{
			ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));
			ImGui::DragFloat("Thickness", &component.Thickness, 0.025f, 0.0f, 1.0f);
			ImGui::DragFloat("Fade", &component.Fade, 0.00025f, 0.0f, 1.0f);
		});

		DrawComponent<Rigidbody2DComponent>("Rigidbody 2D", entity, [](auto& component)
		{
			const char* bodyTypeStrings[] = { "Static", "Dynamic", "Kinematic"};
			const char* currentBodyTypeString = bodyTypeStrings[(int)component.Type];
			if (ImGui::BeginCombo("Body Type", currentBodyTypeString))
			{
				for (int i = 0; i < 2; i++)
				{
					bool isSelected = currentBodyTypeString == bodyTypeStrings[i];
					if (ImGui::Selectable(bodyTypeStrings[i], isSelected))
					{
						currentBodyTypeString = bodyTypeStrings[i];
						component.Type = (Rigidbody2DComponent::BodyType)i;
					}

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}

			ImGui::Checkbox("Fixed Rotation", &component.FixedRotation);
		});

		DrawComponent<BoxCollider2DComponent>("Box Collider 2D", entity, [](auto& component)
		{
			ImGui::DragFloat2("Offset", glm::value_ptr(component.Offset));
			ImGui::DragFloat2("Size", glm::value_ptr(component.Size));
			ImGui::DragFloat("Density", &component.Density, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Friction", &component.Friction, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Restitution", &component.Restitution, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Restitution Threshold", &component.RestitutionThreshold, 0.01f, 0.0f);
		});

		DrawComponent<CircleCollider2DComponent>("Circle Collider 2D", entity, [](auto& component)
		{
			ImGui::DragFloat2("Offset", glm::value_ptr(component.Offset));
			ImGui::DragFloat("Radius", &component.Radius);
			ImGui::DragFloat("Density", &component.Density, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Friction", &component.Friction, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Restitution", &component.Restitution, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Restitution Threshold", &component.RestitutionThreshold, 0.01f, 0.0f);
		});

	}
	
	template<typename T>
	void SceneHierarchyPanel::DisplayAddComponentEntry(const std::string& entryName) {
		if (!m_SelectionContext.HasComponent<T>())
		{
			if (ImGui::MenuItem(entryName.c_str()))
			{
				m_SelectionContext.AddComponent<T>();
				ImGui::CloseCurrentPopup();
			}
		}
	}

}
