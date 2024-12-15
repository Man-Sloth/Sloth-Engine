#pragma once
#include "Sloth/Renderer/Texture.h"
#include <map>

namespace Sloth {

	class AssetManager
	{
	public:
		AssetManager() = default;

		static Ref<Texture2D> LoadTexture(std::string path);
		static void DeleteTexture(std::string path);
		static int GetTextureCount();
		static void ResetTextureCount();

	private:
		static std::map<std::string, Ref<Texture2D>> m_LoadedTextures;
		static std::map<std::string, int> m_AmountUsed;
	};
}
