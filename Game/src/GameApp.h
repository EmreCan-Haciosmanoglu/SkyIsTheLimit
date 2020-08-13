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
		std::vector<Prefab*> LoadRoadPrefabs();
		std::vector<Prefab*> LoadJunctionPrefabs();
		std::vector<Prefab*> LoadEndPrefabs();

		void CombinePrefabs();

	public:
		Prefab* terrainPrefab;

		std::vector<std::array<Prefab*, 3>> roads;
		std::vector<Prefab*> buildings;
		// 0 => Road
		// 1 => Junction
		// 2 => End
		
		TestScene* testScene;
		UIScene* uiScene;
		Debug* debugScene;
	};
}