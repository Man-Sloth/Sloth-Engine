#include "slthpch.h"
#include "TilemapPanel.h"

#include <imgui/imgui.h>

#include <thread>


namespace Sloth {

	// Once we have projects, change this
	extern const std::filesystem::path g_TilePath = "assets/Sprites";
	extern const std::filesystem::path g_AssetsPath = "assets";

	bool compareAnimations(Animation& a1, Animation& a2)
	{
		return(a1.GetName() == a2.GetName());
	}

	TilemapPanel::TilemapPanel()
		: m_CurrentDirectory(g_TilePath)
	{
		m_DirectoryIcon = Texture2D::Create("Resources/Icons/ContentBrowser/DirectoryIcon.png");
		m_FileIcon = Texture2D::Create("Resources/Icons/ContentBrowser/FileIcon.png");
		m_BackButton = Texture2D::Create("Resources/Icons/ContentBrowser/BackButtonSmall.png");
		m_ForwardButton = Texture2D::Create("Resources/Icons/ContentBrowser/ForwardButtonSmall.png");
		m_RefreshButton = Texture2D::Create("Resources/Icons/ContentBrowser/RefreshButton.png");
		m_AnimatorIcon = Texture2D::Create("Resources/Icons/ContentBrowser/AnimatorIcon.png");
	}

	void TilemapPanel::OnImGuiRender(std::vector<Animation>& animationList, std::vector<Ref<Texture2D>>& tileList, TileInspectorPanel& tip)
	{
		if (showPanel)
		{
			ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar;;

			if (!ImGui::Begin("Tiles", &showPanel, window_flags))
			{
				
				// Early out if the window is collapsed, as an optimization.
				ImGui::End();
				return;
			}

			if (ImGui::BeginMenuBar())
			{
				bool isABack = false;
				if (m_CurrentDirectory != std::filesystem::path(g_TilePath))
				{
					isABack = true;
					float backSize = 20.0f;
					if (ImGui::ImageButton((ImTextureID)m_BackButton->GetRendererID(), { backSize, backSize }, { 0, 1 }, { 1, 0 }))
					{
						m_ForwardPaths.push_back(m_CurrentDirectory);
						m_CurrentDirectory = m_CurrentDirectory.parent_path();
						pageLoaded = false;
						for (int t = 0; t < tiles.size(); t++)
							AssetManager::DeleteTexture(tiles[t].get()->GetPath());
						tiles.clear();
						paths.clear();
						animations.clear();
						animators.clear();
						animatorPaths.clear();
						mouseDownLast = true;
					}
				}

				if (m_ForwardPaths.size() > 0)
				{
					if (isABack)
						ImGui::SameLine(0.0f, 0.0f);
					float backSize = 20.0f;
					if (ImGui::ImageButton((ImTextureID)m_ForwardButton->GetRendererID(), { backSize, backSize }, { 0, 1 }, { 1, 0 }))
					{
						m_CurrentDirectory = m_ForwardPaths[m_ForwardPaths.size() - 1];
						pageLoaded = false;
						for (int t = 0; t < tiles.size(); t++)
							AssetManager::DeleteTexture(tiles[t].get()->GetPath());
						tiles.clear();
						paths.clear();
						animations.clear();
						animators.clear();
						animatorPaths.clear();
						mouseDownLast = true;
						m_ForwardPaths.pop_back();
					}
				}

				std::string filenameString = m_CurrentDirectory.filename().string();
				ImGui::Text(filenameString.c_str());


				ImGui::SameLine(ImGui::GetWindowWidth() - 90.0f);
				float backSize = 20.0f;
				if (ImGui::ImageButton((ImTextureID)m_RefreshButton->GetRendererID(), { backSize, backSize }, { 0, 1 }, { 1, 0 }))
				{
					pageLoaded = false;
					for (int t = 0; t < tiles.size(); t++)
						AssetManager::DeleteTexture(tiles[t].get()->GetPath());
					tiles.clear();
					paths.clear();
					animations.clear();
					animators.clear();
					animatorPaths.clear();
					mouseDownLast = true;
				}

				ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove;
				ImGui::SameLine(0.0f, 0.0f);
				if (ImGui::Button("View", { 50.0f, 25.0f }))
					ImGui::OpenPopup("my_select_popup");

				ImGui::SameLine();

				if (ImGui::BeginPopup("my_select_popup", flags))
				{
					if (ImGui::Selectable("Tile View"))
					{
						listView = false;
					}
					if (ImGui::Selectable("List View"))
					{
						listView = true;
					}
					ImGui::EndPopup();
				}
				ImGui::EndMenuBar();
			}

			DrawFiles(animationList, tileList, tip);

			ImGui::End();
		}

		if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
		{
			mouseDownLast = false;
		}

	}

	void TilemapPanel::DrawFiles(std::vector<Animation>& animationList, std::vector<Ref<Texture2D>>& tileList, TileInspectorPanel& tip)
	{
		static float padding = 10.0f;
		static float thumbnailSize = 60.0f;
		float cellSize = thumbnailSize + padding;

		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = (int)(panelWidth / cellSize);
		if (columnCount < 1)
			columnCount = 1;

		if(!listView)
			ImGui::Columns(columnCount, 0, false);
		bool skipImages = false;

		//Draw all folders
		for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
		{
			const auto& path = directoryEntry.path();
			std::string filenameString = path.filename().string();
			if (directoryEntry.is_directory())
			{
				ImGui::PushID(filenameString.c_str());

				Ref<Texture2D> icon = m_DirectoryIcon;

				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

				// Draw Folder Button
				if (!listView)
				{
					ImGui::ImageButton((ImTextureID)icon->GetRendererID(), { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 });

					if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					{
						m_CurrentDirectory /= path.filename();
						m_ForwardPaths.clear();
						pageLoaded = false;
						for (int t = 0; t < tiles.size(); t++)
							AssetManager::DeleteTexture(tiles[t].get()->GetPath());
						
						tiles.clear();
						paths.clear();
						animations.clear();
						animators.clear();
						animatorPaths.clear();
						skipImages = true;
					}
				}
				else
				{
					
					ImGui::Button("", ImVec2(ImGui::GetWindowWidth(), 25.0f));

					if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					{
						m_CurrentDirectory /= path.filename();
						m_ForwardPaths.clear();
						pageLoaded = false;
						for (int t = 0; t < tiles.size(); t++)
							AssetManager::DeleteTexture(tiles[t].get()->GetPath());
						tiles.clear();
						paths.clear();
						animations.clear();
						animators.clear();
						animatorPaths.clear();
						skipImages = true;
					}
					ImGui::SetItemAllowOverlap();

					ImGui::SameLine(10.0f);
					
					//ImGui::SetCursorPos(ImVec2(0, 0));
					ImGui::Image((ImTextureID)icon->GetRendererID(), { 25.0f, 25.0f }, { 0, 1 }, { 1, 0 });
					ImGui::SameLine();
					ImGui::TextWrapped(filenameString.c_str());

				}
				ImGui::PopStyleColor();

				// show name of folder
				if (!listView)
				{
					ImGui::TextWrapped(filenameString.c_str());
					ImGui::NextColumn();
				}

				ImGui::PopID();
			}
			else
			{
				const auto& path = directoryEntry.path();
				std::string filename = path.filename().string();
				std::string extension = "." + filename.substr(filename.find_last_of(".") + 1);
				if (!skipImages) // Skip .pngs on the first pass to display folders first
				{

					if (!pageLoaded) // on the very first frame, load all images in directory
					{
						if (!extension.compare(".png"))
						{
							paths.push_back(path);

							//Ref<Texture2D> sprite = Texture2D::Create(path.string().c_str());
							Ref<Texture2D> sprite = AssetManager::LoadTexture(path.string());
							tiles.push_back(sprite);
						}
						
						if (!extension.compare(".anm"))
						{
							std::string animationPath = path.string();
							Animation animation = Animation(animationPath);
							animations.push_back(animation);
						}

						if (!extension.compare(".anmtr"))
						{
							std::string animatorPath = path.string();
							std::filesystem::path pathName = animatorPath;
							std::string fileName = pathName.filename().string();
							animators.push_back(fileName);
							animatorPaths.push_back(animatorPath);
						}
					}
				}
			}
		}
		if (!skipImages) //when all folders are displayed, display .pngs
		{
			pageLoaded = true;

			//Draw sprites
			for (int i = 0; i < tiles.size(); i++)
			{
				std::string filenameString = paths[i].filename().string();

				ImGui::PushID(filenameString.c_str());

				Ref<Texture2D> image = tiles[i];

				bool found = false;
				for (int j = 0; j < tileList.size(); j++)
				{
					if(tileList[j].get()->GetPath() == image.get()->GetPath())
						found = true;
				}
				if (found)
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5, 0.5, 0.5, 0.5));
				else
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

				bool control = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
				bool shift = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);
				bool a = false;
				if (ImGui::IsWindowFocused())
				{
					a = Input::IsKeyPressed(Key::A);
				}
				bool selectAll = false;
				if (control && a)
					selectAll = true;


				if (selectAll)
				{
					tileList.clear();
					for (int j = 0; j < tiles.size(); j++)
					{
						tileList.push_back(tiles[j]);
					}
					//tile = tiles[0];
					tip.NewTileSelected();
					m_SelectAll = true;
				}

				if (!listView) // regular icon view
				{
					// Select tile to be painted
					
					if (ImGui::ImageButton((ImTextureID)image->GetRendererID(), { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 }))
					{
						if (control) // multi select 1 item at a time
						{
							int foundIndex = -1;
							for (int i = 0; i < tileList.size(); i++)
							{
								if (image == tileList[i])
									foundIndex = i;
							}

							if (foundIndex >= 0)
							{
								if (foundIndex != tileList.size() - 1)
									tileList.erase(tileList.begin() + foundIndex);
								else
								{
									if(tileList.size() != 1)
										tileList.pop_back();
								}
							}
							else
							{
								tileList.push_back(image);
								m_LastClicked = image;
							}
							animationList.clear();
						}
						else if (shift) // multi select range
						{
							if (tileList.size() > 0 && !m_SelectAll)
							{
								tileList.clear();
								bool startSelecting = false;
								bool forward = false;
								for (int j = 0; j < tiles.size(); j++)
								{
									if (!startSelecting)
									{
										if (tiles[j] == m_LastClicked)
										{
											startSelecting = true;
											forward = true;
										}
										else if (tiles[j] == image)
											startSelecting = true;
									}

									if (startSelecting)
										tileList.push_back(tiles[j]);

									if (image == tiles[j] && forward)
										break;
									else if (tiles[j] == m_LastClicked && !forward)
										break;
								}
								animationList.clear();
							}
							else
							{
								tileList.clear();
								tileList.push_back(image);
								m_LastClicked = image;
								m_SelectAll = false;
							}
						}
						else
						{
							tileList.clear();
							tileList.push_back(image);
							m_LastClicked = image;
							animationList.clear();
						}

						//tile = image;
						tip.NewTileSelected();
					}

					if (ImGui::BeginDragDropSource())
					{
						bool isSelected = false;
						for (int i = 0; i < tileList.size(); i++)
						{
							if (tileList[i] == image)
								isSelected = true;
						}

						if (!isSelected)
						{
							tileList.clear();
							tileList.push_back(image);
						}
						std::filesystem::path path = image.get()->GetPath();
						auto relativePath = std::filesystem::relative(path, g_AssetsPath);
						const wchar_t* itemPath = relativePath.c_str();


						size_t length = 0;
						std::string allPaths;
						for (int i = 0; i < tileList.size(); i++)
						{
							std::filesystem::path path = tileList[i].get()->GetPath();
							auto relativePath = std::filesystem::relative(path, g_AssetsPath);
							const wchar_t* itemPath = relativePath.c_str();

							allPaths.append(tileList[i].get()->GetPath());
							length += (wcslen(itemPath) + 1);
						}

						std::wstring widestr = std::wstring(allPaths.begin(), allPaths.end());
						const wchar_t* widecstr = widestr.c_str();

						const wchar_t* item = itemPath;
						ImGui::SetDragDropPayload("TILE_ITEM", widecstr, (wcslen(widecstr)) * sizeof(wchar_t));
						for (int k = 0; k < tileList.size(); k++)
						{
							if (k < 5)
							{
								ImGui::ImageButton((ImTextureID)tileList[k]->GetRendererID(), { 20, 20 }, { 0, 1 }, { 1, 0 });
								ImGui::SameLine();
							}
							else
							{
								ImGui::Text("...");
								break;
							}
						}
						ImGui::EndDragDropSource();
					}
					// show name of file
					ImGui::TextWrapped(filenameString.c_str());
				}
				else
				{

					if(ImGui::Button("", ImVec2(ImGui::GetWindowWidth(), 25.0f)))
					{
						if (control) // multi select 1 item at a time
						{
							int foundIndex = -1;
							for (int i = 0; i < tileList.size(); i++)
							{
								if (image == tileList[i])
									foundIndex = i;
							}

							if (foundIndex >= 0)
							{
								if (foundIndex != tileList.size() - 1)
									tileList.erase(tileList.begin() + foundIndex);
								else
								{
									if (tileList.size() != 1)
										tileList.pop_back();
								}
							}
							else
							{
								tileList.push_back(image);
								m_LastClicked = image;
							}
						}
						else if (shift) // multi select range
						{
							if (tileList.size() > 0 && !m_SelectAll)
							{
								tileList.clear();
								bool startSelecting = false;
								bool forward = false;
								for (int j = 0; j < tiles.size(); j++)
								{
									if (!startSelecting)
									{
										if (tiles[j] == m_LastClicked)
										{
											startSelecting = true;
											forward = true;
										}
										else if (tiles[j] == image)
											startSelecting = true;
									}

									if (startSelecting)
										tileList.push_back(tiles[j]);

									if (image == tiles[j] && forward)
										break;
									else if (tiles[j] == m_LastClicked && !forward)
										break;
								}
							}
							else
							{
								tileList.clear();
								tileList.push_back(image);
								m_LastClicked = image;
								m_SelectAll = false;
							}
						}
						else
						{
							tileList.clear();
							tileList.push_back(image);
							m_LastClicked = image;
						}

						//tile = image;
						tip.NewTileSelected();
					}
					if (ImGui::BeginDragDropSource())
					{
						bool isSelected = false;
						for (int i = 0; i < tileList.size(); i++)
						{
							if (tileList[i] == image)
								isSelected = true;
						}

						if (!isSelected)
						{
							tileList.clear();
							tileList.push_back(image);
						}
						std::filesystem::path path = image.get()->GetPath();
						auto relativePath = std::filesystem::relative(path, g_AssetsPath);
						const wchar_t* itemPath = relativePath.c_str();


						size_t length = 0;
						std::string allPaths;
						for (int i = 0; i < tileList.size(); i++)
						{
							std::filesystem::path path = tileList[i].get()->GetPath();
							auto relativePath = std::filesystem::relative(path, g_AssetsPath);
							const wchar_t* itemPath = relativePath.c_str();

							allPaths.append(tileList[i].get()->GetPath());
							length += (wcslen(itemPath) + 1);
						}

						std::wstring widestr = std::wstring(allPaths.begin(), allPaths.end());
						const wchar_t* widecstr = widestr.c_str();

						const wchar_t* item = itemPath;
						ImGui::SetDragDropPayload("TILE_ITEM", widecstr, (wcslen(widecstr)) * sizeof(wchar_t));
						ImGui::EndDragDropSource();
					}

					ImGui::SetItemAllowOverlap();

					ImGui::SameLine(10.0f);

					ImGui::Image((ImTextureID)image->GetRendererID(), { 25.0f, 25.0f }, { 0, 1 }, { 1, 0 });
					ImGui::SameLine();
					ImGui::TextWrapped(filenameString.c_str());

				}
				ImGui::PopStyleColor();

				if(!listView)
					ImGui::NextColumn();

				ImGui::PopID();
			}
			//Draw animations
			for (int i = 0; i < animations.size(); i++)
			{
				animations[i].Update(1.0f);
				Ref<Texture2D> image = animations[i].GetFrame(animations[i].GetCurrentFrame());
				//Animation animation = animations[i];

				bool found = false;
				for (int j = 0; j < animationList.size(); j++)
				{
					if (compareAnimations(animationList[j], animations[i]))
					{
						found = true;
						tileList.clear();
					}
				}
				if (found)
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5, 0.5, 0.5, 0.5));
				else
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

				bool control = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
				bool shift = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);
				bool a = false;
				if (ImGui::IsWindowFocused())
				{
					if(tileList.size() == 0)
						a = Input::IsKeyPressed(Key::A);
				}
				bool selectAll = false;
				if (control && a)
					selectAll = true;


				if (selectAll)
				{
					animationList.clear();
					for (int j = 0; j < animations.size(); j++)
					{
						animationList.push_back(animations[j]);
					}
					tip.NewTileSelected();
					m_SelectAll = true;
				}

				if (!listView) // regular icon view
				{
					float x = ImGui::GetCursorPosX();
					float y = ImGui::GetCursorPosY();
					
					
					if (ImGui::ImageButton((ImTextureID)image->GetRendererID(), { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 }))
					{
						
					}

					ImGui::SetItemAllowOverlap();
					ImGui::SetCursorPos(ImVec2(x,y));

					std::string id = "##animation" + std::to_string(i);
					ImGui::PushID(const_cast<char*>(id.c_str()));
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
					if (ImGui::Button("", ImVec2(thumbnailSize, thumbnailSize)))
					{
						if (control) // multi select 1 item at a time
						{
							int foundIndex = -1;
							for (int i = 0; i < animationList.size(); i++)
							{
								if (compareAnimations(animationList[i], animations[i]))
									foundIndex = i;
							}

							if (foundIndex >= 0)
							{
								if (foundIndex != animationList.size() - 1)
									animationList.erase(animationList.begin() + foundIndex);
								else
								{
									if (animationList.size() != 1)
										animationList.pop_back();
								}
							}
							else
							{
								animationList.push_back(animations[i]);
								m_LastClickedAnimation = animations[i];
							}
						}
						else if (shift) // multi select range
						{
							if (animationList.size() > 0 && !m_SelectAll)
							{
								animationList.clear();
								bool startSelecting = false;
								bool forward = false;
								for (int j = 0; j < animations.size(); j++)
								{
									if (!startSelecting)
									{
										if (compareAnimations(animations[j], m_LastClickedAnimation))
										{
											startSelecting = true;
											forward = true;
										}
										else if (compareAnimations(animations[j], animations[i]))
											startSelecting = true;
									}

									if (startSelecting)
										animationList.push_back(animations[j]);

									if (compareAnimations(animations[j], animations[i]) && forward)
										break;
									else if (compareAnimations(animations[j], m_LastClickedAnimation) && !forward)
										break;
								}
							}
							else
							{
								animationList.clear();
								animationList.push_back(animations[i]);
								m_LastClickedAnimation = animations[i];
								m_SelectAll = false;
							}
						}
						else
						{
							animationList.clear();
							animationList.push_back(animations[i]);
							m_LastClickedAnimation = animations[i];
						}

						//tile = image;
						tip.NewTileSelected();
					}
					ImGui::PopStyleColor(); ImGui::PopStyleColor(); ImGui::PopStyleColor();
					ImGui::PopID();
					if (ImGui::BeginDragDropSource())
					{
						bool isSelected = false;
						for (int i = 0; i < animationList.size(); i++)
						{
							if (compareAnimations(animationList[i], animations[i]))
								isSelected = true;
						}

						if (!isSelected)
						{
							animationList.clear();
							animationList.push_back(animations[i]);
						}

						size_t length = 0;
						std::string allPaths;
						for (int i = 0; i < animationList.size(); i++)
						{
							std::filesystem::path path = animationList[i].GetPath();
							auto relativePath = std::filesystem::relative(path, g_AssetsPath);
							const wchar_t* itemPath = relativePath.c_str();

							allPaths.append(animationList[i].GetPath());
							length += (wcslen(itemPath) + 1);
						}

						std::wstring widestr = std::wstring(allPaths.begin(), allPaths.end());
						const wchar_t* widecstr = widestr.c_str();

						ImGui::SetDragDropPayload("ANIMATION_ITEM", widecstr, (wcslen(widecstr)) * sizeof(wchar_t));
						for (int k = 0; k < animationList.size(); k++)
						{
							if (k < 5)
							{
								ImGui::ImageButton((ImTextureID)animationList[k].GetFrame(animationList[k].GetCurrentFrame())->GetRendererID(), { 20, 20 }, { 0, 1 }, { 1, 0 });
								ImGui::SameLine();
							}
							else
							{
								ImGui::Text("...");
								break;
							}
						}
						ImGui::EndDragDropSource();
					}

					// show name of file
					ImGui::TextWrapped(animations[i].GetName().c_str());
					ImGui::PopStyleColor();
					ImGui::NextColumn();
				}
				else
				{
					if (ImGui::Button("", ImVec2(ImGui::GetWindowWidth(), 25.0f)))
					{
					}
					ImGui::SetItemAllowOverlap();
					ImGui::SameLine(10.0f);
					ImGui::Image((ImTextureID)image->GetRendererID(), { 25.0f, 25.0f }, { 0, 1 }, { 1, 0 });
					ImGui::SameLine();
					ImGui::TextWrapped(animations[i].GetName().c_str());
					ImGui::PopStyleColor();
				}
				//animation.SetLoadImages(false);
			}
			// Draw Animators
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			for (int i = 0; i < animators.size(); i++)
			{
				std::string id = "animatorButton##" + std::to_string(i);
				ImGui::PushID(id.c_str());
				if (ImGui::ImageButton((ImTextureID)m_AnimatorIcon->GetRendererID(), { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 }))
				{
					
				}
				if (ImGui::BeginDragDropSource())
				{
					std::string allPaths = animatorPaths[i];
					std::wstring widestr = std::wstring(allPaths.begin(), allPaths.end());
					const wchar_t* widecstr = widestr.c_str();
					ImGui::SetDragDropPayload("ANIMATORCONTROLLER_ITEM", widecstr, (wcslen(widecstr)) * sizeof(wchar_t));
					ImGui::Text(allPaths.c_str());
					ImGui::EndDragDropSource();
				}
				ImGui::PopID();
				ImGui::TextWrapped(animators[i].c_str());
				ImGui::NextColumn();
			}
			ImGui::PopStyleColor();


			if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
			{
				tileList.clear();
				animationList.clear();
			}

		}

		ImGui::Columns(1);

		ImGui::Dummy(ImVec2(0, 20.0f));
		std::string numTiles = "Tiles Selected: " + std::to_string(tileList.size());
		ImGui::Text(const_cast<char*>(numTiles.c_str()));
		ImGui::Dummy(ImVec2(0, 20.0f));

		if (!listView)
		{
			ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 16, 512);
			ImGui::SliderFloat("Padding", &padding, 0, 32);
		}
	}

	//Ref<Texture2D>* TilemapPanel::GetTile() {
	//	return m_CurrentTile;
	//}

	void TilemapPanel::SetShow(bool show)
	{
		showPanel = show;
	}

	void TilemapPanel::ClearSelected()
	{
		tiles.clear();
	}
}
