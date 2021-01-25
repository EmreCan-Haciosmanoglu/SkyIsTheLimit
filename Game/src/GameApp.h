#pragma once
#include "Can.h"

#include "Scenes/GameScene.h"
#include "UIScene.h"
#include "Debug.h"

namespace Can
{
	class GameApp : public Can::Application
	{
	public:
		GameApp();
		~GameApp();

	private:
		std::vector<Prefab*> LoadPrefabs(const std::string& folder, const std::string& filter);

		void LoadRoads();
		void LoadBuildings();
		void LoadTrees();

	public:
		Prefab* terrainPrefab;
		Ref<Texture2D> treeMap;

		std::vector<std::array<Prefab*, 3>> roads;
		// 0 => Road
		// 1 => Junction
		// 2 => End

		std::vector<Prefab*> buildings;
		std::vector<Prefab*> trees;
		
		GameScene* gameScene = nullptr;
		UIScene* uiScene = nullptr;
		Debug* debugScene = nullptr;
	};
}