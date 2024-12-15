#define OEMRESOURCE
//#include <windows.h>
#include <conio.h>

#include "slthpch.h"
#include "TileInspectorPanel.h"
#include "Sloth/Scene/SceneSerializer.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace Sloth {

	static bool tempEditVar = false;
	static void DrawVec2Control(const std::string& label, glm::vec2& values, float resetValue = 1.0f, float columnWidth = 100.0f, std::vector<Ref<Texture2D>>& tile = std::vector<Ref<Texture2D>>(), std::vector<glm::vec2>& centerPoints = std::vector<glm::vec2>(), bool& editing = tempEditVar);
	
	TileInspectorPanel::TileInspectorPanel()
	{
		m_Paint = Texture2D::Create("Resources/Icons/Tilemap/Brush.png");
		m_Erase = Texture2D::Create("Resources/Icons/Tilemap/Eraser.png");
		m_Volume = Texture2D::Create("Resources/Icons/Tilemap/Expand1.png");
		m_EyeDropper = Texture2D::Create("Resources/Icons/Tilemap/Dropper.png");
		m_Select = Texture2D::Create("Resources/Icons/Tilemap/Select.png");
		m_NoImage = Texture2D::Create("Resources/Icons/Tilemap/NoImage1.png");

		m_ActivePaint = Texture2D::Create("Resources/Icons/Tilemap/BrushActive.png");
		m_ActiveErase = Texture2D::Create("Resources/Icons/Tilemap/EraserActive.png");
		m_ActiveVolume = Texture2D::Create("Resources/Icons/Tilemap/ExpandActive.png");
		m_ActiveEyeDropper = Texture2D::Create("Resources/Icons/Tilemap/DropperActive.png");
		m_ActiveSelect = Texture2D::Create("Resources/Icons/Tilemap/SelectActive.png");
		m_Link = Texture2D::Create("Resources/Icons/Tilemap/link.png");
		m_LinkBroke = Texture2D::Create("Resources/Icons/Tilemap/linkBroke.png");
		m_Checkerboard = Texture2D::Create("Resources/checkerboard.png");
		m_BlueCircle = Texture2D::Create("Resources/BlueCircle.png");
		m_Visible = Texture2D::Create("Resources/Icons/Tilemap/visible.png");
		m_NotVisible = Texture2D::Create("Resources/Icons/Tilemap/notVisible.png");
		m_Trash = Texture2D::Create("Resources/Icons/Tilemap/trash.png");

		m_ArrowIcon = LoadCursor(NULL, IDC_HAND);

		m_Ico.xHotspot = 0;
		m_Ico.yHotspot = 0;

		mode = BRUSH;

		std::string name = "Layer 0";
		layerNames.push_back(name);
		Buffer buf;
		strcpy(buf.str, "Default");
		buffers.push_back(buf);

		glm::vec2 cellSize = glm::vec2(2.56f, 1.28f);
		m_CellSizes.push_back(cellSize);
		layerVisible.push_back(true);
	}

	void TileInspectorPanel::OnImGuiRender(Ref<Texture2D>& tile, std::vector<Ref<Texture2D>>& tiles, std::vector<Animation>& animations, std::vector <std::vector<glm::vec2>>& objectCoords, std::vector <std::vector<Entity>>& objects, Ref<Scene>& activeScene)
	{
		if (showPanel)
		{
			ImGui::Begin("Tilemap Inspector", &showPanel);
			if (m_CurrentFrame < 0)
				if (tiles.size() > 0)
					m_CurrentFrame = 0;
			if (m_CurrentFrame >= tiles.size())
				m_CurrentFrame = 0;

			float size = ImGui::GetWindowWidth() * 0.75;
			float sizeRatio;

			//ImGui::Dummy(ImVec2(0.0f, 20.0f ));


			ImGui::Columns(2);
			ImGui::SetColumnWidth(0 , 100.0f);
			ImGui::Text("Tools");
			ImGui::NextColumn();

			ImGui::PushMultiItemsWidths(2, ImGui::CalcItemWidth());
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 5.0f, 0 });

			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };
			
			ImGui::SameLine(10.0f);
			size = 32;
			//ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 2.4f));
			Ref<Texture2D> icon = mode == BRUSH ? m_ActivePaint : m_Paint;
			if (ImGui::ImageButton((ImTextureID)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 1), ImVec2(1, 0), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)))
			{
				mode = BRUSH;
			}
			ImGui::SameLine();
			icon = mode == ERASE ? m_ActiveErase : m_Erase;
			if (ImGui::ImageButton((ImTextureID)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 1), ImVec2(1, 0), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)))
			{
				mode = ERASE;
			}
			ImGui::SameLine();
			icon = mode == VOLUME ? m_ActiveVolume : m_Volume;
			if (ImGui::ImageButton((ImTextureID)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 1), ImVec2(1, 0), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)))
			{
				mode = VOLUME;
			}
			ImGui::SameLine();
			icon = mode == EYEDROPPER ? m_ActiveEyeDropper : m_EyeDropper;
			if (ImGui::ImageButton((ImTextureID)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 1), ImVec2(1, 0), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)))
			{
				mode = EYEDROPPER;
			}

			ImGui::SameLine();
			icon = mode == SELECT ? m_ActiveSelect : m_Select;
			if (ImGui::ImageButton((ImTextureID)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 1), ImVec2(1, 0), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)))
			{
				mode = SELECT;
			}

			ImGui::PopItemWidth();
			ImGui::PopStyleVar();
			ImGui::Columns(1);
			

			ImGui::Dummy(ImVec2(0.0f, 20.0f));

			DrawVec2Control("Cell Size", m_CellSizes[m_Selected_Layer]);

			ImGui::Dummy(ImVec2(0.0f, 10.0f));


			//layers

			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 100.f);
			ImGui::Text("Sort Layer");
			ImGui::NextColumn();


			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 37, 0 });

			ImGui::SameLine();
			//Layer visibility button
			Ref<Texture2D> visible;
			if (layerVisible[m_Selected_Layer])
				visible = m_Visible;
			else
				visible = m_NotVisible;
			if (ImGui::ImageButton((ImTextureID)visible->GetRendererID(), { 20.0f, 20.0f }, { 0, 1 }, { 1, 0 }))
			{
				if (layerVisible[m_Selected_Layer])
					layerVisible[m_Selected_Layer] = false;
				else
					layerVisible[m_Selected_Layer] = true;
			}

			ImGui::SameLine(44.0f);
			std::string fullName;
			if (strcmp(buffers[m_Selected_Layer].str, "") == 0)
				fullName = layerNames[m_Selected_Layer] + buffers[m_Selected_Layer].str;
			else
				fullName = layerNames[m_Selected_Layer] + ": " + buffers[m_Selected_Layer].str;

			ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove;
			if (ImGui::Button(const_cast<char*>(fullName.c_str())))
				ImGui::OpenPopup("my_select_popup");

			if (ImGui::BeginPopup("my_select_popup", flags))
			{
				for (int i = 0; i < layerNames.size(); i++)
				{
					std::string fullName;
					if (strcmp(buffers[i].str, "") == 0)
						fullName = layerNames[i] + buffers[i].str;
					else
						fullName = layerNames[i] + ": " + buffers[i].str;

					if (ImGui::Selectable(const_cast<char*>(fullName.c_str())))
						m_Selected_Layer = i;
				}
				if (ImGui::Button("Edit Layers..."))
				{
					editLayers = true;
				}
				ImGui::EndPopup();
			}
			ImGui::PopStyleVar();

			


			ImGui::SameLine();
			//Linked button
			Ref<Texture2D> image;
			if (linkedLayers)
				image = m_Link;
			else
				image = m_LinkBroke;
			if (ImGui::ImageButton((ImTextureID)image->GetRendererID(), { 20.0f, 20.0f }, { 0, 1 }, { 1, 0 }))
			{
				if (linkedLayers)
					linkedLayers = false;
				else
					ImGui::OpenPopup("link_popup");
			}

			if (ImGui::BeginPopup("link_popup", flags))
			{
				if (ImGui::Selectable("Link all grid layers to current layer \"Cell Size\"?"))
					linkedLayers = true;
				ImGui::EndPopup();
			}
			ImGui::Columns(1);

			ImGui::Dummy(ImVec2(0.0f, 20.0f));
			// drop down
			std::string selectedTile = "Selected Tile";
			ImGuiTreeNodeFlags tflags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;
			bool opened = ImGui::TreeNodeEx((void*)(intptr_t)0, tflags, selectedTile.c_str());
			float treePadding = ImGui::GetCursorPosY();

			ImGui::SetCursorPosX((ImGui::GetWindowWidth() * 0.5f) - (size * 0.5f));
			if (tiles.size() > 0)
				tile = tiles[0];
			else
				tile = NULL;

			if (animations.size() > 0)
			{
				m_CurrentAnimation = 0;
				tile = NULL;
			}


			if (opened)
			{
				size = ImGui::GetWindowWidth() * 0.75;

				SceneSerializer ss;
				if (tile != NULL)
				{
					if (!centerLoaded)
					{
						ss = SceneSerializer(tile);
						ss.DeserializeImageData();
						m_centerPoint = ss.GetSpriteRenderer().CenterPoint;
						//centerLoaded = true;
					}
					if (tiles.size() != m_CenterPoints.size())
					{
						m_CenterPoints.clear();
						for (int i = 0; i < tiles.size(); i++)
						{
							ss = SceneSerializer(tiles[i]);
							ss.DeserializeImageData();
							m_CenterPoints.push_back(ss.GetSpriteRenderer().CenterPoint);
						}
					}
					else
					{
						if (tiles.size() == 1)
							if (!centerLoaded)
							{
								ss = SceneSerializer(tile);
								ss.DeserializeImageData();
								m_CenterPoints[0] = ss.GetSpriteRenderer().CenterPoint;
								centerLoaded = true;
							}
					}
					//ImGui::SetCursorPosY((size / sizeRatio) + 20.0f + treePadding);
					ImGui::Dummy(ImVec2(0.0f, 10.0f));
					DrawVec2Control("Center Point", m_CenterPoints[m_CurrentFrame], 0.0f, 100.0f, tiles, m_CenterPoints, editingCenter);
				}
				else if (animations.size() >= 1)
				{
					if (!centerLoaded)
					{
						m_CenterPoints.clear();
						for (int i = 0; i < animations[0].GetSize(); i++)
						{
							ss = SceneSerializer(animations[0].GetFrame(i));
							ss.DeserializeImageData();
							m_centerPoint = ss.GetSpriteRenderer().CenterPoint;
							m_CenterPoints.push_back(m_centerPoint);
						}
						centerLoaded = true;
					}

					//ImGui::SetCursorPosY((size / sizeRatio) + 50.0f + treePadding);
					ImGui::Dummy(ImVec2(0.0f, 10.0f));
					DrawVec2Control("Center Point", m_CenterPoints[animations[0].GetCurrentFrame()], 0.0f, 100.0f, animations[0].GetImages(), m_CenterPoints, editingCenter);
				}

				else
				{
					//DrawVec2Control("Center Point ", glm::vec2(0.0f, 0.0f));
				}

				if (tile != NULL)
				{

					if (m_CurrentFrame >= m_CenterPoints.size())
						m_CurrentFrame = 0;
					//float w = tile.get()->GetWidth();
					//float h = tile.get()->GetHeight();
					float w = tiles[m_CurrentFrame].get()->GetWidth();
					float h = tiles[m_CurrentFrame].get()->GetHeight();

					sizeRatio = w / h;
					float centerRatio = w / size;

					//
					//ImGui::SetCursorPosY(treePadding + 500);
					ImGui::Dummy(ImVec2(0.0f, 10.0f));
					ImGui::SetCursorPosX((ImGui::GetWindowWidth() / 2) - (size / 2));
					//ImGui::SetCursorPosY(treePadding + 200);

					ImGui::Image((ImTextureID)m_Checkerboard->GetRendererID(), { size, size / sizeRatio }, { 0, 1 }, { 1, 0 });
					ImGui::SetItemAllowOverlap();
					ImGui::SameLine((ImGui::GetWindowWidth() / 2) - (size / 2));

					end = std::chrono::steady_clock::now();
					elapsed += std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
					if (elapsed >= 500.0f)
					{
						m_CurrentFrame++;
						if (m_CurrentFrame >= tiles.size())
						{
							m_CurrentFrame = 0;

						}
						elapsed = 0;
					}

					ImGui::Image((ImTextureID)tiles[m_CurrentFrame]->GetRendererID(), { size, size / sizeRatio }, { 0, 1 }, { 1, 0 });
					begin = std::chrono::steady_clock::now();

					if (m_CenterPoints.size() > 0)
					{
						if (editingCenter)
						{
							ImGui::SameLine((ImGui::GetWindowWidth() / 2) - (16) + (m_CenterPoints[0].x / centerRatio * 100.0f));
							ImGui::SetCursorPosY(treePadding + 20 + ((size / sizeRatio) / 2 + (16) + (m_CenterPoints[0].y / centerRatio * 100.0f)));
						}
						else
						{
							if (m_CurrentFrame >= m_CenterPoints.size())
								m_CurrentFrame = 0;
							ImGui::SameLine((ImGui::GetWindowWidth() / 2) - (16) + (m_CenterPoints[m_CurrentFrame].x / centerRatio * 100.0f));
							ImGui::SetCursorPosY(treePadding + 20 + ((size / sizeRatio) / 2 + (16) + (m_CenterPoints[m_CurrentFrame].y / centerRatio * 100.0f)));
						}

						ImGui::Image((ImTextureID)m_BlueCircle->GetRendererID(), { 32, 32 }, { 0, 1 }, { 1, 0 });
					}

				}
				else if (animations.size() > 0)
				{
					animations[0].Update(1.0f);

					float w = animations[0].GetFrame(animations[0].GetCurrentFrame()).get()->GetWidth();
					float h = animations[0].GetFrame(animations[0].GetCurrentFrame()).get()->GetHeight();

					sizeRatio = w / h;
					float centerRatio = w / size;

					ImGui::Dummy(ImVec2(0.0f, 10.0f));
					ImGui::SetCursorPosX((ImGui::GetWindowWidth() / 2) - (size / 2));
					//ImGui::SetCursorPosY(treePadding + 50);

					ImGui::Image((ImTextureID)m_Checkerboard->GetRendererID(), { size, size / sizeRatio }, { 0, 1 }, { 1, 0 });
					ImGui::SetItemAllowOverlap();
					ImGui::SameLine((ImGui::GetWindowWidth() / 2) - (size / 2));

					ImGui::Image((ImTextureID)animations[0].GetFrame(animations[0].GetCurrentFrame())->GetRendererID(), { size, size / sizeRatio }, { 0, 1 }, { 1, 0 });

					if (m_CenterPoints.size() > 0)
					{
						int frame = animations[0].GetCurrentFrame();
						if (editingCenter)
						{
							ImGui::SameLine((ImGui::GetWindowWidth() / 2) - (16) + (m_CenterPoints[0].x / centerRatio * 100.0f));
							ImGui::SetCursorPosY(treePadding + 20 + ((size / sizeRatio) / 2 + (16) + (m_CenterPoints[0].y / centerRatio * 100.0f)));
						}
						else
						{
							if (animations[0].GetCurrentFrame() >= m_CenterPoints.size())
								frame = 0;
							ImGui::SameLine((ImGui::GetWindowWidth() / 2) - (16) + (m_CenterPoints[frame].x / centerRatio * 100.0f));
							ImGui::SetCursorPosY(treePadding + 20 + ((size / sizeRatio) / 2 + (16) + (m_CenterPoints[frame].y / centerRatio * 100.0f)));
						}

						ImGui::Image((ImTextureID)m_BlueCircle->GetRendererID(), { 32, 32 }, { 0, 1 }, { 1, 0 });
					}
				}

				else
				{
					float sizeRatio = (m_NoImage.get()->GetWidth() / m_NoImage.get()->GetHeight());

					//ImGui::Dummy(ImVec2(0.0f, 10.0f));
					ImGui::SetCursorPosX((ImGui::GetWindowWidth() / 2) - (size / 2));
					ImGui::Image((ImTextureID)m_NoImage->GetRendererID(), { size, size / sizeRatio }, { 0, 1 }, { 1, 0 });
				}
				ImGui::TreePop();
			}


			ImGui::End();
		}

		// Edit draw layers. Add/Delete/Name
		if (editLayers)
		{
			
			ImGui::Begin("Edit Layers", &editLayers );
			
			for (int i = 0; i < layerNames.size(); i++)
			{
				if( i == m_Selected_Layer)
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0.3, 0.5, 1));
				else
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
				std::string draglabel = "##dragbutton" + std::to_string(i);
				ImGui::PushID(const_cast<char*>(draglabel.c_str()));
				if (ImGui::Button(draglabel.c_str(), ImVec2(ImGui::GetWindowWidth(), 25.0f)))
				{
					m_Selected_Layer = i;
				}
				if (ImGui::BeginDragDropSource())
				{
					std::string indexText = std::to_string(i);
					std::wstring widestr = std::wstring(indexText.begin(), indexText.end());
					const wchar_t* widecstr = widestr.c_str();

					ImGui::SetDragDropPayload("MOVE_LAYER", widecstr, (wcslen(widecstr)) * sizeof(wchar_t));
					//ImGui::ImageButton((ImTextureID)animation.GetFrame(i)->GetRendererID(), { 30.0f, 30.0f }, { 0, 1 }, { 1, 0 });
					//ImGui::SetCursorPosX(50);
					//ImGui::SetCursorPosY(15);

					//std::filesystem::path filePath = animation.GetFrame(i)->GetPath();
					//std::string fileName = filePath.filename().string();
					std::string dragText = layerNames[i] + ": " + buffers[i].str;
					ImGui::Text(dragText.c_str());
					ImGui::EndDragDropSource();
				}
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MOVE_LAYER"))
					{
						const wchar_t* payloadStr = (const wchar_t*)payload->Data; // string of the index of the item being dragged
						std::wstring ws(payloadStr);
						std::string indexStr(ws.begin(), ws.end());
						int index = std::stoi(indexStr);

						std::vector<std::string>& v = layerNames;
						std::vector<Buffer>& vb = buffers;
						std::vector<bool>& vlv = layerVisible;

						if (index > i)
						{
							//std::rotate(v.rend() - index - 1, v.rend() - index, v.rend() - i);
							std::rotate(vb.rend() - index - 1, vb.rend() - index, vb.rend() - i);
							std::rotate(vlv.rend() - index - 1, vlv.rend() - index, vlv.rend() - i);
							std::rotate(objects.rend() - index - 1, objects.rend() - index, objects.rend() - i);
							std::rotate(objectCoords.rend() - index - 1, objectCoords.rend() - index, objectCoords.rend() - i);
							
							for (int j = i; j <= index; j++)
							{
								for (int k = 0; k < objects[j].size(); k++)
								{
									if (objects[j][k].HasComponent<SpriteRendererComponent>())
										objects[j][k].GetComponent<SpriteRendererComponent>().Layer = j;
								}
							}

						}
						else
						{
							//std::rotate(v.begin() + index, v.begin() + index + 1, v.begin() + i + 1);
							std::rotate(vb.begin()				+ index, vb.begin()				+ index + 1, vb.begin()				+ i + 1);
							std::rotate(vlv.begin()				+ index, vlv.begin()			+ index + 1, vlv.begin()			+ i + 1);
							std::rotate(objects.begin()			+ index, objects.begin()		+ index + 1, objects.begin()		+ i + 1);
							std::rotate(objectCoords.begin()	+ index, objectCoords.begin()	+ index + 1, objectCoords.begin()	+ i + 1);

							for (int j = index; j <= i; j++)
							{
								for (int k = 0; k < objects[j].size(); k++)
								{
									if (objects[j][k].HasComponent<SpriteRendererComponent>())
										objects[j][k].GetComponent<SpriteRendererComponent>().Layer = j;
								}
							}
						}
						
					}
					ImGui::EndDragDropTarget();
				}
				ImGui::PopStyleColor();
				ImGui::PopID();
				ImGui::SetItemAllowOverlap();
				ImGui::SameLine(10.0f);

				//Layer visibility button
				std::string id = "##visible" + std::to_string(i);
				ImGui::PushID(const_cast<char*>(id.c_str()));
				Ref<Texture2D> visible;
				if (layerVisible[i])
					visible = m_Visible;
				else
					visible = m_NotVisible;
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0.3, 0.5, 1));
				if (ImGui::ImageButton((ImTextureID)visible->GetRendererID(), { 20.0f, 20.0f }, { 0, 1 }, { 1, 0 }))
				{
					if (layerVisible[i])
						layerVisible[i] = false;
					else
						layerVisible[i] = true;
				}
				ImGui::PopStyleColor();
				ImGui::PopStyleColor();
				ImGui::PopID();
				ImGui::SameLine();

				ImGui::Text(const_cast<char*>(layerNames[i].c_str()));

				ImGui::SameLine(110.0f);

				ImGui::PushItemWidth(128.0f);
				std::string label = "##" + std::to_string(i);
				ImGui::InputText(const_cast<char*>(label.c_str()), buffers[i].str, 64);
				ImGui::PopItemWidth();
				
				ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove;
				if (i != 0)
				{
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0.3, 0.5, 1));
					ImGui::SameLine();
					std::string label = "Delete##" + std::to_string(i);
					ImGui::PushID(const_cast<char*>(label.c_str()));

					std::string popupID = "delete_popup##" + std::to_string(i);

					if (ImGui::ImageButton((ImTextureID)m_Trash->GetRendererID(), { 20.0f, 20.0f }, { 0, 1 }, { 1, 0 }))
					{
						ImGui::OpenPopup(const_cast<char*>(popupID.c_str()));
					}
					ImGui::PopStyleColor();
					ImGui::PopStyleColor();

					if (ImGui::BeginPopup(const_cast<char*>(popupID.c_str()), flags))
					{
						size_t len = strlen(buffers[i].str);
						std::string text;
						if (len > 0)
							text = "Delete layer " + std::to_string(i) + ": " + buffers[i].str + "?";
						else
							text = "Delete layer " + std::to_string(i) + "?";

						const char* chrptr = text.c_str();

						if (ImGui::Selectable(chrptr))
						{
							if (i <= m_Selected_Layer)
								m_Selected_Layer -= 1;

							for (auto object : objects[i])
							{
								activeScene->DestroyEntity(object);
							}
							objectCoords.erase(objectCoords.begin() + i);
							objects.erase(objects.begin() + i);

							for (int j = i; j < objects.size(); j++)
							{
								for (auto object : objects[j])
								{
									if (object.HasComponent<SpriteRendererComponent>())
									{
										object.GetComponent<SpriteRendererComponent>().Layer = j;
									}
								}
							}
							layerNames.erase(layerNames.begin() + i);
							buffers.erase(buffers.begin() + i);
							layerVisible.erase(layerVisible.begin() + i);
							for (int i = 0; i < layerNames.size(); i++)
							{
								std::string newName = "Layer " + std::to_string(i);
								layerNames.at(i) = newName;
							}
						}
						ImGui::EndPopup();
					}
					ImGui::PopID();
				}
				
			}
			if (ImGui::Button("+ Add Layer", ImVec2(80, 25)))
			{
				std::string layerName = "Layer " + std::to_string(layerNames.size());
				layerNames.push_back(layerName);
				layerVisible.push_back(true);


				Buffer buffer;
				strcpy(buffer.str, "");
				buffers.push_back(buffer);

				objectCoords.resize(layerNames.size());
				objects.resize(layerNames.size());

				glm::vec2 cellSize = glm::vec2(2.56f, 1.28f);
				m_CellSizes.push_back(cellSize);

				//objectCoords.resize(layerNames.size());
				//objects.resize(layerNames.size());

			}

			ImGui::End();
		}


		//ImGui::ShowDemoWindow();
		
	}

	int TileInspectorPanel::GetDrawMode()
	{
		return mode;
	}

	void TileInspectorPanel::SetDrawMode(int newMode)
	{
		mode = newMode;
	}

	int TileInspectorPanel::GetSelectedLayer()
	{
		return m_Selected_Layer;
	}

	void TileInspectorPanel::SetCellSize(float x, float y)
	{
		m_CellSizes[m_Selected_Layer] = glm::vec2(x, y);
	}

	glm::vec2 TileInspectorPanel::GetCellSize()
	{
		return m_CellSizes[m_Selected_Layer];
	}

	std::vector<std::string> TileInspectorPanel::GetLayerNames()
	{
		return layerNames;
	}

	void TileInspectorPanel::SetLayerNames(std::vector<std::string> l)
	{
		layerNames = l;
	}

	std::vector<Buffer> TileInspectorPanel::GetBuffers()
	{
		return buffers;
	}

	std::vector<glm::vec2> TileInspectorPanel::GetLayerCells()
	{
		return m_CellSizes;
	}

	void TileInspectorPanel::SetLayerCells(std::vector<glm::vec2> cells)
	{
		m_CellSizes = cells;
	}

	void TileInspectorPanel::SetBuffers(std::vector<Buffer> b)
	{
		buffers = b;
	}

	void TileInspectorPanel::ClearLayers()
	{
		//m_CellSize = glm::vec2(2.56f, 1.28f);
		m_CellSizes.clear();
		glm::vec2 cellSize = glm::vec2(2.56f, 1.28f);
		m_CellSizes.push_back(cellSize);

		layerNames.clear();
		buffers.clear();
		m_Selected_Layer = 0;

		std::string name = "Layer 0";
		layerNames.push_back(name);
		Buffer buf;
		strcpy(buf.str, "Default");
		buffers.push_back(buf);
	}

	void TileInspectorPanel::SyncLayers()
	{
		for (int i = 0; i < m_CellSizes.size(); i++)
		{
			m_CellSizes[i] = m_CellSizes[m_Selected_Layer];
		}
	}

	void TileInspectorPanel::SetLinked(bool linked)
	{
		linkedLayers = linked;
	}

	bool TileInspectorPanel::IsGridLinked()
	{
		return linkedLayers;
	}

	std::vector<bool> TileInspectorPanel::GetVisibleLayers()
	{
		return layerVisible;
	}

	void TileInspectorPanel::SetVisibleLayers(std::vector<bool>& visibleLayers)
	{
		layerVisible = visibleLayers;
	}

	void TileInspectorPanel::SetShow(bool show)
	{
		showPanel = true;
	}

	void TileInspectorPanel::NewTileSelected()
	{
		centerLoaded = false;
		
	}

	void TileInspectorPanel::SetCenterPoint(glm::vec2 centerPoint)
	{
		m_CenterPoints.clear();
		m_CenterPoints.push_back(centerPoint);
	}

	std::vector<glm::vec2>& TileInspectorPanel::GetCenterPoints()
	{
		return m_CenterPoints;
	}

	static void DrawVec2Control(const std::string& label, glm::vec2& values, float resetValue, float columnWidth, std::vector<Ref<Texture2D>>& textures, std::vector<glm::vec2>& centerPoints, bool& editing)
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
			if (!label.compare("Cell Size")) // grid size
				values.x = 2.56f;
			else // center point
			{
				values.x = 0.0f;
				
				for (int i = 0; i < textures.size(); i++)
				{
					centerPoints[i].x = 0.0f;
					std::cout << "Saved Image META data!\n";
					SpriteRendererComponent src;
					src.Texture = textures[i];
					src.CenterPoint = centerPoints[0];
					SceneSerializer ss(src);
					ss.SerializeImageData();
				}
			}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();


		bool changingValues = false;
		if (label.compare("Cell Size"))
		{
			if (centerPoints.size() > 0)
			{
				ImGui::DragFloat("##X", &centerPoints[0].x, 0.01f, 0.0f, 0.0f, "%.2f");

				if (ImGui::IsItemActive())
					changingValues = true;

			}
		}
		else
			ImGui::DragFloat("##X", &values.x, 0.01f, 0.1f, 0.0f, "%.2f");

		if (label.compare("Cell Size")) //center point
		{
			if ((ImGui::IsItemDeactivatedAfterEdit()))
			{
				for (int i = 0; i < textures.size(); i++)
				{
					centerPoints[i] = centerPoints[0];
					std::cout << "Saved Image META data!\n";
					SpriteRendererComponent src;
					src.Texture = textures[i];
					src.CenterPoint = centerPoints[0];
					SceneSerializer ss(src);
					ss.SerializeImageData();
				}
			}
		}
		else // grid size
		{
			if (values.x < 0.1f)
				values.x = 0.1f;
		}

		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize))
		{
			if (!label.compare("Cell Size")) // grid size
				values.y = 1.28f;
			else // center point
			{
				values.y = 0.0f;

				for (int i = 0; i < textures.size(); i++)
				{
					centerPoints[i].y = 0.0f;
					std::cout << "Saved Image META data!\n";
					SpriteRendererComponent src;
					src.Texture = textures[i];
					src.CenterPoint = centerPoints[0];
					SceneSerializer ss(src);
					ss.SerializeImageData();
				}
			}
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();

		if (label.compare("Cell Size"))
		{
			if (centerPoints.size() > 0)
			{
				ImGui::DragFloat("##Y", &centerPoints[0].y, 0.01f, 0.0f, 0.0f, "%.2f");

				if (ImGui::IsItemActive())
					changingValues = true;
			}

			if (changingValues)
				editing = true;
			else
				editing = false;
		}
		else
			ImGui::DragFloat("##Y", &values.y, 0.01f, 0.1f, 0.0f, "%.2f");

		if (label.compare("Cell Size"))
		{
			if ((ImGui::IsItemDeactivatedAfterEdit()))
			{
				for (int i = 0; i < textures.size(); i++)
				{
					centerPoints[i] = centerPoints[0];
					std::cout << "Saved Image META data!\n";
					SpriteRendererComponent src;
					src.Texture = textures[i];
					src.CenterPoint = centerPoints[0];
					SceneSerializer ss(src);
					ss.SerializeImageData();
				}
			}
		}
		else
		{
			if (values.y < 0.1f)
				values.y = 0.1f;
		}

		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();
	}
}
