#pragma once
#include "Can/Layers/Layer.h"
#include "Can/Camera/Orthographic_Camera_Controller.h"
#include "Can/Immediate_Renderer/Immediate_Renderer.h" 

namespace Can
{
	constexpr u8 draw_building_panel_home{ 0 };

	constexpr u8 draw_building_panel_general{ 1 };
	constexpr u8 draw_building_panel_special{ 2 };

	constexpr u8 draw_building_panel_housing{ 3 };
	constexpr u8 draw_building_panel_commercial{ 4 };
	constexpr u8 draw_building_panel_industry{ 5 };

	constexpr u8 draw_building_panel_hospital{ 6 };
	constexpr u8 draw_building_panel_police{ 7 };
	constexpr u8 draw_building_panel_fire_station{ 8 };

	class Object;
	class GameScene;
	class Perspective_Camera;
	struct Game_Scene_UI : public Layer::Layer
	{
		Game_Scene_UI() {}

		void OnAttach() override;
		void OnDetach() override;

		bool OnUpdate(TimeStep ts) override;
		void OnEvent(Event::Event& event) override;

		void* font = nullptr;
		Orthographic_Camera_Controller camera_controller;

		Label_Theme label_theme_button;
		Label_Theme label_theme_camera_mode_button;
		Label_Theme label_theme_text;
		Label_Theme label_theme_large_text;

		Button_Theme button_theme_cross;
		Button_Theme button_theme_camera_mode;
		Button_Theme button_theme_thumb;
		Button_Theme button_theme_track;

		Button_Theme button_theme_back;

		Button_Theme button_theme_general_buildings;
		Button_Theme button_theme_special_buildings;

		Button_Theme button_theme_housing_buildings;
		Button_Theme button_theme_commercial_buildings;
		Button_Theme button_theme_industry_buildings;

		Button_Theme button_theme_hospital_buildings;
		Button_Theme button_theme_police_buildings;
		Button_Theme button_theme_fire_station_buildings;

		Slider_Theme slider_theme_horizontal;
		Slider_Theme slider_theme_vertical;

		Sub_Region_Theme sub_region_theme_details;
		Rect rect_sub_region{
			0, 		// x
			0, 		// y
			0, 		// z
			200, 	// w
			100 	// h
		};


		Object* focus_object = nullptr;
		GameScene* game_scene = nullptr;
		Perspective_Camera* game_scene_camera = nullptr;

		bool force_update = false;
		bool draw_building_panel = false;

		u8 draw_building_panel_inside_type = draw_building_panel_home;

		std::vector<Can::Ref<Can::Texture2D>> housing_building_tumbnail_image_files{};
		std::vector<Can::Ref<Can::Texture2D>> commercial_building_tumbnail_image_files{};
		std::vector<Can::Ref<Can::Texture2D>> industry_building_tumbnail_image_files{};

		std::vector<Can::Ref<Can::Texture2D>> hospital_building_tumbnail_image_files{};
		std::vector<Can::Ref<Can::Texture2D>> police_building_tumbnail_image_files{};
		std::vector<Can::Ref<Can::Texture2D>> fire_station_building_tumbnail_image_files{};
	};

	void init_game_scene_ui_layer(Game_Scene_UI& ui, GameScene& game_scene);
	void deinit_game_scene_ui_layer(Game_Scene_UI& ui);

	void on_game_scene_ui_layer_attach(Game_Scene_UI& ui);
	void on_game_scene_ui_layer_detach(Game_Scene_UI& ui);

	bool on_game_scene_ui_layer_update(Game_Scene_UI& ui, TimeStep ts);
	bool on_game_scene_ui_layer_key_released(void* p, Event::KeyReleasedEvent& event);
	bool on_game_scene_ui_layer_key_typed(void* p, Event::KeyTypedEvent& event);

	bool on_game_scene_ui_layer_mouse_pressed(void* p, Event::MouseButtonPressedEvent& event);
	bool on_game_scene_ui_layer_mouse_moved(void* p, Event::MouseMovedEvent& event);

	void update_screen(Game_Scene_UI& ui);
	void draw_screen(Game_Scene_UI& ui);


	bool on_game_scene_ui_layer_mouse_released(void* p, Event::MouseButtonReleasedEvent& event);
}