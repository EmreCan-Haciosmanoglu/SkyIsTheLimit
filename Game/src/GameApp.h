#pragma once
#include "Can.h"

#include "TestScene.h"
#include "UIScene.h"

namespace Can
{
	class GameApp : public Can::Application
	{
	public:
		GameApp();
		~GameApp();

		void LoadRoads();
		void LoadJunctions();
		void LoadEnds();

	public:
		std::vector<Prefab*> roads;
		std::vector<Prefab*> junctions;
		std::vector<Prefab*> ends;

		Prefab* terrainPrefab;

	private:
		TestScene* testScene;
		UIScene* uiScene;
	};
}