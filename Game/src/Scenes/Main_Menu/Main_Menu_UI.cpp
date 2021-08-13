#include "canpch.h"
#include "Main_Menu_UI.h"

#include "Can/Renderer/RenderCommand.h"
#include "Can/Renderer/Renderer2D.h"

#include "Can/Immediate_Renderer/Immediate_Renderer.h"

namespace Can
{
	void Main_Menu_UI::OnAttach()
	{
		on_main_menu_ui_layer_attach(*this);
	}
	void Main_Menu_UI::OnDetach()
	{
		on_main_menu_ui_layer_detach(*this);
	}
	void Main_Menu_UI::OnUpdate(TimeStep ts)
	{
		on_main_menu_ui_layer_update(*this, ts);
	}
	void Main_Menu_UI::OnEvent(Event::Event& event)
	{
		on_main_menu_ui_layer_event(*this, event);
	}

	extern Buffer_Data buffer_data;
	void init_main_menu_ui_layer(Main_Menu_UI& ui)
	{
		init_orthographic_camera_controller(ui.camera_controller, 16.0f / 9.0f, 10.0f, false);
		auto ptr = &ui.camera_controller;
		buffer_data.camera_controller = ptr;
	}

	void on_main_menu_ui_layer_attach(Main_Menu_UI& ui)
	{
	}
	void on_main_menu_ui_layer_detach(Main_Menu_UI& ui)
	{
	}

	void on_main_menu_ui_layer_update(Main_Menu_UI& ui, TimeStep ts)
	{
		auto& camera_controller = ui.camera_controller;

		RenderCommand::SetClearColor({ 0.3f, 0.3f, 0.3f, 1.0f });
		RenderCommand::Clear();
		RenderCommand::enable_depth_testing(false);

		set_camera_for_immediate_renderer();

		Rect s;
		s.x = 0.0f;
		s.y = 0.0f;
		s.w = 1280.0f;
		s.h = 720.0f;

		immediate_quad(s, v4(0.3f, 0.5f, 0.8f, 1.0f));
		Rect r;
		r.x = 20.0f;
		r.y = 40.0f;
		r.h = 20.0f;
		r.w = 100.0f;

		immediate_quad(r, v4(0.8f, 0.3f, 0.5f, 1.0f));

		immediate_flush();
		RenderCommand::enable_depth_testing(true);
	}
	void on_main_menu_ui_layer_event(Main_Menu_UI& ui, Event::Event& event)
	{
	}
}