#pragma once
#include "Can.h"

#include "Scenes/GameScene.h"
#include "UIScene.h"
//#include "Debug.h"

#include "Scenes/Main_Menu/Main_Menu.h"

namespace Can
{
	class GameApp : public Can::Application
	{
	public:
		GameApp(const Can::WindowProps& props);
		void start_the_game(std::string& save_name, bool is_old_game = false);
		~GameApp();

	private:
		std::vector<Prefab*> LoadPrefabs(const std::string& folder, const std::string& filter);

		void LoadTrees();
		void LoadPeople();

	public:
		Prefab* terrainPrefab;
		Ref<Texture2D> terrainTexture;

		Ref<Texture2D> treeMap;
		Ref<Texture2D> addTexture;				// Put better name
		Ref<Texture2D> saveTexture;				// Put better name
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

		std::vector<struct Road_Type> road_types;
		std::vector<struct Vehicle_Type> vehicle_types;
		std::vector<struct Building_Type> building_types;

		std::vector<Prefab*> buildings;
		std::vector<Prefab*> trees;
		std::vector<Prefab*> people;

		Main_Menu main_menu{};

		GameScene* gameScene = nullptr;
		UIScene* uiScene = nullptr;
		//Debug* debugScene = nullptr;

		Perspective_Camera_Controller perspective_camera_controller;

		static GameApp* instance;
	};
}