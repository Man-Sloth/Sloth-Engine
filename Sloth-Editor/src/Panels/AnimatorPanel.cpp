#define OEMRESOURCE
#include <conio.h>

#include "slthpch.h"
#include "AnimatorPanel.h"
#include "Sloth/Scene/SceneSerializer.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace Sloth {

	extern const std::filesystem::path g_AssetPath;

	AnimatorPanel::AnimatorPanel(AnimationPanel& ap)
	{
		m_Checkerboard = Texture2D::Create("Resources/checkerboard.png");
		//AssetManager::LoadTexture("Resources/checkerboard.png");
		m_Trash = Texture2D::Create("Resources/Icons/Tilemap/trash.png");
		m_Save = Texture2D::Create("Resources/Icons/Floppy.png");
		m_Open = Texture2D::Create("Resources/Icons/Open.png");
		m_Edit = Texture2D::Create("Resources/Icons/Edit.png");
		m_FileName = Buffer();
		m_AnimationPanel = &ap;
	}

	void AnimatorPanel::OnImGuiRender()
	{
		if (showAnimatorPanel)
		{
			if (m_AnimationPanel->IsUpdated())
			{
				std::string animPath = m_AnimationPanel->GetAnimation().GetPath();

				UpdateAnimation(animPath);

				m_AnimationPanel->UpdateComplete();
			}

			ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar;;
			ImGui::Begin("Animator Controller", &showAnimatorPanel, window_flags);

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			//Top menu bar
			if (ImGui::BeginMenuBar())
			{
				ImGui::SetCursorPosX(10);
				if (ImGui::ImageButton((ImTextureID)m_Open->GetRendererID(), { 18, 18 }, { 0, 1 }, { 1, 0 }))
				{

					std::string filepath = FileDialogs::OpenFile("Animator (*.anmtr)\0*.anmtr\0");
					std::filesystem::path pathName = filepath;
					std::string fileName = pathName.filename().string();
	
					//fileName.erase(fileName.length() - 4);

					if (!filepath.empty())
					{
						m_Animator = Animator();
						SceneSerializer ss;
						ss.DeserializeAnimator(filepath);

						
						m_Animator.SetAnimations(ss.GetAnimations());
						m_Animator.SetTags(ss.GetTags());
						m_TimeMaster = ss.GetSpeed();
						m_Animator.SetSpeed(m_TimeMaster);

						for (int i = 0; i < m_Animator.GetSize(); i++)
						{
							if (i == 0)
								radioButtons.push_back(true);
							else
								radioButtons.push_back(false);

							//Animation animation = m_Animator.GetAnimation(i);
							//for (int j = 0; j < animation.GetSize(); j++)
							//{
							//	AssetManager::LoadTexture(animation.GetFrame(j).get()->GetPath());
							//}
						}
						m_FilePath = filepath;
						m_SavedName = fileName;

						//strcpy(m_FileName.str, fileName.c_str());
					}
				}
				ImGui::SetCursorPosX(40);
				if (ImGui::ImageButton((ImTextureID)m_Save->GetRendererID(), { 18, 18 }, { 0, 1 }, { 1, 0 }))
				{
					if (m_FilePath.empty())
					{
						std::string filepath = FileDialogs::SaveFile("Animator (*.anmtr)\0*.anmtr\0");
						std::filesystem::path pathName = filepath;
						std::string fileName = pathName.filename().string();

						if (!filepath.empty())
						{
							SceneSerializer ss;
							std::vector<Animation> animations = m_Animator.GetAnimations();
							std::vector<std::string> paths;
							std::vector<Buffer> tags = m_Animator.GetTags();
							for (int i = 0; i < animations.size(); i++)
							{
								std::string path = animations[i].GetPath();
								paths.push_back(path);
							}
							ss.SerializeAnimator(paths, tags, filepath, m_TimeMaster);

							m_FilePath = filepath;
							m_SavedName = fileName;
						}
					}
					else
					{
						if (!m_FilePath.empty())
						{
							SceneSerializer ss;
							std::vector<Animation> animations = m_Animator.GetAnimations();
							std::vector<std::string> paths;
							std::vector<Buffer> tags = m_Animator.GetTags();
							for (int i = 0; i < animations.size(); i++)
							{
								std::string path = animations[i].GetPath();
								paths.push_back(path);
							}
							ss.SerializeAnimator(paths, tags, m_FilePath, m_TimeMaster);
						}
					}
				}
				ImGui::PushItemWidth(128.0f);
				ImGui::Text(m_SavedName.c_str());

				if (!m_FilePath.empty())
				{
					if (ImGui::Button("X"))
					{
						m_FilePath = "";
						m_SavedName = "*Untitled Animator Controller";
						m_Animator = Animator();
						m_TimeMaster = 1.0f;
					}
				}

				ImGui::PopItemWidth();
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
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ANIMATION_ITEM"))
				{
					const wchar_t* path = (const wchar_t*)payload->Data; // Bring large string of all images brought over
					std::wstring ws(path);
					std::string allPaths(ws.begin(), ws.end());
					int pos = 0;
					int count = 0;
					std::string target = ".anm";
					std::vector<int> positions;
					while ((pos = allPaths.find(target, pos)) != std::string::npos) { //save how many times .anm shows up to separate paths
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


					for (int i = 0; i < subPaths.size(); i++) // load each animation from separated  paths
					{
						std::wstring widestr = std::wstring(subPaths[i].begin(), subPaths[i].end());
						const wchar_t* path = widestr.c_str();
						std::filesystem::path animationPath = std::filesystem::path(path);
						std::string pathString = animationPath.string();
						Animation animation = Animation(pathString);

						//m_Animations.push_back(animation);
						m_Animator.AddAnimation(animation);
						for (int i = 0; i < animation.GetSize(); i++)
						{
							AssetManager::LoadTexture(animation.GetFrame(i).get()->GetPath());
						}
						if (m_Animator.GetSize() == 1)
							radioButtons.push_back(true);
						else
							radioButtons.push_back(false);

						Buffer b;
						std::string tagString = "Tag " + std::to_string(m_Animator.GetSize());
						strcpy(b.str, tagString.c_str());
						m_Animator.AddTag(b);
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
			Animation animation;
			// Animation of Currently loaded frames
			if (m_Animator.GetSize() > 0)
			{
				m_Animator.Update();
				m_Animator.SetSpeed(m_TimeMaster);
				animation = m_Animator.GetCurrentAnimation();

				//copy to animation didn't add a number count to the textures so we will manually add them
				for (int i = 0; i < animation.GetSize(); i++)
					AssetManager::LoadTexture(animation.GetFrame(i).get()->GetPath());

				if (animation.GetSize() > 0)
				{
					float sizeX = animation.GetFrame(animation.GetCurrentFrame()).get()->GetWidth();
					float sizeY = animation.GetFrame(animation.GetCurrentFrame()).get()->GetHeight();
					float sizeRatio = sizeY / sizeX;

					float xOffset = size / sizeRatio;
					float yOffset = size * sizeRatio;

					if (sizeY >= sizeX)
					{
						ImGui::SetCursorPosY(60.0f);
						ImGui::SetCursorPosX((ImGui::GetWindowWidth() / 2) - (xOffset / 2));
						ImGui::Image((ImTextureID)animation.GetFrame(animation.GetCurrentFrame())->GetRendererID(), { xOffset, size }, { 0, 1 }, { 1, 0 });
					}
					else if (sizeX > sizeY)
					{
						ImGui::SetCursorPosY((60.0f) + (size * 0.25f));
						ImGui::SetCursorPosX((ImGui::GetWindowWidth() / 2) - (size / 2));
						ImGui::Image((ImTextureID)animation.GetFrame(animation.GetCurrentFrame())->GetRendererID(), { size, yOffset }, { 0, 1 }, { 1, 0 });
					}

				}
			}

			ImGui::SetCursorPosY(260.0f);
			//Amount of animation frames
			if (m_Animator.GetSize() > 0)
			{
				ImGui::SetCursorPosX((ImGui::GetWindowWidth() / 2) - (size / 4));
				std::string animationSize = "Animation Size: " + std::to_string(animation.GetSize());
				ImGui::Text(animationSize.c_str());

				ImGui::SetCursorPosX((ImGui::GetWindowWidth() / 2) - (size / 4));
				std::string curFrame = "Current Frame: " + std::to_string(m_Animator.GetCurrentAnimation().GetCurrentFrame() + 1);
				ImGui::Text(curFrame.c_str());
			}

			ImGui::Dummy(ImVec2(0, 5));
			ImGui::SetCursorPosX((25));

			// Animation Speed Slider
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::Text("Animation Speed Multiplier");
			ImGui::SameLine(200);
			ImGui::SetNextItemWidth(100);
			ImGui::DragFloat("##AnimationSpeed", &m_TimeMaster, 0.001f, 0.0f, 0.0f, "%.3f");
			if (m_TimeMaster < 0.001f)
				m_TimeMaster = 0.001f;
			if (animation.GetSize() > 0)
			{
				if (ImGui::IsItemActive())
				{
					for (int i = 0; i < animation.GetSize(); i++)
					{
						//animation.SetFrameTime(i, m_TimeMaster);
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
				std::string text = "Clear all animations?";

				const char* chrptr = text.c_str();

				if (ImGui::Selectable(chrptr))
				{
					for (int j = 0; j < m_Animator.GetSize(); j++)
					{
						animation = m_Animator.GetAnimation(j);
						for (int i = 0; i < animation.GetSize(); i++)
						{
							AssetManager::DeleteTexture(animation.GetFrame(i).get()->GetPath());
						}
					}
					m_Animator.ClearAnimations();
					radioButtons.clear();
				}
				ImGui::EndPopup();
			}
			ImGui::PopStyleColor();
			ImGui::PopID();
			ImGui::Separator();
			ImGui::Dummy(ImVec2(0, 20));

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			// Draw list of animations
			size = 25.0f;
			for (int i = 0; i < m_Animator.GetSize(); i++)
			{
				std::string label = "##Animation" + std::to_string(i);
				ImGui::PushID(const_cast<char*>(label.c_str()));
				if (ImGui::Button(label.c_str(), ImVec2(ImGui::GetWindowWidth(), 25.0f)))
				{
					// dummy button to stretch the length of the button
					for (int j = 0; j < radioButtons.size(); j++)
						radioButtons[j] = false;
					m_Animator.SetAnimation(i);
					radioButtons[i] = true;
				}
				if (ImGui::BeginDragDropSource())
				{
					std::string frameText = std::to_string(i);
					std::wstring widestr = std::wstring(frameText.begin(), frameText.end());
					const wchar_t* widecstr = widestr.c_str();

					ImGui::SetDragDropPayload("MOVE_ANIMATION", widecstr, (wcslen(widecstr)) * sizeof(wchar_t));
					ImGui::ImageButton((ImTextureID)m_Animator.GetAnimation(i).GetFrame(0)->GetRendererID(), {30.0f, 30.0f}, {0, 1}, {1, 0});
					ImGui::SetCursorPosX(50);
					ImGui::SetCursorPosY(15);

					ImGui::Text(m_Animator.GetAnimation(i).GetName().c_str());
					ImGui::EndDragDropSource();
				}

				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MOVE_ANIMATION"))
					{
						const wchar_t* payloadStr = (const wchar_t*)payload->Data; // string of the index of the item being dragged
						std::wstring ws(payloadStr);
						std::string indexStr(ws.begin(), ws.end());
						int index = std::stoi(indexStr);

						//std::vector<Ref<Texture2D>>& v = animation.GetImages();
						std::vector<Animation>& v = m_Animator.GetAnimations();
						std::vector<float>& vt = animation.GetTimes();
						std::vector<bool>& r = radioButtons;
						std::vector<Buffer>& b = m_Animator.GetTags();

						if (index > i)
						{
							std::rotate(v.rend() - index - 1, v.rend() - index, v.rend() - i);
							//std::rotate(vt.rend() - index - 1, vt.rend() - index, vt.rend() - i);
							std::rotate(r.rend() - index - 1, r.rend() - index, r.rend() - i);
							std::rotate(b.rend() - index - 1, b.rend() - index, b.rend() - i);
						}
						else
						{
							std::rotate(v.begin() + index, v.begin() + index + 1, v.begin() + i + 1);
							std::rotate(vt.begin() + index, vt.begin() + index + 1, vt.begin() + i + 1);
							std::rotate(r.begin() + index, r.begin() + index + 1, r.begin() + i + 1);
							std::rotate(b.begin() + index, b.begin() + index + 1, b.begin() + i + 1);
						}
						for (int j = 0; j < r.size(); j++)
							if (r[j])
								m_Animator.SetAnimation(j);
						m_Animator.SetFrame(0);
					}
					ImGui::EndDragDropTarget();
				}
				ImGui::PopID();
				ImGui::SetItemAllowOverlap();
				ImGui::SameLine(10.0f);


				//std::string frameNumber = std::to_string(i + 1) + ": ";
				//ImGui::Text(frameNumber.c_str());
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8, 0.8, 0.8, 1));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 0.2, 0.8, 1));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0.8, 1));
				label = "##Animation" + std::to_string(i);
				bool active = false;
				if (radioButtons[i])
					active = true;

				if (ImGui::RadioButton(const_cast<char*>(label.c_str()), active))
				{
					for (int j = 0; j < radioButtons.size(); j++)
						radioButtons[j] = false;

					radioButtons[i] = true;
				}
				ImGui::PopStyleColor(); ImGui::PopStyleColor(); ImGui::PopStyleColor();
				ImGui::SameLine(40);

				std::string fileName = m_Animator.GetAnimation(i).GetName();
				ImGui::Image((ImTextureID)m_Animator.GetAnimation(i).GetFrame(0)->GetRendererID(), ImVec2(size, size), {0, 1}, {1, 0}); //Image of frame

				ImGui::SameLine();
				ImGui::Text(fileName.c_str()); // frame name

				ImGui::SameLine(220);
				ImGui::PushItemWidth(128.0f);
				std::string tag = "##tag" + std::to_string(i); // animation tag
				ImGui::InputText(tag.c_str(), m_Animator.GetTag(i).str, 64);
				ImGui::PopItemWidth();

				//ImGui::SetNextItemWidth(100);
				//label = "##Time" + std::to_string(i);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2, 0.2, 0.2, 1));
				//ImGui::DragFloat(const_cast<char*>(label.c_str()), &m_Animations[i].GetFrameTime(0), 0.001f, 0.0f, 0.0f, "%.3f"); // drag box to change value of frame time
				//if (m_Animations[i].GetFrameTime(0) < 0.001f)
				//	m_Animations[i].GetFrameTime(0) = 0.001f;
				ImGui::SameLine();

				label = "##Edit" + std::to_string(i);
				ImGui::PushID(label.c_str());
				if (ImGui::ImageButton((ImTextureID)m_Edit->GetRendererID(), { 20.0f, 20.0f }, { 0, 1 }, { 1, 0 })) // Edit animation
				{
					m_AnimationPanel->SetShow(true);
					m_AnimationPanel->LoadAnimation(m_Animator.GetAnimation(i).GetPath());
					
				}
				ImGui::PopID();
				ImGui::SameLine();
				ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove;
				label = "Delete##" + std::to_string(i);
				ImGui::PushID(const_cast<char*>(label.c_str()));

				std::string popupID = "delete_popup##" + std::to_string(i);

				if (ImGui::ImageButton((ImTextureID)m_Trash->GetRendererID(), { 20.0f, 20.0f }, { 0, 1 }, { 1, 0 })) // delete frame button
				{
					ImGui::OpenPopup(const_cast<char*>(popupID.c_str()));
				}
				ImGui::PopStyleColor();

				if (ImGui::BeginPopup(const_cast<char*>(popupID.c_str()), flags)) // confirm delete pop-up
				{
					std::string text = "Delete animation " + std::to_string(i + 1) + ": " + fileName + "?";

					const char* chrptr = text.c_str();

					if (ImGui::Selectable(chrptr))
					{
						if (m_Animator.GetSize() > 1)
						{
							radioButtons.erase(radioButtons.begin() + i);
						}
						else
						{
							radioButtons.pop_back();
						}
						animation = m_Animator.GetAnimation(i);
						for (int i = 0; i < animation.GetSize(); i++)
						{
							AssetManager::DeleteTexture(animation.GetFrame(i).get()->GetPath());
						}
						m_Animator.DeleteAnimation(i);
					}
					if(radioButtons.size() > 0)
						radioButtons[m_Animator.GetAnimationIndex()] = true;
					ImGui::EndPopup();
				}
				ImGui::PopID();
				ImGui::Separator(); // dividing line between frames
			}
			ImGui::PopStyleColor();

			ImGui::End();
		}
	}

	void AnimatorPanel::SetShow(bool show)
	{
		showAnimatorPanel = show;
	}

	void AnimatorPanel::UpdateAnimation(std::string path)
	{
		for (int i = 0; i < m_Animator.GetSize(); i++)
		{
			std::string currentPath = m_Animator.GetAnimation(i).GetPath();
			if (m_Animator.GetAnimation(i).GetPath().compare(path) != 0)
			{
				// this is not the same animation as the one loaded in the animation window
			}
			else
			{
				m_Animator.ReloadAnimation(i, Animation(path));
			}
		}
	}
}
