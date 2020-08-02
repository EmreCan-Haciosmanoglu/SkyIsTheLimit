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
		std::vector<Ref<Prefab>> roads;
		std::vector<Ref<Prefab>> junctions;
		std::vector<Ref<Prefab>> ends;

		Ref<Prefab> terrainPrefab;

	private:
		TestScene* testScene;
		UIScene* uiScene;
	};
}