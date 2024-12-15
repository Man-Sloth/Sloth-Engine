#define OEMRESOURCE

#include "slthpch.h"
#include "Animator.h"
#include "Sloth.h"

namespace Sloth {

	Animator::Animator()
	{}

	Animator::~Animator() {}

	void Animator::Update()
	{
		m_Animations[m_CurrentAnimation].Update(m_SpeedMultiplier);
	}

	void Animator::AddAnimation(Animation animation)
	{
		m_Animations.push_back(animation);
	}

	Animation& Animator::GetAnimation(int index)
	{
		if (m_Animations.size() > 0)
			return m_Animations[index];
		else
			return Animation();
	}

	void Animator::SetAnimation(int index)
	{
		m_CurrentAnimation = index;
		m_CurrentFrame = 0;
	}

	std::vector<Animation>& Animator::GetAnimations()
	{
		return m_Animations;
	}

	void Animator::DeleteAnimation(int index)
	{
		if (GetSize() > 1)
		{
			m_Animations.erase(m_Animations.begin() + index);
			m_Tags.erase(m_Tags.begin() + index);
		}
		else
		{
			m_Animations.pop_back();
			m_Tags.pop_back();
		}
		if (m_CurrentAnimation >= m_Animations.size())
			m_CurrentAnimation = m_Animations.size() - 1;
		if (m_CurrentAnimation < 0)
			m_CurrentAnimation = 0;
		m_CurrentFrame = 0;
	}

	void Animator::DeleteAnimation(std::string tag)
	{
		//todo
	}

	void Animator::ClearAnimations()
	{
		m_Animations.clear();
		m_Tags.clear();
	}

	int Animator::GetSize()
	{
		return m_Animations.size();
	}

	void Animator::AddTag(Buffer tag)
	{
		m_Tags.push_back(tag);
	}

	Buffer& Animator::GetTag(int index)
	{
		return m_Tags[index];
	}

	std::vector<Buffer>& Animator::GetTags()
	{
		return m_Tags;
	}

	Animation& Animator::GetCurrentAnimation()
	{
		return m_Animations[m_CurrentAnimation];
	}

	int Animator::GetAnimationIndex()
	{
		return m_CurrentAnimation;
	}

	void Animator::SetFrame(int frame)
	{
		m_CurrentFrame = frame;
	}

	void Animator::ReloadAnimation(int index, Animation animation)
	{
		m_Animations[index] = animation;
		m_CurrentFrame = 0;
	}

	void Animator::SetAnimations(std::vector<Animation>& animations)
	{
		m_Animations = animations;
	}

	void Animator::SetTags(std::vector<Buffer>& tags)
	{
		m_Tags = tags;
	}

	void Animator::SetSpeed(float speed)
	{
		m_SpeedMultiplier = speed;
	}

	void Animator::SetName(std::string name)
	{
		m_Name = name;
	}

	std::string Animator::GetName()
	{
		return m_Name;
	}

	std::string Animator::GetPath()
	{
		return m_Path;
	}

	void Animator::SetPath(std::string path)
	{
		m_Path = path;
	}
}
