#pragma once
#include "Can.h"

#include "TestScene.h"
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
		void LoadHouses();

	public:
		Prefab* terrainPrefab;

		std::vector<std::array<Prefab*, 3>> roads;
		// 0 => Road
		// 1 => Junction
		// 2 => End

		std::vector<Prefab*> buildings;
		std::vector<Prefab*> houses;
		
		TestScene* testScene = nullptr; 
		UIScene* uiScene = nullptr;
		Debug* debugScene = nullptr;
	};
}