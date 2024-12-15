#define OEMRESOURCE

#include "slthpch.h"
#include "Animation.h"
#include "Sloth/Scene/SceneSerializer.h"
#include "Sloth.h"

namespace Sloth {

	Animation::Animation()
	{}

	Animation::Animation(std::string path)
	{
		std::string filepath = path;
		std::filesystem::path pathName = filepath;
		name = pathName.filename().string();
		std::string fileName = pathName.filename().string();
		fileName.erase(fileName.length() - 4);
		currentFrame = 0;
		m_Path = path;

		if (!filepath.empty())
		{
			SceneSerializer ss;
			ss.DeserializeAnimation(filepath);
			framePaths = ss.GetFramePaths();
			std::vector<float> frameTimes = ss.GetFrameTimes();
			SetImages(framePaths, frameTimes);
			SetCenterPoints(ss.GetCenterPoints());
			m_Path = filepath;
			currentFrame = 0;
		}
	}

	Animation::Animation(const Animation& a)
	{
		images = a.images;
		framePaths = a.framePaths;
		frameTimes = a.frameTimes;
		name = a.name;
		m_CenterPoints = a.m_CenterPoints;
		m_Path = a.m_Path;
		// To insure temp variables don't mess up image management, "load" a copy of the images when an animation copy is made
		for (int i = 0; i < images.size(); i++)
		{
			AssetManager::LoadTexture(a.images[i].get()->GetPath());
		}
	}

	Animation::~Animation()
	{
		for (int i = 0; i < images.size(); i++)
			AssetManager::DeleteTexture(images[i].get()->GetPath());
	}

	void Animation::Update(float speed)
	{
		if (frameTimes.size() > 0)
		{
			end = std::chrono::steady_clock::now();
			elapsed += std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
			if (elapsed >= frameTimes[currentFrame] * 1000 / speed)
			{
				elapsed = 0.0f;
				currentFrame++;
			}
			if (currentFrame >= images.size())
				currentFrame = 0;
			begin = std::chrono::steady_clock::now();
		}
	}

	Ref<Texture2D> Animation::GetFrame(int frameIndex)
	{
		return images[frameIndex];
	}

	float& Animation::GetFrameTime(int frameIndex)
	{
		return frameTimes[frameIndex];
	}

	void Animation::SetFrameTime(int frameIndex, float newTime)
	{
		frameTimes[frameIndex] = newTime;
	}

	int Animation::GetSize()
	{
		return images.size();
	}

	void Animation::AddFrame(Ref<Texture2D>& texture)
	{
		images.push_back(texture);
		frameTimes.push_back(1.0f);
	}

	void Animation::DeleteFrame(int frameIndex)
	{
		if (images.size() > 1)
		{
			AssetManager::DeleteTexture(images[frameIndex].get()->GetPath());
			images.erase(images.begin() + frameIndex);
			frameTimes.erase(frameTimes.begin() + frameIndex);
		}
		else
		{
			AssetManager::DeleteTexture(images[frameIndex].get()->GetPath());
			images.pop_back();
			frameTimes.pop_back();
		}
	}

	void Animation::ClearFrames()
	{
		for (int i = 0; i < images.size(); i++)
			AssetManager::DeleteTexture(images[i].get()->GetPath());
		images.clear();
		frameTimes.clear();
	}

	std::vector<Ref<Texture2D>>& Animation::GetImages()
	{
		return images;
	}

	std::vector<float>& Animation::GetTimes()
	{
		return frameTimes;
	}

	void Animation::SetImages(std::vector<std::string>& paths, std::vector<float>& times)
	{
		images.clear();
		frameTimes.clear();

		for (int i = 0; i < paths.size(); i++)
		{
			//AssetManager::LoadTexture(paths[i]);
			//Ref<Texture2D> frame = Texture2D::Create(paths[i].c_str());
			images.push_back(AssetManager::LoadTexture(paths[i]));
			frameTimes.push_back(times[i]);
			
		}
		//loadedImages = true;
	}

	std::string Animation::GetName()
	{
		return name;
	}

	int Animation::GetCurrentFrame()
	{
		return currentFrame;
	}

	void Animation::SetFrame(int frame)
	{
		currentFrame = frame;
	}

	std::string Animation::GetPath()
	{
		return m_Path;
	}

	std::vector<std::string>& Animation::GetPaths()
	{
		return framePaths;
	}

	void Animation::SetLoadImages(bool set)
	{
		loadedImages = set;
	}

	void Animation::SetCenterPoints(std::vector<glm::vec2>& centerPoints)
	{
		m_CenterPoints = centerPoints;
	}

	glm::vec2 Animation::GetCenterPoint(int index)
	{
		return m_CenterPoints[index];
	}

	std::vector<glm::vec2>& Animation::GetCenterPoints()
	{
		return m_CenterPoints;
	}

}
