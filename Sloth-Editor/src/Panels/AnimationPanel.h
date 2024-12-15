#pragma once

#include "Sloth/Renderer/Texture.h"
#include "Sloth/Core/Buffer.h"
#include "Sloth/Core/Animation.h"
#include "Sloth.h"
#include "Sloth/Utils/PlatformUtils.h"


#include <filesystem>

namespace Sloth {



	class AnimationPanel
	{
	public:
		AnimationPanel();

		void OnImGuiRender();

		void SetShow(bool show);

		void LoadAnimation(std::string path);

		bool IsUpdated();
		void UpdateComplete();

		Animation& GetAnimation();

		void NewAnimation();

		

	private:

		Animation animation;
		//AnimatorPanel* m_AnimatorPanel;
		Ref<Texture2D> m_Checkerboard, m_Trash, m_Open, m_Save;
		float m_TimeMaster = 1.000f;
		float elapsed = 0.0f;
		int m_CurrentFrame = 0;
		bool showAnimationPanel = false;
		bool m_Updated = false;
		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();;
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		
		Buffer m_FileName;
		std::filesystem::path m_FilePath;
		std::string m_SavedName = "*Untitled Animation";


	};

}
