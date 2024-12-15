#pragma once

#include "Sloth/Renderer/Texture.h"
#include "Sloth/Core/Buffer.h"
#include "Sloth.h"

#include <filesystem>

namespace Sloth {



	class TileInspectorPanel
	{
	public:
		TileInspectorPanel();

		void OnImGuiRender(Ref<Texture2D>& tile, std::vector<Ref<Texture2D>>& tiles, std::vector<Animation>& animations, std::vector <std::vector<glm::vec2>>& objectCoords, std::vector <std::vector<Entity>>& objects, Ref<Scene>& activeScene);
		int GetDrawMode();
		void SetDrawMode(int newMode);

		Ref<Texture2D>* GetTile();
		int GetSelectedLayer();

		void SetCellSize(float x, float y);
		glm::vec2 GetCellSize();

		std::vector<std::string> GetLayerNames();
		void SetLayerNames(std::vector<std::string> l);
		std::vector<Buffer> GetBuffers();
		void SetBuffers(std::vector<Buffer> b);

		std::vector<glm::vec2> GetLayerCells();
		void SetLayerCells(std::vector<glm::vec2> cells);

		void SetShow(bool show);
		void NewTileSelected();
		
		void ClearLayers();
		void SyncLayers();
		void SetLinked(bool linked);
		bool IsGridLinked();

		std::vector<bool> GetVisibleLayers();
		void SetVisibleLayers(std::vector<bool>& visibleLayers);

		void SetCenterPoint(glm::vec2 centerPoint);
		std::vector<glm::vec2>& GetCenterPoints();

	private:

		Ref<Texture2D>* m_CurrentTile;
		std::vector<Ref<Texture2D>*> m_CurrentTiles;
		int m_CurrentFrame = -1;
		int m_CurrentAnimation = -1;
		bool editingCenter = false;

		Ref<Texture2D> m_Paint;
		Ref<Texture2D> m_Erase;
		Ref<Texture2D> m_Volume;
		Ref<Texture2D> m_EyeDropper;
		Ref<Texture2D> m_Select;

		Ref<Texture2D> m_ActivePaint;
		Ref<Texture2D> m_ActiveErase;
		Ref<Texture2D> m_ActiveVolume;
		Ref<Texture2D> m_ActiveEyeDropper;
		Ref<Texture2D> m_ActiveSelect;
		Ref<Texture2D> m_Link;
		Ref<Texture2D> m_LinkBroke;
		Ref<Texture2D> m_Checkerboard;
		Ref<Texture2D> m_BlueCircle;
		Ref<Texture2D> m_Visible;
		Ref<Texture2D> m_NotVisible;
		Ref<Texture2D> m_Trash;

		Ref<Texture2D> m_NoImage;

		HICON m_ArrowIcon;
		HCURSOR m_ArrowCursor;
		ICONINFO m_Ico;
		std::vector<std::string> layerNames = std::vector<std::string>(1);
		std::vector<Buffer> buffers = std::vector<Buffer>(1);
		std::vector<bool> layerVisible;
		int m_Selected_Layer = 0;

		enum drawMode { BRUSH = 0, ERASE = 1, VOLUME = 2, EYEDROPPER = 3, SELECT = 4};
		int mode;

		std::vector<glm::vec2> m_CellSizes;
		glm::vec2 m_CellSize = glm::vec2(2.56f, 1.28f);
		glm::vec2 m_centerPoint = glm::vec2(0.0f, 0.0f);
		std::vector<glm::vec2> m_CenterPoints;
		
		bool showPanel = true;
		bool editLayers = false;
		bool linkedLayers = true;
		bool centerLoaded = false;

		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();;
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		float elapsed = 0.0f;

	};

}
