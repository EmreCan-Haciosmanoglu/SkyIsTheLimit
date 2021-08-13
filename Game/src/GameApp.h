#pragma once
#include "Can.h"

#include "Scenes/GameScene.h"
#include "UIScene.h"
#include "Debug.h"

#include "Types/RoadType.h"

#include "Scenes/Main_Menu/Main_Menu.h"

namespace Can
{
	class GameApp : public Can::Application
	{
	public:
		GameApp(const Can::WindowProps& props);
		~GameApp();

	private:
		std::vector<Prefab*> LoadPrefabs(const std::string& folder, const std::string& filter);

		void LoadRoadTypes();
		void LoadBuildings();
		void LoadTrees();
		void LoadCars();

	public:
		Prefab* terrainPrefab;
		Ref<Texture2D> terrainTexture;

		Ref<Texture2D> treeMap;
		
		Ref<Texture2D> addTexture;				// Put better name
		Ref<Texture2D> pauseTexture;			// Put better name
		Ref<Texture2D> removeTexture;			// Put better name
		Ref<Texture2D> cancelTexture;			// Put better name
		Ref<Texture2D> changeTexture;			// Put better name
		Ref<Texture2D> upgradeTexture;			// Put better name
		Ref<Texture2D> straightTexture;			// Put better name
		Ref<Texture2D> quadraticTexture;		// Put better name
		Ref<Texture2D> downgradeTexture;		// Put better name
		Ref<Texture2D> cubic1234Texture;		// Put better name
		Ref<Texture2D> cubic1243Texture;		// Put better name
		Ref<Texture2D> cubic1342Texture;		// Put better name
		Ref<Texture2D> cubic1432Texture;		// Put better name
		Ref<Texture2D> normalSpeedTexture;		// Put better name
		Ref<Texture2D> twoTimesSpeedTexture;	// Put better name
		Ref<Texture2D> fourTimesSpeedTexture;	// Put better name


		std::vector<RoadType> road_types;

		std::vector<Prefab*> buildings;
		std::vector<Prefab*> trees;
		std::vector<Prefab*> cars;

		Main_Menu main_menu{};
		
		GameScene* gameScene = nullptr;
		UIScene* uiScene = nullptr;
		Debug* debugScene = nullptr;
	};
}