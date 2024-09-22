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

		Ref<Texture2D> tree_map;
		Ref<Texture2D> add_texture;					// Put better name
		Ref<Texture2D> save_texture;				// Put better name
		Ref<Texture2D> pause_texture;				// Put better name
		Ref<Texture2D> remove_texture;				// Put better name
		Ref<Texture2D> cancel_texture;				// Put better name
		Ref<Texture2D> change_texture;				// Put better name
		Ref<Texture2D> upgrade_texture;				// Put better name
		Ref<Texture2D> straight_texture;			// Put better name
		Ref<Texture2D> quadratic_texture;			// Put better name
		Ref<Texture2D> downgrade_texture;			// Put better name
		Ref<Texture2D> cubic_1234_texture;			// Put better name
		Ref<Texture2D> cubic_1243_texture;			// Put better name
		Ref<Texture2D> cubic_1342_texture;			// Put better name
		Ref<Texture2D> cubic_1432_texture;			// Put better name
		Ref<Texture2D> normal_speed_texture;		// Put better name
		Ref<Texture2D> two_times_speed_texture;		// Put better name
		Ref<Texture2D> four_times_speed_texture;	// Put better name

		Ref<Texture2D> houses_texture;				// Put better name
		Ref<Texture2D> residentials_texture;		// Put better name
		Ref<Texture2D> industrials_texture;			// Put better name
		Ref<Texture2D> offices_texture;				// Put better name
		Ref<Texture2D> hospitals_texture;			// Put better name
		Ref<Texture2D> police_stations_texture;		// Put better name
		Ref<Texture2D> gcf_texture;					// Put better name

		Ref<Texture2D> garbage_filled_icon;			// Put better name

		std::vector<struct Road_Type> road_types;
		std::vector<struct Vehicle_Type> vehicle_types;
		std::vector<u64> personal_vehicles{};
		std::vector<u64> commercial_vehicles{};
		std::vector<u64> industrial_vehicles{};
		std::vector<u64> ambulances{};
		std::vector<u64> police_cars{};
		std::vector<u64> garbage_trucks{};
		std::vector<struct Building_Type> building_types;

		std::vector<Prefab*> trees;
		std::vector<Prefab*> people;

		Main_Menu main_menu{};

		GameScene* gameScene{ nullptr };
		UIScene* uiScene{ nullptr };
		//Debug* debugScene = nullptr;

		Perspective_Camera_Controller perspective_camera_controller;

		static GameApp* instance;
	};
}