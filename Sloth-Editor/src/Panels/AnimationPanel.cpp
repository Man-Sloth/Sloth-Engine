#define OEMRESOURCE
//#include <windows.h>
#include <conio.h>

#include "slthpch.h"
#include "AnimationPanel.h"
#include "Sloth/Scene/SceneSerializer.h"
#include "Sloth/Renderer/AssetManager.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>


namespace Sloth {

	extern const std::filesystem::path g_AssetPath;

	AnimationPanel::AnimationPanel()
	{
		m_Checkerboard = Texture2D::Create("Resources/checkerboard.png");
		m_Trash = Texture2D::Create("Resources/Icons/Tilemap/trash.png");
		m_Save = Texture2D::Create("Resources/Icons/Floppy.png");
		m_Open = Texture2D::Create("Resources/Icons/Open.png");
		m_FileName = Buffer();
	}

	void AnimationPanel::OnImGuiRender()
	{
		if (showAnimationPanel)
		{

			ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar;;
			ImGui::Begin("Animation", &showAnimationPanel, window_flags);

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			//Top menu bar
			if (ImGui::BeginMenuBar())
			{
				ImGui::SetCursorPosX(10);
				if (ImGui::ImageButton((ImTextureID)m_Open->GetRendererID(), { 18, 18 }, { 0, 1 }, { 1, 0 }))
				{
					std::string filepath = FileDialogs::OpenFile("Animation (*.anm)\0*.anm\0");
					std::filesystem::path pathName = filepath;
					std::string fileName = pathName.filename().string();

					if (!filepath.empty())
					{
						//SceneSerializer ss;
						//ss.DeserializeAnimation(filepath);
						for (int i = 0; i < animation.GetSize(); i++)
						{
							AssetManager::DeleteTexture(animation.GetFrame(i).get()->GetPath());
						}
						animation = Animation(filepath);
						for (int i = 0; i < animation.GetSize(); i++)
						{
							AssetManager::LoadTexture(animation.GetFrame(i).get()->GetPath());
						}
						//std::vector<std::string> framePaths = ss.GetFramePaths();
						//std::vector<float> frameTimes = ss.GetFrameTimes();
						//animation.SetImages(framePaths, frameTimes);
						strcpy(m_FileName.str, fileName.c_str());
						m_CurrentFrame = 0;

						m_FilePath = filepath;
						m_SavedName = fileName;
					}
				}
				ImGui::SetCursorPosX(40);
				if (ImGui::ImageButton((ImTextureID)m_Save->GetRendererID(), { 18, 18 }, { 0, 1 }, { 1, 0 }))
				{
					std::string file = m_FileName.str;

					if (m_FilePath.empty())
					{
						std::string filepath = FileDialogs::SaveFile("Animation (*.anm)\0*.anm\0");
						std::filesystem::path pathName = filepath;
						std::string fileName = pathName.filename().string();

						if (!filepath.empty())
						{
							SceneSerializer ss;
							std::vector <Ref<Texture2D>> images = animation.GetImages();
							std::vector <std::string> paths;
							std::vector <float> times = animation.GetTimes();
							for (int i = 0; i < images.size(); i++)
							{
								std::string path = images[i].get()->GetPath();
								paths.push_back(path);
							}

							ss.SerializeAnimation(paths, times, filepath);
							m_FilePath = filepath;
							m_SavedName = fileName;
							m_Updated = true;
						}
					}
					else
					{
						SceneSerializer ss;
						std::vector <Ref<Texture2D>> images = animation.GetImages();
						std::vector <std::string> paths;
						std::vector <float> times = animation.GetTimes();
						for (int i = 0; i < images.size(); i++)
						{
							std::string path = images[i].get()->GetPath();
							paths.push_back(path);
						}

						ss.SerializeAnimation(paths, times, m_FilePath);
						m_Updated = true;
					}

				}
				ImGui::PushItemWidth(128.0f);
				ImGui::Text(m_SavedName.c_str());
				ImGui::PopItemWidth();
				if (!m_FilePath.empty())
				{
					if (ImGui::Button("X"))
					{
						m_FilePath = "";
						m_SavedName = "*Untitled Animation";
						animation.ClearFrames();
						animation = Animation();
						m_CurrentFrame = 0;
						elapsed = 0.0f;
					}
				}
				ImGui::EndMenuBar();
			}
			ImGui::PopStyleColor();

			//Button that covers whole window for dropping payloads
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
			int width = ImGui::GetWindowWidth() - 15;
			int height = ImGui::GetWindowHeight() - 35;
			ImGui::SetCursorPosY(ImGui::GetScrollY() + 25);// move the payload button with the window's scrolled offset to stay centered in window
			ImGui::Button("##Window", ImVec2(width, height));

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TILE_ITEM"))
				{
					const wchar_t* path = (const wchar_t*)payload->Data; // Bring large string of all images brought over
					std::wstring ws(path);
					std::string allPaths(ws.begin(), ws.end());
					int pos = 0;
					int count = 0;
					std::string target = ".png";
					std::vector<int> positions;
					while ((pos = allPaths.find(target, pos)) != std::string::npos) { //save how many times .png shows up to separate paths
						count++;
						pos += target.length();
						positions.push_back(pos);
					}

					std::vector<std::string> subPaths;
					pos = 0;
					for (int i = 0; i < count; i++)
					{
						subPaths.push_back(allPaths.substr(pos, positions[i] - pos)); // Save each path
						pos += subPaths[i].length();
					}


					for (int i = 0; i < subPaths.size(); i++) // load each image from separated  paths
					{
						std::wstring widestr = std::wstring(subPaths[i].begin(), subPaths[i].end());
						const wchar_t* path = widestr.c_str();
						std::filesystem::path texturePath = std::filesystem::path(path);
						Ref<Texture2D> texture = AssetManager::LoadTexture(texturePath.string());
						if (texture->IsLoaded())
							animation.AddFrame(texture);
						else
							SLTH_WARN("Could not load texture {0}", texturePath.filename().string());
					}
				}
				ImGui::EndDragDropTarget();
			}
			ImGui::PopStyleColor(); ImGui::PopStyleColor(); ImGui::PopStyleColor();
			ImGui::SetItemAllowOverlap();
			ImGui::SetCursorPosY(60.0f);

			//Checkerboard background
			float size = 200.0f;
			ImGui::SetCursorPosX((ImGui::GetWindowWidth() / 2) - (size / 2));
			ImGui::Image((ImTextureID)m_Checkerboard->GetRendererID(), ImVec2(size, size), { 0, 1 }, { 1, 0 });
			ImGui::Dummy(ImVec2(0, 3));
			ImGui::SetItemAllowOverlap();

			// Animation of Currently loaded frames

			end = std::chrono::steady_clock::now();
			elapsed += std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
			if (animation.GetSize() > 0)
			{
				if (elapsed >= animation.GetFrameTime(m_CurrentFrame) * 1000)
				{
					m_CurrentFrame++;
					if (m_CurrentFrame >= animation.GetSize())
					{
						m_CurrentFrame = 0;

					}
					elapsed = 0;
				}
				float sizeX = animation.GetFrame(m_CurrentFrame).get()->GetWidth();
				float sizeY = animation.GetFrame(m_CurrentFrame).get()->GetHeight();
				float sizeRatio = sizeY / sizeX;

				float xOffset = size / sizeRatio;
				float yOffset = size * sizeRatio;

				if (sizeY >= sizeX)
				{
					ImGui::SetCursorPosY(60.0f);
					ImGui::SetCursorPosX((ImGui::GetWindowWidth() / 2) - (xOffset / 2));
					ImGui::Image((ImTextureID)animation.GetFrame(m_CurrentFrame)->GetRendererID(), { xOffset, size }, { 0, 1 }, { 1, 0 });
				}
				else if (sizeX > sizeY)
				{
					ImGui::SetCursorPosY((60.0f) + (size * 0.25f));
					ImGui::SetCursorPosX((ImGui::GetWindowWidth() / 2) - (size / 2));
					ImGui::Image((ImTextureID)animation.GetFrame(m_CurrentFrame)->GetRendererID(), { size, yOffset }, { 0, 1 }, { 1, 0 });
				}

			}
			begin = std::chrono::steady_clock::now();

			ImGui::SetCursorPosY(260.0f);

			if (animation.GetSize() > 0)
			{
				//Amount of animation frames
				ImGui::SetCursorPosX((ImGui::GetWindowWidth() / 2) - (size / 4));
				std::string animationSize = "Animation Size: " + std::to_string(animation.GetSize());
				ImGui::Text(animationSize.c_str());

				ImGui::SetCursorPosX((ImGui::GetWindowWidth() / 2) - (size / 4));
				std::string curFrame = "Current Frame: " + std::to_string(m_CurrentFrame + 1);
				ImGui::Text(curFrame.c_str());
			}

			ImGui::Dummy(ImVec2(0, 5));
			ImGui::SetCursorPosX((25));

			// Master frame time slider
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::Text("Master Frame Time");
			ImGui::SameLine(200);
			ImGui::SetNextItemWidth(100);
			ImGui::DragFloat("##TimeMaster", &m_TimeMaster, 0.001f, 0.0f, 0.0f, "%.3f");
			if (m_TimeMaster < 0.001f)
				m_TimeMaster = 0.001f;
			if (animation.GetSize() > 0)
			{
				if (ImGui::IsItemActive())
				{
					for (int i = 0; i < animation.GetSize(); i++)
					{
						animation.SetFrameTime(i, m_TimeMaster);
					}
				}
			}

			ImGui::SameLine();

			ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove;
			std::string label = "Delete##Clear";
			ImGui::PushID(const_cast<char*>(label.c_str()));

			std::string popupID = "delete_popup##Clear";

			if (ImGui::ImageButton((ImTextureID)m_Trash->GetRendererID(), { 20.0f, 20.0f }, { 0, 1 }, { 1, 0 }))
			{
				ImGui::OpenPopup(const_cast<char*>(popupID.c_str()));
			}

			if (ImGui::BeginPopup(const_cast<char*>(popupID.c_str()), flags))
			{
				std::string text = "Clear all animation frames?";

				const char* chrptr = text.c_str();

				if (ImGui::Selectable(chrptr))
				{
					animation.ClearFrames();
					m_CurrentFrame = 0;
				}
				ImGui::EndPopup();
			}
			ImGui::PopStyleColor();
			ImGui::PopID();
			ImGui::Separator();
			ImGui::Dummy(ImVec2(0, 20));

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			// Draw all Frames in animation in a list
			size = 25.0f;
			for (int i = 0; i < animation.GetSize(); i++)
			{
				std::string label = "##frame" + std::to_string(i);
				ImGui::PushID(const_cast<char*>(label.c_str()));
				if (ImGui::Button(label.c_str(), ImVec2(ImGui::GetWindowWidth(), 25.0f)))
				{

				}
				if (ImGui::BeginDragDropSource())
				{
					std::string frameText = std::to_string(i);
					std::wstring widestr = std::wstring(frameText.begin(), frameText.end());
					const wchar_t* widecstr = widestr.c_str();

					ImGui::SetDragDropPayload("MOVE_FRAME", widecstr, (wcslen(widecstr)) * sizeof(wchar_t));
					ImGui::ImageButton((ImTextureID)animation.GetFrame(i)->GetRendererID(), { 30.0f, 30.0f }, { 0, 1 }, { 1, 0 });
					ImGui::SetCursorPosX(50);
					ImGui::SetCursorPosY(15);

					std::filesystem::path filePath = animation.GetFrame(i)->GetPath();
					std::string fileName = filePath.filename().string();
					ImGui::Text(fileName.c_str());
					ImGui::EndDragDropSource();
				}
				
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MOVE_FRAME"))
					{
						const wchar_t* payloadStr = (const wchar_t*)payload->Data; // string of the index of the item being dragged
						std::wstring ws(payloadStr);
						std::string indexStr(ws.begin(), ws.end());
						int index = std::stoi(indexStr);

						std::vector<Ref<Texture2D>>& v = animation.GetImages();
						std::vector<float>& vt = animation.GetTimes();

						if (index > i)
						{
							std::rotate(v.rend() - index - 1, v.rend() - index, v.rend() - i);
							std::rotate(vt.rend() - index - 1, vt.rend() - index, vt.rend() - i);
						}
						else
						{
							std::rotate(v.begin() + index, v.begin() + index + 1, v.begin() + i + 1);
							std::rotate(vt.begin() + index, vt.begin() + index + 1, vt.begin() + i + 1);
						}
					}
					ImGui::EndDragDropTarget();
				}
				ImGui::PopID();
				ImGui::SetItemAllowOverlap();
				ImGui::SameLine(10.0f);


				std::string frameNumber = std::to_string(i + 1) + ": ";
				ImGui::Text(frameNumber.c_str());
				ImGui::SameLine(40);

				std::filesystem::path filePath = animation.GetFrame(i)->GetPath();
				std::string fileName = filePath.filename().string();
				ImGui::Image((ImTextureID)animation.GetFrame(i)->GetRendererID(), ImVec2(size, size), { 0, 1 }, { 1, 0 }); //Image of frame

				ImGui::SameLine();
				ImGui::Text(fileName.c_str()); // frame name
				ImGui::SameLine(200);
				ImGui::SetNextItemWidth(100);
				label = "##Time" + std::to_string(i);
				ImGui::DragFloat(const_cast<char*>(label.c_str()), &animation.GetFrameTime(i), 0.001f, 0.0f, 0.0f, "%.3f"); // drag box to change value of frame time
				if (animation.GetFrameTime(i) < 0.001f)
					animation.GetFrameTime(i) = 0.001f;
				ImGui::SameLine();

				ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove;
				label = "Delete##" + std::to_string(i);
				ImGui::PushID(const_cast<char*>(label.c_str()));

				std::string popupID = "delete_popup##" + std::to_string(i);

				if (ImGui::ImageButton((ImTextureID)m_Trash->GetRendererID(), { 20.0f, 20.0f }, { 0, 1 }, { 1, 0 })) // delete frame button
				{
					ImGui::OpenPopup(const_cast<char*>(popupID.c_str()));
				}

				if (ImGui::BeginPopup(const_cast<char*>(popupID.c_str()), flags)) // confirm delete pop-up
				{
					std::string text = "Delete frame " + std::to_string(i + 1) + ": " + fileName + "?";

					const char* chrptr = text.c_str();

					if (ImGui::Selectable(chrptr))
					{
						animation.DeleteFrame(i);
						m_CurrentFrame = 0;
					}
					ImGui::EndPopup();
				}
				ImGui::PopID();
				ImGui::Separator(); // dividing line between frames
			}
			ImGui::PopStyleColor();

			ImGui::End();
		}
	}

	void AnimationPanel::SetShow(bool show)
	{
		showAnimationPanel = show;
	}

	void AnimationPanel::LoadAnimation(std::string path)
	{
		if (!path.empty())
		{
			std::string filepath = path;
			std::filesystem::path pathName = filepath;
			std::string fileName = pathName.filename().string();
			//fileName.erase(fileName.length() - 4);

			animation = Animation(path);
			std::vector<std::string> framePaths = animation.GetPaths();
			std::vector<float> frameTimes = animation.GetTimes();
			strcpy(m_FileName.str, fileName.c_str());
			m_CurrentFrame = 0;

			m_FilePath = filepath;
			m_SavedName = fileName;
		}
		else
		{
			//Not a valid path
		}

	}

	bool AnimationPanel::IsUpdated()
	{
		return m_Updated;
	}

	void AnimationPanel::UpdateComplete()
	{
		m_Updated = false;
	}

	Animation& AnimationPanel::GetAnimation()
	{
		return animation;
	}
}
