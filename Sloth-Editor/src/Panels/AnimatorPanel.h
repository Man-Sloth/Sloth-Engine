#pragma once

#include "Sloth/Renderer/Texture.h"
#include "Sloth/Core/Buffer.h"
//#include "Sloth/Core/Animation.h"
#include "Sloth/Core/Animator.h"
#include "Sloth.h"
#include "Sloth/Utils/PlatformUtils.h"
#include "AnimationPanel.h"

#include <filesystem>

namespace Sloth {

	class AnimatorPanel
	{
	public:
		AnimatorPanel(AnimationPanel& ap);

		void OnImGuiRender();

		void SetShow(bool show);

		void UpdateAnimation(std::string path);



	private:

		Animator m_Animator;
		AnimationPanel* m_AnimationPanel;
		std::vector<bool> radioButtons;
		Ref<Texture2D> m_Checkerboard, m_Trash, m_Open, m_Save, m_Edit;
		float m_TimeMaster = 1.000f;
		float elapsed = 0.0f;
		//int m_CurrentFrame = 0;
		bool showAnimatorPanel = false;
		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		Buffer m_FileName;
		std::filesystem::path m_FilePath;
		std::string m_SavedName = "*Untitled Animator Controller";


	};

}
