#pragma once
#include "Can/Layers/Layer.h"
#include "Can/Camera/Orthographic_Camera_Controller.h"
#include "Can/Immediate_Renderer/Immediate_Renderer.h" 

namespace Can
{

	struct Game_Scene_UI : public Layer::Layer
	{
		Game_Scene_UI() {}

		void OnAttach() override;
		void OnDetach() override;

		bool OnUpdate(TimeStep ts) override;
		void OnEvent(Event::Event& event) override;

		void* font = nullptr;
		Orthographic_Camera_Controller camera_controller;
		bool force_update = false;
	};

	void init_game_scene_ui_layer(Game_Scene_UI& ui);
	void deinit_game_scene_ui_layer(Game_Scene_UI& ui);

	void on_game_scene_ui_layer_attach(Game_Scene_UI& ui);
	void on_game_scene_ui_layer_detach(Game_Scene_UI& ui);

	bool on_game_scene_ui_layer_update(Game_Scene_UI& ui, TimeStep ts);
	bool on_game_scene_ui_layer_key_released(void* p, Event::KeyReleasedEvent& event);
	bool on_game_scene_ui_layer_key_typed(void* p, Event::KeyTypedEvent& event);

	bool on_game_scene_ui_layer_mouse_pressed(void* p, Event::MouseButtonPressedEvent& event);
	bool on_game_scene_ui_layer_mouse_moved(void* p, Event::MouseMovedEvent& event);

	void draw_screen(Game_Scene_UI& ui);
}