#pragma once
#include "Can/Layers/Layer.h"
#include "Can/Camera/Orthographic_Camera_Controller.h"
#include "Can/Immediate_Renderer/Immediate_Renderer.h" 

namespace Can
{
	enum class Draw_Building_Panel : u8 {
		home = 0,
		general = 1,
		special = 2,
		housing = 3,
		commercial = 4,
		industry = 5,
		hospital = 6,
		police = 7,
		garbage_collection_facility = 8
	};

	class Object;
	class GameScene;
	class Building;
	class Perspective_Camera;
	struct Game_Scene_UI : public Layer::Layer
	{
		Game_Scene_UI() {}

		void OnAttach() override;
		void OnDetach() override;

		bool OnUpdate(TimeStep ts) override;
		void OnEvent(Event::Event& event) override;

		void* font{ nullptr };
		Orthographic_Camera_Controller camera_controller;

		Label_Theme label_theme_title;
		Label_Theme label_theme_button;
		Label_Theme label_theme_camera_mode_button;
		Label_Theme label_theme_left_alinged_small_black_text;
		Label_Theme label_theme_left_alinged_xsmall_black_text;
		Label_Theme label_theme_left_alinged_xsmall_red_text;
		Label_Theme label_theme_text;
		Label_Theme label_theme_large_text;

		Button_Theme button_theme_cross;
		Button_Theme button_theme_camera_mode;
		Button_Theme button_theme_thumb;
		Button_Theme button_theme_track;


		Button_Theme button_theme_back;
		Button_Theme button_theme_construction_modes;
		Button_Theme button_theme_sub_menus;
		Button_Theme button_theme_buildings;

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
		Rect rect_selected_building_detail_panel{
			0, 		// x
			0, 		// y
			1, 		// z
			600, 	// w
			600 	// h
		};
		Rect rect_selected_building_detail_panel_background{
			0, 		// x
			0, 		// y
			0, 		// z
			602, 	// w
			602 	// h
		};


		Object* focus_object{ nullptr };
		Building* selected_building{ nullptr };
		GameScene* game_scene{ nullptr };
		Perspective_Camera* game_scene_camera{ nullptr };

		bool force_update{ false };
		bool draw_building_panel{ false };

		Draw_Building_Panel draw_building_panel_inside_type{ Draw_Building_Panel::home };

		std::vector<Can::Ref<Can::Texture2D>> industry_building_tumbnail_image_files{};

		std::vector<Can::Ref<Can::Texture2D>> hospital_building_tumbnail_image_files{};
		std::vector<Can::Ref<Can::Texture2D>> police_building_tumbnail_image_files{};
		std::vector<Can::Ref<Can::Texture2D>> garbage_collection_facility_building_tumbnail_image_files{};
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