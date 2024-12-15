#pragma once

#include "Sloth/Renderer/Texture.h"
#include "TileInspectorPanel.h"
#include "Sloth/Core/Animation.h"
#include "Sloth/Core/Animator.h"

#include <filesystem>

namespace Sloth {

	class TilemapPanel
	{
	public:
		TilemapPanel();

		void OnImGuiRender(std::vector<Animation>& animationList, std::vector<Ref<Texture2D>>& tileList, TileInspectorPanel& tip);

		//Ref<Texture2D>* GetTile();

		void SetShow(bool show);

		void ClearSelected();

	private:

		void DrawFiles(std::vector<Animation>& animationList, std::vector<Ref<Texture2D>>& tileList, TileInspectorPanel& tip);

		std::filesystem::path m_CurrentDirectory;

		Ref<Texture2D> m_DirectoryIcon;
		Ref<Texture2D> m_FileIcon;
		Ref<Texture2D> m_BackButton;
		Ref<Texture2D> m_ForwardButton;
		Ref<Texture2D> m_RefreshButton;
		Ref<Texture2D> m_AnimatorIcon;

		Ref<Texture2D>* m_CurrentTile;
		std::vector<Ref<Texture2D>> tiles;
		std::vector<Animation> animations;
		std::vector<std::string> animators;
		std::vector<std::string>animatorPaths;
		Ref<Texture2D> m_LastClicked;
		Animation m_LastClickedAnimation;
		bool m_SelectAll = false;

		std::vector<std::filesystem::path> paths;
		std::vector<std::filesystem::path> m_ForwardPaths;


		int currentTile = 0;
		bool pageLoaded = false;
		bool newPage = false;
		bool mouseDownLast = false;
		bool listView = false;
		bool showPanel = true;
	};

}
