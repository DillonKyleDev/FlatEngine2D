#pragma once
#include <vector>
#include "GameObject.h"


namespace FlatEngine
{
	class Scene
	{
	public:
		Scene();
		~Scene();

		void SetName(std::string name);
		std::string GetName();
		void AddSceneObject(GameObject sceneObject);
		std::vector<GameObject> GetSceneObjects();
		// Managing GameObjects
		GameObject CreateGameObject();
		void DeleteGameObject(GameObject gameObject);
		void IncrementID();
		std::vector<GameObject>& GetGameObjects();
		long GetCurrentID();

	private:
		std::string name;
		std::vector<GameObject> sceneObjects;
		long currentID;
	};
}