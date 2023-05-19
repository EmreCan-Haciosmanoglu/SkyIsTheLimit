#include "canpch.h"
#include "Game_Scene_UI.h"

#include "Can/EntryPoint.h"
#include "Can/Renderer/RenderCommand.h"
#include "Can/Renderer/Renderer2D.h"


#include "Can/Font/FontFlags.h"

namespace Can
{
	void Game_Scene_UI::OnAttach()
	{
		on_game_scene_ui_layer_attach(*this);
	}
	void Game_Scene_UI::OnDetach()
	{
		on_game_scene_ui_layer_detach(*this);
	}

	bool Game_Scene_UI::OnUpdate(TimeStep ts)
	{
		return on_game_scene_ui_layer_update(*this, ts);
	}
	void Game_Scene_UI::OnEvent(Event::Event& event)
	{
		Event::EventDispatcher dispatcher(event);
		dispatcher.dispatch<Event::KeyReleasedEvent>(this, CAN_BIND_EVENT_FN(on_game_scene_ui_layer_key_released));
		dispatcher.dispatch<Event::KeyTypedEvent>(this, CAN_BIND_EVENT_FN(on_game_scene_ui_layer_key_typed));

		dispatcher.dispatch<Event::MouseButtonPressedEvent>(this, CAN_BIND_EVENT_FN(on_game_scene_ui_layer_mouse_pressed));
		dispatcher.dispatch<Event::MouseMovedEvent>(this, CAN_BIND_EVENT_FN(on_game_scene_ui_layer_mouse_moved));
	}

	extern Buffer_Data buffer_data;
	void init_game_scene_ui_layer(Game_Scene_UI& ui)
	{
		init_orthographic_camera_controller(ui.camera_controller, 16.0f / 9.0f, 10.0f, false);
		auto ptr = &ui.camera_controller;
		buffer_data.camera_controller = ptr;
		ui.font = load_font("assets/fonts/DancingScript/DancingScript-Regular.ttf");


	}
	void deinit_game_scene_ui_layer(Game_Scene_UI& ui)
	{}

	void on_game_scene_ui_layer_attach(Game_Scene_UI& ui)
	{}
	void on_game_scene_ui_layer_detach(Game_Scene_UI& ui)
	{}

	bool on_game_scene_ui_layer_update(Game_Scene_UI& ui, TimeStep ts)
	{
		set_camera_for_immediate_renderer();

		draw_screen(ui);

		immediate_flush();
		return ui.force_update;
		return false;
	}

	bool on_game_scene_ui_layer_key_released(void* p, Event::KeyReleasedEvent& event)
	{
		Game_Scene_UI& ui = *((Game_Scene_UI*)p);
		return false;
	}
	bool on_game_scene_ui_layer_mouse_pressed(void* p, Event::MouseButtonPressedEvent& event)
	{
		Game_Scene_UI& ui = *((Game_Scene_UI*)p);
		return false;
	}
	bool on_game_scene_ui_layer_mouse_moved(void* p, Event::MouseMovedEvent& event)
	{
		Game_Scene_UI& ui = *((Game_Scene_UI*)p);
		return false;
	}
	bool on_game_scene_ui_layer_key_typed(void* p, Event::KeyTypedEvent& event)
	{
		return false;
	}

	void draw_screen(Game_Scene_UI& ui)
	{
	}
}