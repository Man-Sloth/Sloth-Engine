#pragma once

#include "Scene.h"
#include "Sloth/Core/Buffer.h"
#include "Components.h"

namespace Sloth {


	class SceneSerializer
	{
	public:

		SceneSerializer();
		SceneSerializer(SpriteRendererComponent& s);
		SceneSerializer(Ref<Texture2D>& t);

		SceneSerializer(
			const Ref<Scene>& scene,
			std::vector<std::string>& layerNames,
			std::vector<Buffer>& buffers,
			std::vector<std::vector<Entity>>& objects,
			std::vector<std::vector<glm::vec2>>& objectCoords,
			std::vector<glm::vec2>& cellSizes
		);

		

		void Serialize(const std::string& filepath);
		void SerializeRuntime(const std::string& filepath);
		void SerializeImageData();
		void SerializeAnimation(std::vector<std::string> paths, std::vector<float> times, std::filesystem::path savePath);
		void SerializeAnimator(std::vector<std::string> paths, std::vector<Buffer> tags, std::filesystem::path savePath, float speed);

		bool Deserialize(const std::string& filepath);
		bool DeserializeRuntime(const std::string& filepath);
		bool DeserializeImageData();
		bool DeserializeAnimation(const std::string& filepath);
		bool DeserializeAnimator(const std::string& filepath);

		std::vector<std::string> GetLayers();
		std::vector<Buffer> GetBuffers();
		std::vector<std::vector<Entity>> GetObjects();
		std::vector<std::vector<glm::vec2>> GetObjectCoords();
		std::vector<bool> GetVisibleLayers();
		std::vector<glm::vec2> GetCellSizes();
		SpriteRendererComponent GetSpriteRenderer();
		bool GetGridLocked();
		void SetGridLocked(bool locked);

		std::vector<std::string>& GetFramePaths();
		std::vector<float>& GetFrameTimes();

		std::vector <Animation>& GetAnimations();
		Animator GetAnimator();
		std::vector<Buffer>& GetTags();
		float GetSpeed();
		void SetAnimationsLoaded(bool set);
		std::vector<glm::vec2>& GetCenterPoints();

		void ClearAnimator();

	private:
		Ref<Scene> m_Scene;
		std::vector<std::vector<Entity>> m_Objects;
		std::vector<std::vector<glm::vec2>> m_ObjectCoords;
		std::vector<std::string> m_LayerNames;
		std::vector<Buffer> m_Buffers;
		std::vector<bool> m_VisibleLayers;
		std::vector<glm::vec2> m_CellSizes;

		// Animation variables
		std::vector<std::string> m_FramePaths;
		std::vector<float> m_FrameTimes;

		//animator variables
		std::vector<Animation> m_Animations;
		std::vector<Buffer> m_Tags;
		float m_Speed;
		Animator m_Animator;
		std::vector<glm::vec2> m_CenterPoints;

		//glm::vec2 m_CellSize = glm::vec2(1,1);
		SpriteRendererComponent m_Src;
		bool gridLocked = true;
	};

}
