#pragma once
#include "Scene.h"

namespace FlatEngine
{
	class SceneManager
	{
	public:
		SceneManager();
		~SceneManager();

		void SaveScene(Scene scene);
		void LoadScene(std::string name);
		Scene &GetLoadedScene();

	private:
		Scene loadedScene;
	};
}
