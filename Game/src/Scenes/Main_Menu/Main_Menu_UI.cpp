#include "canpch.h"
#include "Main_Menu_UI.h"

#include "Can/EntryPoint.h"
#include "Can/Renderer/RenderCommand.h"
#include "Can/Renderer/Renderer2D.h"

#include "Can/Immediate_Renderer/Immediate_Renderer.h"

#include "Can/Font/FontFlags.h"

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
		//ui.font = new Font("assets/fonts/DancingScript/DancingScript-Regular.ttf");
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
		auto& window = main_application->GetWindow();
		u64 width_in_pixels = window.GetWidth();
		u64 height_in_pixels = window.GetHeight();

		RenderCommand::SetClearColor({ 0.35f, 0.35f, 0.35f, 1.0f });
		RenderCommand::Clear();
		RenderCommand::enable_depth_testing(false);

		set_camera_for_immediate_renderer();


		Rect button_rect;
		button_rect.x = 100;
		button_rect.y = 400;
		button_rect.w = 250;
		button_rect.h = 40;

		Label_Theme label_theme;
		label_theme.color = { 0.9f, 0.9f, 0.9f, 1.0f };

		Button_Theme button_theme;
		button_theme.background_color = { 0.80f, 0.50f, 0.30f, 1.0f };
		button_theme.background_color_over = { 0.90f, 0.70f, 0.55f, 1.0f };
		button_theme.background_color_pressed = { 0.95f, 0.85f, 0.70f, 1.0f };
		button_theme.label_theme = &label_theme;

		std::string title = "Sky Is The Limit";
		std::string text_1 = "Continue The Game";
		std::string text_2 = "Start New Game";
		std::string text_3 = "Load A Game";
		std::string text_4 = "Options";
		std::string text_5 = "Credits";
		std::string text_6 = "Exit";

		Rect title_rect;
		title_rect.x = 0;
		title_rect.y = height_in_pixels - 200;
		title_rect.w = width_in_pixels;
		title_rect.h = 200;

		immediate_text(title, title_rect, label_theme);

		//label_theme.font = ui.font;
		label_theme.font_size_in_pixel = 18;

		u8 flags = immediate_button(button_rect, text_1, button_theme, 1);
		if (flags & BUTTON_STATE_FLAGS_PRESSED)
			std::cout << "Continue The Game is Pressed\n";
		if (flags & BUTTON_STATE_FLAGS_RELEASED)
			std::cout << "Continue The Game is Released\n";

		button_rect.y = 350;
		flags = immediate_button(button_rect, text_2, button_theme, 2);
		if (flags & BUTTON_STATE_FLAGS_PRESSED)
			std::cout << "Start New Game is Pressed\n";
		if (flags & BUTTON_STATE_FLAGS_RELEASED)
			std::cout << "Start New Game is Released\n";

		button_rect.y = 300;
		flags = immediate_button(button_rect, text_3, button_theme, 3);
		if (flags & BUTTON_STATE_FLAGS_PRESSED)
			std::cout << "Load A Game is Pressed\n";
		if (flags & BUTTON_STATE_FLAGS_RELEASED)
			std::cout << "Load A Game is Released\n";

		button_rect.y = 250;
		flags = immediate_button(button_rect, text_4, button_theme, 4);
		if (flags & BUTTON_STATE_FLAGS_PRESSED)
			std::cout << "Options is Pressed\n";
		if (flags & BUTTON_STATE_FLAGS_RELEASED)
			std::cout << "Options is Released\n";

		button_rect.y = 200;
		flags = immediate_button(button_rect, text_5, button_theme, 5);
		if (flags & BUTTON_STATE_FLAGS_PRESSED)
			std::cout << "Credits is Pressed\n";
		if (flags & BUTTON_STATE_FLAGS_RELEASED)
			std::cout << "Credits is Released\n";

		button_rect.y = 150;
		flags = immediate_button(button_rect, text_6, button_theme, 6);
		if (flags & BUTTON_STATE_FLAGS_PRESSED)
			std::cout << "Exit is Pressed\n";
		if (flags & BUTTON_STATE_FLAGS_RELEASED)
			std::cout << "Exit is Released\n";

		immediate_flush();
		RenderCommand::enable_depth_testing(true);
	}
	void on_main_menu_ui_layer_event(Main_Menu_UI& ui, Event::Event& event)
	{
	}
}