#include "slthpch.h"
#include "SceneSerializer.h"
#include "Sloth/Renderer/AssetManager.h"

#include "Entity.h"

#include <fstream>

#include <yaml-cpp/yaml.h>

namespace YAML {

	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};

}
namespace Sloth {

	

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

	static std::string RigidBody2DBodyTypeToString(Rigidbody2DComponent::BodyType bodyType)
	{
		switch (bodyType)
		{
			case Rigidbody2DComponent::BodyType::Static:    return "Static";
			case Rigidbody2DComponent::BodyType::Dynamic:   return "Dynamic";
			case Rigidbody2DComponent::BodyType::Kinematic: return "Kinematic";
		}

		SLTH_CORE_ASSERT(false, "Unknown body type");
		return {};
	}

	static Rigidbody2DComponent::BodyType RigidBody2DBodyTypeFromString(const std::string& bodyTypeString)
	{
		if (bodyTypeString == "Static")    return Rigidbody2DComponent::BodyType::Static;
		if (bodyTypeString == "Dynamic")   return Rigidbody2DComponent::BodyType::Dynamic;
		if (bodyTypeString == "Kinematic") return Rigidbody2DComponent::BodyType::Kinematic;
	
		SLTH_CORE_ASSERT(false, "Unknown body type");
		return Rigidbody2DComponent::BodyType::Static;
	}

	SceneSerializer::SceneSerializer()
	{}

	SceneSerializer::SceneSerializer(SpriteRendererComponent& s)
		: m_Src(s)
	{
		//SerializeImageMetaData(m_Src);
	}

	SceneSerializer::SceneSerializer(Ref<Texture2D>& t)
	{
		m_Src.Texture = t;
	}

	SceneSerializer::SceneSerializer(
		const Ref <Scene>& scene, 
		std::vector<std::string>& layerNames, 
		std::vector<Buffer> & buffers,
		std::vector<std::vector<Entity>>& objects,
		std::vector<std::vector<glm::vec2>>& objectCoords,
		std::vector<glm::vec2>& cellSizes)
		: m_Scene(scene), m_LayerNames(layerNames), m_Buffers(buffers), m_Objects(objects), m_ObjectCoords(objectCoords), m_CellSizes(cellSizes)
	{
	}

	void SceneSerializer::SerializeImageData()
	{
		std::string filepath = m_Src.Texture.get()->GetPath().c_str();
		filepath.erase(filepath.length() - 4);
		filepath.append(".META");

		YAML::Emitter out;
		out << YAML::BeginMap; // Image data
		out << YAML::Key << "CenterPoint" << YAML::Value << m_Src.CenterPoint;
		out << YAML::Key << "UseCenterPoint" << YAML::Value << m_Src.UseCenterPoint;
		out << YAML::EndMap; // Image data

		std::ofstream fout(filepath);
		fout << out.c_str();
	}

	void SceneSerializer::SerializeAnimation(std::vector<std::string> paths, std::vector<float> times, std::filesystem::path savePath)
	{
		std::filesystem::path pathName = savePath;
		std::string fileName = pathName.filename().string();

		YAML::Emitter out;
		
		out << YAML::BeginMap;
		out << YAML::Key << "AnimationFrames" << YAML::Value << YAML::BeginSeq;
		for (int i = 0; i < paths.size(); i++)
		{
			out << YAML::BeginMap; // Animation Data
			out << YAML::Key << "FramePath" << YAML::Value << paths[i];
			out << YAML::Key << "FrameTime" << YAML::Value << times[i];
			out << YAML::EndMap; // Aniamtion data
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(savePath);
		fout << out.c_str();
		std::cout << "Animation: " + fileName + " was saved!\n";
	}

	void SceneSerializer::SerializeAnimator(std::vector<std::string> paths, std::vector<Buffer> tags, std::filesystem::path savePath, float speed)
	{
		std::filesystem::path pathName = savePath;
		std::string fileName = pathName.filename().string();

		YAML::Emitter out;

		out << YAML::BeginMap;
		out << YAML::Key << "Speed" << YAML::Value << speed;
		//out << YAML::EndMap;

		//out << YAML::BeginMap;
		out << YAML::Key << "Animations" << YAML::Value << YAML::BeginSeq;
		for (int i = 0; i < paths.size(); i++)
		{
			out << YAML::BeginMap; // Animator Data
			out << YAML::Key << "Path" << YAML::Value << paths[i];
			out << YAML::Key << "Tag" << YAML::Value << tags[i].str;
			out << YAML::EndMap; // Animator data
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(savePath);
		fout << out.c_str();
		std::cout << "Animator: " + fileName + " was saved!\n";
	}

	static void SerializeCell(YAML::Emitter& out, glm::vec2 cellSize)
	{
		out << YAML::BeginMap; // Cell Size
		out << YAML::Key << "Cell" << YAML::Value << cellSize;
		out << YAML::EndMap; // Cell Size
	}

	static void SerializeLayers(YAML::Emitter& out, std::string& layerName, Buffer& buffer)
	{
		out << YAML::BeginMap; // Layer
		out << YAML::Key << "Layer" << YAML::Value << layerName;
		out << YAML::Key << "Buffer" << YAML::Value << buffer.str;
		out << YAML::EndMap; // Layer
	}

	static void SerializeEntity(YAML::Emitter& out, Entity entity)
	{
		SLTH_CORE_ASSERT(entity.HasComponent<IDComponent>());

		out << YAML::BeginMap; // Entity
		out << YAML::Key << "Entity" << YAML::Value << entity.GetUUID();

		if (entity.HasComponent<TagComponent>())
		{
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap; // TagComponent

			auto& tag = entity.GetComponent<TagComponent>().Tag;
			out << YAML::Key << "Tag" << YAML::Value << tag;

			out << YAML::EndMap; // TagComponent
		}

		if (entity.HasComponent<TransformComponent>())
		{
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap; // TransformComponent

			auto& tc = entity.GetComponent<TransformComponent>();
			out << YAML::Key << "Translation" << YAML::Value << tc.Translation;
			out << YAML::Key << "Rotation" << YAML::Value << tc.Rotation;
			out << YAML::Key << "Scale" << YAML::Value << tc.Scale;
			out << YAML::Key << "Snap" << YAML::Value << tc.Snap;

			out << YAML::EndMap; // TransformComponent
		}

		if (entity.HasComponent<CameraComponent>())
		{
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap; // CameraComponent

			auto& cameraComponent = entity.GetComponent<CameraComponent>();
			auto& camera = cameraComponent.Camera;

			out << YAML::Key << "Camera" << YAML::Value;
			out << YAML::BeginMap; // Camera
			out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera.GetProjectionType();
			out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.GetPerspectiveVerticalFOV();
			out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.GetPerspectiveNearClip();
			out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.GetPerspectiveFarClip();
			out << YAML::Key << "OrthographicSize" << YAML::Value << camera.GetOrthographicSize();
			out << YAML::Key << "OrthographicNear" << YAML::Value << camera.GetOrthographicNearClip();
			out << YAML::Key << "OrthographicFar" << YAML::Value << camera.GetOrthographicFarClip();
			out << YAML::EndMap; // Camera

			out << YAML::Key << "Primary" << YAML::Value << cameraComponent.Primary;
			out << YAML::Key << "FixedAspectRatio" << YAML::Value << cameraComponent.FixedAspectRatio;

			out << YAML::EndMap; // CameraComponent
		}

		if (entity.HasComponent<SpriteRendererComponent>())
		{
			out << YAML::Key << "SpriteRendererComponent";
			out << YAML::BeginMap; // SpriteRendererComponent

			auto& spriteRendererComponent = entity.GetComponent<SpriteRendererComponent>();
			out << YAML::Key << "Color" << YAML::Value << spriteRendererComponent.Color;
			if (spriteRendererComponent.Texture)
				out << YAML::Key << "TexturePath" << YAML::Value << spriteRendererComponent.Texture->GetPath();

			out << YAML::Key << "TilingFactor" << YAML::Value << spriteRendererComponent.TilingFactor;

			out << YAML::Key << "Layer" << YAML::Value << spriteRendererComponent.Layer;

			out << YAML::Key << "UseCenterPoint" << YAML::Value << spriteRendererComponent.UseCenterPoint;

			out << YAML::EndMap; // SpriteRendererComponent
		}

		if (entity.HasComponent<AnimatorComponent>())
		{
			out << YAML::Key << "AnimatorComponent";
			out << YAML::BeginMap; // AnimatorComponent

			auto& animatorComponent = entity.GetComponent<AnimatorComponent>();

			out << YAML::Key << "AnimatorPath" << YAML::Value << animatorComponent.animator.GetPath().c_str();
			if (animatorComponent.animator.GetPath().empty())
			{
				std::string animations = "";
				for (int i = 0; i < animatorComponent.animator.GetAnimations().size(); i++)
				{
					animations += animatorComponent.animator.GetAnimation(i).GetPath();
					if (i < animatorComponent.animator.GetAnimations().size() - 1)
						animations += ",";
				}
					out << YAML::Key << "AnimationPaths" << YAML::Value << animations;
			}
			out << YAML::EndMap;; // Animator Component
		}

		if (entity.HasComponent<CircleRendererComponent>())
		{
			out << YAML::Key << "CircleRendererComponent";
			out << YAML::BeginMap; // CircleRendererComponent

			auto& circleRendererComponent = entity.GetComponent<CircleRendererComponent>();
			out << YAML::Key << "Color" << YAML::Value << circleRendererComponent.Color;
			out << YAML::Key << "Thickness" << YAML::Value << circleRendererComponent.Thickness;
			out << YAML::Key << "Fade" << YAML::Value << circleRendererComponent.Fade;

			out << YAML::EndMap; // CircleRendererComponent
		}

		if (entity.HasComponent<Rigidbody2DComponent>())
		{
			out << YAML::Key << "Rigidbody2DComponent";
			out << YAML::BeginMap; // Rigidbody2DComponent

			auto& rb2dComponent = entity.GetComponent<Rigidbody2DComponent>();
			out << YAML::Key << "BodyType" << YAML::Value << RigidBody2DBodyTypeToString(rb2dComponent.Type);
			out << YAML::Key << "FixedRotation" << YAML::Value << rb2dComponent.FixedRotation;

			out << YAML::EndMap; // Rigidbody2DComponent
		}

		if (entity.HasComponent<BoxCollider2DComponent>())
		{
			out << YAML::Key << "BoxCollider2DComponent";
			out << YAML::BeginMap; // BoxCollider2DComponent

			auto& bc2dComponent = entity.GetComponent<BoxCollider2DComponent>();
			out << YAML::Key << "Offset" << YAML::Value << bc2dComponent.Offset;
			out << YAML::Key << "Size" << YAML::Value << bc2dComponent.Size;
			out << YAML::Key << "Density" << YAML::Value << bc2dComponent.Density;
			out << YAML::Key << "Friction" << YAML::Value << bc2dComponent.Friction;
			out << YAML::Key << "Restitution" << YAML::Value << bc2dComponent.Restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << bc2dComponent.RestitutionThreshold;

			out << YAML::EndMap; // BoxCollider2DComponent
		}

		if (entity.HasComponent<CircleCollider2DComponent>())
		{
			out << YAML::Key << "CircleCollider2DComponent";
			out << YAML::BeginMap; // CircleCollider2DComponent

			auto& cc2dComponent = entity.GetComponent<CircleCollider2DComponent>();
			out << YAML::Key << "Offset" << YAML::Value << cc2dComponent.Offset;
			out << YAML::Key << "Radius" << YAML::Value << cc2dComponent.Radius;
			out << YAML::Key << "Density" << YAML::Value << cc2dComponent.Density;
			out << YAML::Key << "Friction" << YAML::Value << cc2dComponent.Friction;
			out << YAML::Key << "Restitution" << YAML::Value << cc2dComponent.Restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << cc2dComponent.RestitutionThreshold;

			out << YAML::EndMap; // CircleCollider2DComponent
		}

		out << YAML::EndMap; // Entity
	}

	void SceneSerializer::Serialize(const std::string& filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << "Untitled";

		out << YAML::Key << "CellSize" << YAML::Value << YAML::BeginSeq;
		for (int i = 0; i < m_CellSizes.size(); i++)
		{
			SerializeCell(out, m_CellSizes[i]);
		}
		out << YAML::EndSeq;

		out << YAML::Key << "LockedGrids" << YAML::Value << YAML::BeginSeq;
		out << YAML::BeginMap; // Cell Locked
		out << YAML::Key << "Locked" << YAML::Value << gridLocked;
		out << YAML::EndMap; // Cell Locked
		out << YAML::EndSeq;

		out << YAML::Key << "Layers" << YAML::Value << YAML::BeginSeq;
		for (int i = 0; i < m_LayerNames.size(); i++)
		{
			SerializeLayers(out, m_LayerNames[i], m_Buffers[i]);
		}
		out << YAML::EndSeq;

		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
		m_Scene->m_Registry.each([&](auto entityID)
		{
			Entity entity = { entityID, m_Scene.get() };
			if (!entity)
				return;

			SerializeEntity(out, entity);
		});
		out << YAML::EndSeq;

		out << YAML::EndMap;

		std::ofstream fout(filepath);
		fout << out.c_str();
	}

	void SceneSerializer::SerializeRuntime(const std::string& filepath)
	{
		// Not implemented
		SLTH_CORE_ASSERT(false);
	}

	bool SceneSerializer::Deserialize(const std::string& filepath)
	{
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filepath);
		}
		catch (YAML::ParserException e)
		{
			SLTH_CORE_ERROR("Failed to load .Sloth file '{0}'\n     {1}", filepath, e.what());
			return false;
		}

		if (!data["Scene"])
			return false;

		std::string sceneName = data["Scene"].as<std::string>();
		SLTH_CORE_TRACE("Deserializing scene '{0}'", sceneName);
		m_CellSizes.clear();
		auto cellSize = data["CellSize"];
		if (cellSize)
		{
			for (auto cell : cellSize)
			{
				glm::vec2 cellSize = cell["Cell"].as <glm::vec2>();
				m_CellSizes.push_back(cellSize);
			}
		}

		auto lockedGrid = data["LockedGrids"];
		if (lockedGrid)
		{
			for (auto lock : lockedGrid)
			{
				bool lockedGrid = lock["Locked"].as <bool>();
				gridLocked = lockedGrid;
			}
		}

		auto layers = data["Layers"];
		if (layers)
		{
			m_LayerNames.clear();
			m_Buffers.clear();
			m_VisibleLayers.clear();
			
			for (auto layer : layers)
			{
				std::string name = layer["Layer"].as <std::string>();
				m_LayerNames.push_back(name);
				std::string buffer = layer["Buffer"].as<std::string>();
				Buffer b;
				strcpy(b.str, buffer.c_str());
				m_Buffers.push_back(b);
				m_VisibleLayers.push_back(true);
			}
			m_Objects.resize(m_LayerNames.size());
			m_ObjectCoords.resize(m_LayerNames.size());
		}

		auto entities = data["Entities"];
		if (entities)
		{
			for (auto entity : entities)
			{
				uint64_t uuid = entity["Entity"].as<uint64_t>();

				std::string name;
				auto tagComponent = entity["TagComponent"];
				if (tagComponent)
					name = tagComponent["Tag"].as<std::string>();

				SLTH_CORE_TRACE("Deserialized entity with ID = {0}, name = {1}", uuid, name);

				Entity deserializedEntity = m_Scene->CreateEntityWithUUID(uuid, name);

				Sloth::TransformComponent transComp;
				auto transformComponent = entity["TransformComponent"];
				if (transformComponent)
				{
					// Entities always have transforms
					auto& tc = deserializedEntity.GetComponent<TransformComponent>();
					tc.Translation = transformComponent["Translation"].as<glm::vec3>();
					tc.Rotation = transformComponent["Rotation"].as<glm::vec3>();
					tc.Scale = transformComponent["Scale"].as<glm::vec3>();
					tc.Snap = transformComponent["Snap"].as<bool>();
					transComp = tc;
				}

				auto cameraComponent = entity["CameraComponent"];
				if (cameraComponent)
				{
					auto& cc = deserializedEntity.AddComponent<CameraComponent>();

					auto& cameraProps = cameraComponent["Camera"];
					cc.Camera.SetProjectionType((SceneCamera::ProjectionType)cameraProps["ProjectionType"].as<int>());

					cc.Camera.SetPerspectiveVerticalFOV(cameraProps["PerspectiveFOV"].as<float>());
					cc.Camera.SetPerspectiveNearClip(cameraProps["PerspectiveNear"].as<float>());
					cc.Camera.SetPerspectiveFarClip(cameraProps["PerspectiveFar"].as<float>());

					cc.Camera.SetOrthographicSize(cameraProps["OrthographicSize"].as<float>());
					cc.Camera.SetOrthographicNearClip(cameraProps["OrthographicNear"].as<float>());
					cc.Camera.SetOrthographicFarClip(cameraProps["OrthographicFar"].as<float>());

					cc.Primary = cameraComponent["Primary"].as<bool>();
					cc.FixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
				}

				auto spriteRendererComponent = entity["SpriteRendererComponent"];
				if (spriteRendererComponent)
				{
					auto& src = deserializedEntity.AddComponent<SpriteRendererComponent>();
					src.Color = spriteRendererComponent["Color"].as<glm::vec4>();

					if (spriteRendererComponent["UseCenterPoint"])
						src.UseCenterPoint = spriteRendererComponent["UseCenterPoint"].as<bool>();

					if (spriteRendererComponent["TexturePath"])
					{
						//src.Texture = Texture2D::Create(spriteRendererComponent["TexturePath"].as<std::string>());
						src.Texture = AssetManager::LoadTexture(spriteRendererComponent["TexturePath"].as <std::string>());

						// Load any meta data for image if it exists
						if (src.UseCenterPoint)
						{
							std::string filepath = src.Texture.get()->GetPath().c_str();
							filepath.erase(filepath.length() - 4);
							filepath.append(".META");

							YAML::Node imageData;

							std::ifstream f(filepath.c_str());
							if (f.good())
							{
								std::wstring ws = std::wstring(filepath.begin(), filepath.end());
								const wchar_t* path = ws.c_str();
								const std::filesystem::path p = std::filesystem::path(path);
								std::string filename = p.filename().string();
								std::cout << filename + " found!\n";
								imageData = YAML::LoadFile(filepath);

								if (!imageData["CenterPoint"])
								{
									std::cout << "No Center Point\n";
								}
								else
									src.CenterPoint = imageData["CenterPoint"].as<glm::vec2>();
							}
							else
							{
								//std::cout << "META file doesn't exist.";
								//return false;
							}
						}
					}

					if (spriteRendererComponent["TilingFactor"])
						src.TilingFactor = spriteRendererComponent["TilingFactor"].as<float>();

					if (spriteRendererComponent["Layer"])
						src.Layer = spriteRendererComponent["Layer"].as<int>();

					if (transComp.Snap)
					{
						if (m_ObjectCoords.size() <= src.Layer)
							m_ObjectCoords.resize(src.Layer + 1);

						m_ObjectCoords[src.Layer].push_back(glm::vec2(transComp.Translation.x, transComp.Translation.y));
					}
				}

				auto animatorComponent = entity["AnimatorComponent"];
				if (animatorComponent)
				{
					auto& ac = deserializedEntity.AddComponent<AnimatorComponent>();
					std::string path = animatorComponent["AnimatorPath"].as<std::string>();

					if (!path.empty())
					{
						DeserializeAnimator(path);
						ac.animator = GetAnimator();
						ac.animator.SetPath(path);

						std::filesystem::path pathName = path;
						std::string controllerName = pathName.filename().string();
						ac.animator.SetName(controllerName);
					}
					else
					{
						std::string animPaths = animatorComponent["AnimationPaths"].as <std::string>();
						std::vector<std::string> vPaths;

						// Parce animations in unsaved animator
						std::string str = animPaths;
						std::string delimiter = ",";
						size_t pos = 0;
						std::string token;
						while ((pos = str.find(delimiter)) != std::string::npos)
						{
							token = str.substr(0, pos);
							vPaths.push_back(token);
							str.erase(0, pos + delimiter.length());
						}
						vPaths.push_back(str);

						for (int i = 0; i < vPaths.size();i++)
						{
							ac.animator.AddAnimation(Animation(vPaths[i]));
						}
					}
					ClearAnimator();
				}

				auto circleRendererComponent = entity["CircleRendererComponent"];
				if (circleRendererComponent)
				{
					auto& crc = deserializedEntity.AddComponent<CircleRendererComponent>();
					crc.Color = circleRendererComponent["Color"].as<glm::vec4>();
					crc.Thickness = circleRendererComponent["Thickness"].as<float>();
					crc.Fade = circleRendererComponent["Fade"].as<float>();
				}

				auto rigidbody2DComponent = entity["Rigidbody2DComponent"];
				if (rigidbody2DComponent)
				{
					auto& rb2d = deserializedEntity.AddComponent<Rigidbody2DComponent>();
					rb2d.Type = RigidBody2DBodyTypeFromString(rigidbody2DComponent["BodyType"].as<std::string>());
					rb2d.FixedRotation = rigidbody2DComponent["FixedRotation"].as<bool>();
				}

				auto boxCollider2DComponent = entity["BoxCollider2DComponent"];
				if (boxCollider2DComponent)
				{
					auto& bc2d = deserializedEntity.AddComponent<BoxCollider2DComponent>();
					bc2d.Offset = boxCollider2DComponent["Offset"].as<glm::vec2>();
					bc2d.Size = boxCollider2DComponent["Size"].as<glm::vec2>();
					bc2d.Density = boxCollider2DComponent["Density"].as<float>();
					bc2d.Friction = boxCollider2DComponent["Friction"].as<float>();
					bc2d.Restitution = boxCollider2DComponent["Restitution"].as<float>();
					bc2d.RestitutionThreshold = boxCollider2DComponent["RestitutionThreshold"].as<float>();
				}

				auto circleCollider2DComponent = entity["CircleCollider2DComponent"];
				if (circleCollider2DComponent)
				{
					auto& cc2d = deserializedEntity.AddComponent<CircleCollider2DComponent>();
					cc2d.Offset = circleCollider2DComponent["Offset"].as<glm::vec2>();
					cc2d.Radius = circleCollider2DComponent["Radius"].as<float>();
					cc2d.Density = circleCollider2DComponent["Density"].as<float>();
					cc2d.Friction = circleCollider2DComponent["Friction"].as<float>();
					cc2d.Restitution = circleCollider2DComponent["Restitution"].as<float>();
					cc2d.RestitutionThreshold = circleCollider2DComponent["RestitutionThreshold"].as<float>();
				}

				if (deserializedEntity.HasComponent<SpriteRendererComponent>())
				{
					int layer = deserializedEntity.GetComponent<SpriteRendererComponent>().Layer;
					if (m_Objects.size() <= layer)
						m_Objects.resize(layer + 1);
					m_Objects[layer].push_back(deserializedEntity);
				}
			}
		}

		return true;
	}

	bool SceneSerializer::DeserializeRuntime(const std::string& filepath)
	{
		// Not implemented
		SLTH_CORE_ASSERT(false);
		return false;
	}

	bool SceneSerializer::DeserializeImageData()
	{
		std::string filepath = m_Src.Texture.get()->GetPath().c_str();
		filepath.erase(filepath.length() - 4);
		filepath.append(".META");

		YAML::Node imageData;

		std::ifstream f(filepath.c_str());
		if (f.good())
		{
			std::wstring ws = std::wstring(filepath.begin(), filepath.end());
			const wchar_t* path = ws.c_str();
			const std::filesystem::path p = std::filesystem::path(path);
			std::string filename = p.filename().string();
			//std::cout << filename + " found!\n";
			imageData = YAML::LoadFile(filepath);
		}
		else
		{
			//std::cout << "META file doesn't exist.";
			return false;
		}

		if (!imageData["CenterPoint"])
		{
			//std::cout << "No Center Point\n";
		}
		else
			m_Src.CenterPoint = imageData["CenterPoint"].as<glm::vec2>();

		return true;
	}

	bool SceneSerializer::DeserializeAnimation(const std::string& filePath)
	{
		YAML::Node data;
	
		std::string filepath = filePath;
		std::filesystem::path filePathName = filePath;
		std::string fileName = filePathName.filename().string();

		try
		{
			data = YAML::LoadFile(filepath);
		}
		catch (YAML::ParserException e)
		{
			SLTH_CORE_ERROR("Failed to load .ANIM file '{0}'\n     {1}", filepath, e.what());
			return false;
		}

		auto animationFrames = data["AnimationFrames"];
		if (animationFrames)
		{
			m_FramePaths.clear();
			m_FrameTimes.clear();

			for (auto frame : animationFrames)
			{
				std::string path = frame["FramePath"].as<std::string>();
				m_FramePaths.push_back(path);
				float time = frame["FrameTime"].as<float>();
				m_FrameTimes.push_back(time);

				filepath = path;
				filepath.erase(filepath.length() - 4);
				filepath.append(".META");

				YAML::Node imageData;

				std::ifstream f(filepath.c_str());
				if (f.good())
				{
					std::wstring ws = std::wstring(filepath.begin(), filepath.end());
					const wchar_t* path = ws.c_str();
					const std::filesystem::path p = std::filesystem::path(path);
					std::string filename = p.filename().string();
					//std::cout << filename + " found!\n";
					imageData = YAML::LoadFile(filepath);
				}
				else
				{
					//std::cout << "META file doesn't exist.";
					m_CenterPoints.push_back(glm::vec2(0, 0));
				}

				if (!imageData["CenterPoint"])
				{
					//std::cout << "No Center Point\n";
					//m_CellSizes.push_back(glm::vec2(0, 0));
				}
				else
					m_CenterPoints.push_back(imageData["CenterPoint"].as<glm::vec2>());
			}
		}
		std::cout << "Animation: " + fileName + " was Loaded!\n";
		return true;
	}

	bool SceneSerializer::DeserializeAnimator(const std::string& filePath)
	{
		YAML::Node data;

		std::string filepath = filePath;
		std::filesystem::path filePathName = filePath;
		std::string fileName = filePathName.filename().string();

		try
		{
			data = YAML::LoadFile(filepath);
		}
		catch (YAML::ParserException e)
		{
			SLTH_CORE_ERROR("Failed to load .ANMTR file '{0}'\n     {1}", filepath, e.what());
			return false;
		}

		m_Speed = data["Speed"].as<float>();

		auto animations = data["Animations"];
		if (animations)
		{
			for (auto a : animations)
			{
				std::string path = a["Path"].as<std::string>();
				Animation animation = Animation(path);
				m_Animations.push_back(animation);

				std::string tag = a["Tag"].as<std::string>();
				Buffer tagBuffer; 
				strcpy(tagBuffer.str, tag.c_str());
				m_Tags.push_back(tagBuffer);
			}
			m_Animator.SetAnimations(m_Animations);
		}
		
		std::cout << "Animator: " + fileName + " was Loaded!\n";
		return true;
	}


	std::vector<std::string> SceneSerializer::GetLayers()
	{
		return m_LayerNames;
	}

	std::vector<Buffer> SceneSerializer::GetBuffers()
	{
		return m_Buffers;
	}

	std::vector<std::vector<Entity>> SceneSerializer::GetObjects()
	{
		return m_Objects;
	}

	std::vector<std::vector<glm::vec2>> SceneSerializer::GetObjectCoords()
	{
		return m_ObjectCoords;
	}

	std::vector<glm::vec2> SceneSerializer::GetCellSizes()
	{
		return m_CellSizes;
	}

	std::vector<bool> SceneSerializer::GetVisibleLayers()
	{
		return m_VisibleLayers;
	}

	SpriteRendererComponent SceneSerializer::GetSpriteRenderer()
	{
		return m_Src;
	}

	bool SceneSerializer::GetGridLocked()
	{
		return gridLocked;
	}

	void SceneSerializer::SetGridLocked(bool locked)
	{
		gridLocked = locked;
	}

	std::vector<std::string>& SceneSerializer::GetFramePaths()
	{
		return m_FramePaths;
	}

	std::vector<float>& SceneSerializer::GetFrameTimes()
	{
		return m_FrameTimes;
	}

	std::vector <Animation>& SceneSerializer::GetAnimations()
	{
		return m_Animations;
	}

	Animator SceneSerializer::GetAnimator()
	{
		return m_Animator;
	}

	std::vector<Buffer>& SceneSerializer::GetTags()
	{
		return m_Tags;
	}

	float SceneSerializer::GetSpeed()
	{
		return m_Speed;
	}

	void SceneSerializer::SetAnimationsLoaded(bool set)
	{
	}

	std::vector<glm::vec2>& SceneSerializer::GetCenterPoints()
	{
		return m_CenterPoints;
	}

	void SceneSerializer::ClearAnimator()
	{
		m_Animations.clear();
	}
}
