#include "slthpch.h"
#include "Sloth/Renderer/AssetManager.h"

namespace Sloth {

	Ref<Texture2D> AssetManager::LoadTexture(std::string path)
	{
		if (m_LoadedTextures.find(path) == m_LoadedTextures.end()) {
			// not found
			Ref<Texture2D> texture = Texture2D::Create(path);
			m_LoadedTextures.insert({ path, texture });
			m_AmountUsed.insert({ path, 1 });
		}
		else {
			// found
			m_AmountUsed[path] += 1;
		}
		return m_LoadedTextures[path];
	}

	void AssetManager::DeleteTexture(std::string path)
	{
		if (m_LoadedTextures.find(path) == m_LoadedTextures.end()) 
		{
			// not found
		}
		else 
		{
			// found

			if (m_AmountUsed[path] == 1)
			{
				m_LoadedTextures.erase(path);
				m_AmountUsed.erase(path);
			}
			else if ( m_AmountUsed[path] > 1)
			{
				m_AmountUsed[path] -= 1;
			}
			else
			{
				m_LoadedTextures.clear();
				m_AmountUsed.clear();
			}
		}
	}

	int AssetManager::GetTextureCount()
	{
		return m_LoadedTextures.size();
	}

	void AssetManager::ResetTextureCount()
	{
		m_AmountUsed.clear();
		m_LoadedTextures.clear();
	}
}
