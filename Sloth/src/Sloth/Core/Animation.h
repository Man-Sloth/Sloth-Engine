#pragma once
#include "Sloth/Renderer/Texture.h"
#include <filesystem>

namespace Sloth {

	class Animation
	{
	public:
		Animation();
		Animation(std::string path);
		Animation(const Animation& a);       //copy constructor

		~Animation();
		void Update(float speed);
		Ref<Texture2D> GetFrame(int frameIndex);
		float& GetFrameTime(int frameIndex);
		void SetFrameTime(int frameIndex, float newTime);
		int GetSize();
		void AddFrame(Ref<Texture2D>& texture);
		void DeleteFrame(int frameIndex);
		void ClearFrames();
		std::vector<Ref<Texture2D>>& GetImages();
		std::vector<float>& GetTimes();
		void SetImages(std::vector<std::string>& paths, std::vector<float>& times);
		std::string GetName();
		int GetCurrentFrame();
		void SetFrame(int frame);
		std::string GetPath();
		std::vector<std::string>& GetPaths();
		void SetLoadImages(bool load);
		void SetCenterPoints(std::vector<glm::vec2>& centerPoints);
		glm::vec2 GetCenterPoint(int index);
		std::vector<glm::vec2>& GetCenterPoints();


	private:
		std::vector<Ref<Texture2D>> images;
		std::vector<glm::vec2> m_CenterPoints;
		std::vector<std::string> framePaths;
		std::vector<float> frameTimes;
		int currentFrame = 0;
		float elapsed = 0.0f;
		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();;
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		std::string name = "";
		std::string m_Path = "";
		bool loadedImages = false;
	};
}
