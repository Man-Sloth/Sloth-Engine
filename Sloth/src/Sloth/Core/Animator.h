#pragma once
#include "Sloth/Renderer/Texture.h"
#include "Animation.h"
#include "Sloth/Core/Buffer.h"

namespace Sloth {

	class Animator
	{
	public:

		Animator();
		~Animator();
		void Update();
		void AddAnimation(Animation animation);
		Animation& GetAnimation(int index);
		void SetAnimation(int index);
		std::vector<Animation>& GetAnimations();
		void DeleteAnimation(int index);
		void DeleteAnimation(std::string tag);
		void ClearAnimations();
		int GetSize();
		void AddTag(Buffer tag);
		Buffer& GetTag(int index);
		std::vector<Buffer>& GetTags();
		Animation& GetCurrentAnimation();
		int GetAnimationIndex();
		void SetFrame(int Frame);
		void ReloadAnimation(int index, Animation animation);
		void SetAnimations(std::vector <Animation>& animations);
		void SetTags(std::vector<Buffer>& tags);
		void SetSpeed(float speed);
		void SetName(std::string name);
		std::string GetName();
		std::string GetPath();
		void SetPath(std::string path);

	private:
		
		std::vector<Buffer> m_Tags;
		std::vector<Animation> m_Animations;
		std::string m_Name = "";
		std::string m_Path = "";
		int m_CurrentAnimation = 0;
		int m_CurrentFrame = 0;
		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();;
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		float elapsed = 0.0f;
		float m_SpeedMultiplier = 1.0f;
		
	};
}
