#include "slthpch.h"
#include "Entity.h"

namespace Sloth {

	Entity::Entity(entt::entity handle, Scene* scene)
		: m_EntityHandle(handle), m_Scene(scene)
	{
	}

}
