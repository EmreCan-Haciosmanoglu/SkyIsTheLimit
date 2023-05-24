#pragma once
#include "Can/Layers/Layer.h"
#include "Can/Camera/Orthographic_Camera_Controller.h"
#include "Can/Immediate_Renderer/Immediate_Renderer.h" 

namespace Can
{

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
		Label_Theme label_theme_text;

		Button_Theme button_theme_cross;
		Button_Theme button_theme_thumb;
		Button_Theme button_theme_track;

		Slider_Theme slider_theme_horizontal;
		Slider_Theme slider_theme_vertical;

		Sub_Region_Theme sub_region_theme_details;

		u32 menu_x = 0;
		u32 menu_y = 0;

		Object* focus_object = nullptr;
		Perspective_Camera* game_scene_camera = nullptr;

		bool force_update = false;
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
}