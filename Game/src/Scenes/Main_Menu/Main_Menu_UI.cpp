#include "canpch.h"
#include "Main_Menu_UI.h"

#include "Can/EntryPoint.h"
#include "Can/Renderer/RenderCommand.h"
#include "Can/Renderer/Renderer2D.h"

#include "Can/Immediate_Renderer/Immediate_Renderer.h"

#include "Can/Font/FontFlags.h"
#include "GameApp.h"

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
		Event::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Event::KeyReleasedEvent>(CAN_BIND_EVENT_FN(Main_Menu_UI::OnKeyReleased));
	}

	bool Main_Menu_UI::OnKeyReleased(Event::KeyReleasedEvent& event)
	{
		return on_main_menu_ui_layer_key_released(*this, event);
	}

	extern Buffer_Data buffer_data;
	void init_main_menu_ui_layer(Main_Menu_UI& ui)
	{
		init_orthographic_camera_controller(ui.camera_controller, 16.0f / 9.0f, 10.0f, false);
		auto ptr = &ui.camera_controller;
		buffer_data.camera_controller = ptr;
		ui.font = load_font("assets/fonts/DancingScript/DancingScript-Regular.ttf");
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

		RenderCommand::SetClearColor({ 0.35f, 0.35f, 0.35f, 0.0f });
		RenderCommand::Clear();
		//RenderCommand::enable_depth_testing(false);

		set_camera_for_immediate_renderer();

		switch (ui.current_menu)
		{
		case Menus::MainMenu:
			main_menu_screen(ui);
			break;
		case Menus::Options:
			options_screen(ui);
			break;
		case Menus::Credits:
			credits_screen(ui);
			break;
		default:
			break;
		}

		immediate_flush();
		//RenderCommand::enable_depth_testing(true);
	}


	bool on_main_menu_ui_layer_key_released(Main_Menu_UI& ui, Event::KeyReleasedEvent& event)
	{
		Perspective_Camera_Controller& controller = GameApp::instance->perspective_camera_controller;
		if (ui.key_bind_selection_is_openned)
		{
			ui.key_is_pressed = true;
			ui.key_bind_selection_is_openned = false;
			auto key = event.GetKeyCode();

			if (ui.selected_key == 0) controller.forward_key = key;
			else if (ui.selected_key == 1) controller.backward_key = key;
			else if (ui.selected_key == 2) controller.left_key = key;
			else if (ui.selected_key == 3) controller.right_key = key;
			else if (ui.selected_key == 4) controller.rotate_cw_key = key;
			else if (ui.selected_key == 5) controller.rotate_ccw_key = key;
			else if (ui.selected_key == 6) controller.pitch_down_key = key;
			else if (ui.selected_key == 7) controller.pitch_up_key = key;
			else if (ui.selected_key == 8) controller.raise_key = key;
			else if (ui.selected_key == 9) controller.lower_key = key;
			else if (ui.selected_key == 10) controller.increase_fov_key = key;
			else if (ui.selected_key == 11) controller.decrease_fov_key = key;

			return true;
		}
		return false;
	}

	void main_menu_screen(Main_Menu_UI& ui)
	{
		const s32 margin = 10;

		auto& window = main_application->GetWindow();
		u64 width_in_pixels = window.GetWidth();
		u64 height_in_pixels = window.GetHeight();

		Rect button_rect;
		button_rect.x = 80;
		button_rect.y = 400;
		button_rect.w = 250;
		button_rect.h = 40;


		Label_Theme label_theme;
		label_theme.color = { 0.9f, 0.9f, 0.9f, 1.0f };
		label_theme.font = (Font*)ui.font;
		label_theme.font_size_in_pixel = 85;

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

		label_theme.font_size_in_pixel = 24;
		label_theme.font = nullptr;
		u16 flags = immediate_button(button_rect, text_1, button_theme, __LINE__);
		if (flags & BUTTON_STATE_FLAGS_PRESSED)
			std::cout << "Continue The Game is Pressed\n";
		if (flags & BUTTON_STATE_FLAGS_RELEASED)
			std::cout << "Continue The Game is Released\n";

		button_rect.y -= button_rect.h + margin;
		flags = immediate_button(button_rect, text_2, button_theme, __LINE__);
		if (flags & BUTTON_STATE_FLAGS_PRESSED)
			std::cout << "Start New Game is Pressed\n";
		if (flags & BUTTON_STATE_FLAGS_RELEASED)
			std::cout << "Start New Game is Released\n";

		button_rect.y -= button_rect.h + margin;
		flags = immediate_button(button_rect, text_3, button_theme, __LINE__);
		if (flags & BUTTON_STATE_FLAGS_PRESSED)
			std::cout << "Load A Game is Pressed\n";
		if (flags & BUTTON_STATE_FLAGS_RELEASED)
			std::cout << "Load A Game is Released\n";

		button_rect.y -= button_rect.h + margin;
		flags = immediate_button(button_rect, text_4, button_theme, __LINE__);
		if (flags & BUTTON_STATE_FLAGS_PRESSED)
			std::cout << "Options is Pressed\n";
		if (flags & BUTTON_STATE_FLAGS_RELEASED)
		{
			std::cout << "Options is Released\n";
			if (flags & BUTTON_STATE_FLAGS_OVER)
				ui.current_menu = Menus::Options;
		}

		button_rect.y -= button_rect.h + margin;
		flags = immediate_button(button_rect, text_5, button_theme, __LINE__);
		if (flags & BUTTON_STATE_FLAGS_PRESSED)
			std::cout << "Credits is Pressed\n";
		if (flags & BUTTON_STATE_FLAGS_RELEASED)
		{
			std::cout << "Credits is Released\n";
			if (flags & BUTTON_STATE_FLAGS_OVER)
				ui.current_menu = Menus::Credits;
		}

		button_rect.y = 80;
		flags = immediate_button(button_rect, text_6, button_theme, __LINE__);
		if (flags & BUTTON_STATE_FLAGS_PRESSED)
			std::cout << "Exit is Pressed\n";
		if (flags & BUTTON_STATE_FLAGS_RELEASED)
		{
			std::cout << "Exit is Released\n";
			if (flags & BUTTON_STATE_FLAGS_OVER)
				main_application->m_Running = false;
		}
	}
	void options_screen(Main_Menu_UI& ui)
	{
		const s32 margin = 10;
		s32 left_screen_margin = 50;
		s32 options_y_margin = 5;
		auto& camera_controller = GameApp::instance->perspective_camera_controller;

		auto& window = main_application->GetWindow();
		u64 width_in_pixels = window.GetWidth();
		u64 height_in_pixels = window.GetHeight();

		if (Input::IsKeyPressed(KeyCode::Escape))
			ui.current_menu = Menus::MainMenu;

		Label_Theme label_theme;
		Label_Theme graphics_options_label_theme;
		/*Label_Themes*/ {
			label_theme.font = buffer_data.default_font;
			label_theme.font_size_in_pixel = 85;
			label_theme.color = { 0.9f, 0.9f, 0.9f, 1.0f };

			graphics_options_label_theme.font = buffer_data.default_font;
			graphics_options_label_theme.font_size_in_pixel = 20;
			graphics_options_label_theme.color = { 0.9f, 0.9f, 0.9f, 1.0f };
			graphics_options_label_theme.flags = FontFlags::LeftAligned;
		}

		Button_Theme button_theme;
		Button_Theme button_theme_selected;
		Button_Theme button_theme_centered;
		/*Button_Themes*/ {
			button_theme.label_theme = &label_theme;
			button_theme.background_color = { 0.80f, 0.50f, 0.30f, 1.0f };
			button_theme.background_color_over = { 0.90f, 0.70f, 0.55f, 1.0f };
			button_theme.background_color_pressed = { 0.95f, 0.85f, 0.70f, 1.0f };

			button_theme_selected.label_theme = &graphics_options_label_theme;
			button_theme_selected.background_color = { 0.70f, 0.50f, 0.40f, 1.0f };
			button_theme_selected.background_color_over = { 0.80f, 0.70f, 0.60f, 1.0f };
			button_theme_selected.background_color_pressed = { 0.90f, 0.85f, 0.80f, 1.0f };

			button_theme_centered.label_theme = &label_theme;
			button_theme_centered.background_color = { 0.80f, 0.50f, 0.30f, 1.0f };
			button_theme_centered.background_color_over = { 0.90f, 0.70f, 0.55f, 1.0f };
			button_theme_centered.background_color_pressed = { 0.95f, 0.85f, 0.70f, 1.0f };
		}

		Drop_Down_List_Theme drop_down_list_theme;
		drop_down_list_theme.button_theme = &button_theme;
		drop_down_list_theme.button_theme_selected = &button_theme_selected;
		drop_down_list_theme.background_color = { 0.25f, 0.25f, 0.25f, 1.0f };


		Rect title_rect;
		Rect background_rect;
		Rect popup_rect;
		Rect button_rect;
		Rect cross_rect;
		Rect key_selection_rect;
		/*Rects*/ {
			title_rect.x = 0;
			title_rect.y = height_in_pixels - 200;
			title_rect.w = width_in_pixels;
			title_rect.h = 200;

			background_rect.x = 0;
			background_rect.y = 0;
			background_rect.w = width_in_pixels;
			background_rect.h = height_in_pixels;

			button_rect.x = 80;
			button_rect.y = 400;
			button_rect.w = 250;
			button_rect.h = 40;

			cross_rect.w = 30;
			cross_rect.h = 30;
			cross_rect.x = width_in_pixels - cross_rect.w - 50;
			cross_rect.y = height_in_pixels - cross_rect.w - 50;

			key_selection_rect.w = 100;
			key_selection_rect.h = 50;
			key_selection_rect.x = width_in_pixels - key_selection_rect.w - left_screen_margin;

			popup_rect.w = 200;
			popup_rect.h = 50;
			popup_rect.x = width_in_pixels / 2 - popup_rect.w / 2;
			popup_rect.y = height_in_pixels / 2 - popup_rect.h / 2;
		}

		std::string title = "Options";
		std::string text_1 = "Controls";
		std::string text_2 = "Key Bindings";
		std::string text_3 = "Graphics";
		std::string text_4 = "Audio";
		std::string text_5 = "Others";
		std::string text_6 = "Back";

		immediate_text(title, title_rect, label_theme);


		label_theme.font_size_in_pixel = buffer_data.default_font_size_in_pixel;
		u16 flags = immediate_button(button_rect, text_1, button_theme, __LINE__);
		if (flags & BUTTON_STATE_FLAGS_PRESSED)
			std::cout << "Controls is Pressed\n";
		if (flags & BUTTON_STATE_FLAGS_RELEASED)
		{
			std::cout << "Controls is Released\n";
			if (flags & BUTTON_STATE_FLAGS_OVER)
				ui.current_options_submenus = Options_Submenus::Controls;
		}

		button_rect.y -= button_rect.h + margin;
		flags = immediate_button(button_rect, text_2, button_theme, __LINE__);
		if (flags & BUTTON_STATE_FLAGS_PRESSED)
			std::cout << "Key Bindings is Pressed\n";
		if (flags & BUTTON_STATE_FLAGS_RELEASED)
		{
			std::cout << "Key Bindings is Released\n";
			if (flags & BUTTON_STATE_FLAGS_OVER)
				ui.current_options_submenus = Options_Submenus::Key_Bindings;
		}

		button_rect.y -= button_rect.h + margin;
		flags = immediate_button(button_rect, text_3, button_theme, __LINE__);
		if (flags & BUTTON_STATE_FLAGS_PRESSED)
			std::cout << "Graphics is Pressed\n";
		if (flags & BUTTON_STATE_FLAGS_RELEASED)
		{
			std::cout << "Graphics is Released\n";
			if (flags & BUTTON_STATE_FLAGS_OVER)
				ui.current_options_submenus = Options_Submenus::Graphics;
		}

		button_rect.y -= button_rect.h + margin;
		flags = immediate_button(button_rect, text_4, button_theme, __LINE__);
		if (flags & BUTTON_STATE_FLAGS_PRESSED)
			std::cout << "Audio is Pressed\n";
		if (flags & BUTTON_STATE_FLAGS_RELEASED)
		{
			std::cout << "Audio is Released\n";
			if (flags & BUTTON_STATE_FLAGS_OVER)
				ui.current_options_submenus = Options_Submenus::Audio;
		}

		button_rect.y -= button_rect.h + margin;
		flags = immediate_button(button_rect, text_5, button_theme, __LINE__);
		if (flags & BUTTON_STATE_FLAGS_PRESSED)
			std::cout << "Others is Pressed\n";
		if (flags & BUTTON_STATE_FLAGS_RELEASED)
		{
			std::cout << "Others is Released\n";
			if (flags & BUTTON_STATE_FLAGS_OVER)
				ui.current_options_submenus = Options_Submenus::Others;
		}


		button_rect.y = 80;
		flags = immediate_button(button_rect, text_6, button_theme, __LINE__);
		if (flags & BUTTON_STATE_FLAGS_PRESSED)
			std::cout << "Back is Pressed\n";
		if (flags & BUTTON_STATE_FLAGS_RELEASED)
		{
			std::cout << "Back is Released\n";
			if (flags & BUTTON_STATE_FLAGS_OVER)
				ui.current_menu = Menus::MainMenu;
		}

		button_theme.label_theme = &graphics_options_label_theme;
		if (ui.current_options_submenus == Options_Submenus::Controls)
		{
			std::string text_type = "Control Type : ";
			std::string text_first_person_camera_sensitivity = "First Person Camera Sensitivity : ";
			std::string text_top_down_camera_sensitivity = "Top Down Camera Sensitivity : ";
			std::string text_field_of_view_angle = "Field of View Angle : ";

			std::ostringstream ss_fpc_sensitivity;
			ss_fpc_sensitivity << ui.fpc_sensitivity;
			std::string text_fpc_sensitivity(ss_fpc_sensitivity.str());

			std::ostringstream ss_tdc_sensitivity;
			ss_tdc_sensitivity << ui.tdc_sensitivity;
			std::string text_tdc_sensitivity(ss_tdc_sensitivity.str());

			std::ostringstream ss_fov;
			ss_fov << ui.field_of_view_angle;
			std::string text_fov(ss_fov.str());


			Rect r;
			r.x = 400;
			r.y = 400;
			r.w = width_in_pixels - r.x - 50;
			r.h = 50;
			v4 background_color{ 0.65f, 0.65f, 0.65f, 1.0f };
			immediate_quad(r, background_color);
			r.z++;
			immediate_text(text_type, r, graphics_options_label_theme);
			r.z--;

			Rect drop_list_rect;
			drop_list_rect.w = 200;
			drop_list_rect.h = 50;
			drop_list_rect.x = width_in_pixels - drop_list_rect.w - left_screen_margin;
			drop_list_rect.y = r.y;
			drop_list_rect.z = r.z + 10;

			std::vector<std::string> items{
				"Key Board",
				"Game Pad (Not Implemented)"
			};

			u64 selected_item = 0;
			u16 flags = immediate_drop_down_list(drop_list_rect, items, selected_item, drop_down_list_theme, __LINE__);
			if (flags & DROP_DOWN_LIST_STATE_FLAGS_PRESSED)
				std::cout << "drop down list is Pressed\n";
			if (flags & DROP_DOWN_LIST_STATE_FLAGS_RELEASED)
				std::cout << "drop down list is Released\n";
			if (flags & DROP_DOWN_LIST_STATE_FLAGS_ITEM_CHANGED)
				std::cout << "Active drop down list item is changed, but it doesn't affect the game\n";


			r.y -= r.h + options_y_margin;
			immediate_quad(r, background_color);
			r.z++;
			immediate_text(text_first_person_camera_sensitivity, r, graphics_options_label_theme);
			r.z--;

			Rect track_rect;
			track_rect.w = 200;
			track_rect.h = 8;
			track_rect.x = width_in_pixels - track_rect.w - left_screen_margin;
			track_rect.y = r.y + r.h / 2 - track_rect.h / 2;
			track_rect.z = r.z + 1;

			Rect thumb_rect;
			thumb_rect.w = 30;
			thumb_rect.h = 30;

			Slider_Theme slider_theme;
			slider_theme.track_theme = drop_down_list_theme.button_theme;
			slider_theme.thumb_theme = drop_down_list_theme.button_theme_selected;
			slider_theme.flags = SLIDER_THEME_FLAGS_CLAMP_VALUE;
			//slider_theme.flags |= SLIDER_THEME_FLAGS_THUMB_AT_BOTTOM;
			//slider_theme.y_offset_in_pixels = 5;

			s32 some_value = 50;
			Rect value_rect = r;
			value_rect.w = some_value;
			value_rect.x = track_rect.x - (value_rect.w + margin);
			value_rect.z = r.z + 1;
			Label_Theme value_theme;
			value_theme.color = graphics_options_label_theme.color;
			value_theme.flags = FontFlags::LeftAligned;
			value_theme.font = graphics_options_label_theme.font;
			value_theme.font_size_in_pixel = graphics_options_label_theme.font_size_in_pixel;

			immediate_text(text_fpc_sensitivity, value_rect, value_theme);
			immediate_slider_float(track_rect, thumb_rect, std::string(""), 3.0f, ui.fpc_sensitivity, 6.0f, slider_theme, __LINE__);

			r.y -= r.h + options_y_margin;
			immediate_quad(r, background_color);
			r.z++;
			immediate_text(text_top_down_camera_sensitivity, r, graphics_options_label_theme);
			r.z--;
			track_rect.y = r.y + r.h / 2 - track_rect.h / 2;
			value_rect.y = r.y;
			immediate_text(text_tdc_sensitivity, value_rect, value_theme);
			immediate_slider_float(track_rect, thumb_rect, std::string(""), 3.0f, ui.tdc_sensitivity, 6.0f, slider_theme, __LINE__);


			r.y -= r.h + options_y_margin;
			immediate_quad(r, background_color);
			r.z++;
			immediate_text(text_field_of_view_angle, r, graphics_options_label_theme);
			r.z--;
			track_rect.y = r.y + r.h / 2 - track_rect.h / 2;
			value_rect.y = r.y;
			immediate_text(text_fov, value_rect, value_theme);
			immediate_slider_float(track_rect, thumb_rect, std::string(""), 10.0f, ui.field_of_view_angle, 90.0f, slider_theme, __LINE__);
		}
		else if (ui.current_options_submenus == Options_Submenus::Key_Bindings)
		{
			Rect r;
			r.x = 400;
			r.y = 600;
			r.w = width_in_pixels - r.x - 50;
			r.h = 50;
			v4 background_color{ 0.65f, 0.65f, 0.65f, 1.0f };
			key_selection_rect.z = r.z + 1;

			std::string text_forward_key_bind = "Forward :";
			std::string text_backward_key_bind = "Backward :";
			std::string text_left_key_bind = "Left :";
			std::string text_right_key_bind = "Right :";
			std::string text_rotate_cw_key_bind = "Rotate Clockwise :";
			std::string text_rotate_ccw_key_bind = "Backward Counter Clockwise :";
			std::string text_pitch_down_key_bind = "Pitch Down :";
			std::string text_pitch_up_key_bind = "Pitch Up :";
			std::string text_raise_key_bind = "Raise :";
			std::string text_lower_key_bind = "Lower :";
			std::string text_increase_fov_key_bind = "Increase FOV :";
			std::string text_decrease_fov_key_bind = "Decrease FOV :";

			std::string text_select_a_key = "Press Any Key";


			const std::string& text_selected_forward_key = keycode_to_string(camera_controller.forward_key);
			const std::string& text_selected_backward_key = keycode_to_string(camera_controller.backward_key);
			const std::string& text_selected_left_key = keycode_to_string(camera_controller.left_key);
			const std::string& text_selected_right_key = keycode_to_string(camera_controller.right_key);
			const std::string& text_selected_rotate_cw_key = keycode_to_string(camera_controller.rotate_cw_key);
			const std::string& text_selected_rotate_ccw_key = keycode_to_string(camera_controller.rotate_ccw_key);
			const std::string& text_selected_pitch_down_key = keycode_to_string(camera_controller.pitch_down_key);
			const std::string& text_selected_pitch_up_key = keycode_to_string(camera_controller.pitch_up_key);
			const std::string& text_selected_raise_key = keycode_to_string(camera_controller.raise_key);
			const std::string& text_selected_lower_key = keycode_to_string(camera_controller.lower_key);
			const std::string& text_selected_increase_fov_key = keycode_to_string(camera_controller.increase_fov_key);
			const std::string& text_selected_decrease_fov_key = keycode_to_string(camera_controller.decrease_fov_key);
			std::string text_cross = "X";

			{
				immediate_quad(r, background_color);
				r.z++;
				immediate_text(text_forward_key_bind, r, graphics_options_label_theme);
				r.z--;
				key_selection_rect.y = r.y;
				u16 flags = immediate_button(key_selection_rect, text_selected_forward_key, button_theme_centered, __LINE__);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
				{
					ui.key_bind_selection_is_openned = true;
					ui.selected_key = 0;
				}
			}
			{
				r.y -= r.h + options_y_margin;
				immediate_quad(r, background_color);
				r.z++;
				immediate_text(text_backward_key_bind, r, graphics_options_label_theme);
				r.z--;
				key_selection_rect.y = r.y;
				u16 flags = immediate_button(key_selection_rect, text_selected_backward_key, button_theme_centered, __LINE__);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
				{
					ui.key_bind_selection_is_openned = true;
					ui.selected_key = 1;
				}
			}
			{
				r.y -= r.h + options_y_margin;
				immediate_quad(r, background_color);
				r.z++;
				immediate_text(text_left_key_bind, r, graphics_options_label_theme);
				r.z--;
				key_selection_rect.y = r.y;
				u16 flags = immediate_button(key_selection_rect, text_selected_left_key, button_theme_centered, __LINE__);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
				{
					ui.key_bind_selection_is_openned = true;
					ui.selected_key = 2;
				}
			}
			{
				r.y -= r.h + options_y_margin;
				immediate_quad(r, background_color);
				r.z++;
				immediate_text(text_right_key_bind, r, graphics_options_label_theme);
				r.z--;
				key_selection_rect.y = r.y;
				u16 flags = immediate_button(key_selection_rect, text_selected_right_key, button_theme_centered, __LINE__);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
				{
					ui.key_bind_selection_is_openned = true;
					ui.selected_key = 3;
				}
			}
			{
				r.y -= r.h + options_y_margin;
				immediate_quad(r, background_color);
				r.z++;
				immediate_text(text_rotate_cw_key_bind, r, graphics_options_label_theme);
				r.z--;
				key_selection_rect.y = r.y;
				u16 flags = immediate_button(key_selection_rect, text_selected_rotate_cw_key, button_theme_centered, __LINE__);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
				{
					ui.key_bind_selection_is_openned = true;
					ui.selected_key = 4;
				}
			}
			{
				r.y -= r.h + options_y_margin;
				immediate_quad(r, background_color);
				r.z++;
				immediate_text(text_rotate_ccw_key_bind, r, graphics_options_label_theme);
				r.z--;
				key_selection_rect.y = r.y;
				u16 flags = immediate_button(key_selection_rect, text_selected_rotate_ccw_key, button_theme_centered, __LINE__);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
				{
					ui.key_bind_selection_is_openned = true;
					ui.selected_key = 5;
				}
			}
			{
				r.y -= r.h + options_y_margin;
				immediate_quad(r, background_color);
				r.z++;
				immediate_text(text_pitch_down_key_bind, r, graphics_options_label_theme);
				r.z--;
				key_selection_rect.y = r.y;
				u16 flags = immediate_button(key_selection_rect, text_selected_pitch_down_key, button_theme_centered, __LINE__);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
				{
					ui.key_bind_selection_is_openned = true;
					ui.selected_key = 6;
				}
			}
			{
				r.y -= r.h + options_y_margin;
				immediate_quad(r, background_color);
				r.z++;
				immediate_text(text_pitch_up_key_bind, r, graphics_options_label_theme);
				r.z--;
				key_selection_rect.y = r.y;
				u16 flags = immediate_button(key_selection_rect, text_selected_pitch_up_key, button_theme_centered, __LINE__);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
				{
					ui.key_bind_selection_is_openned = true;
					ui.selected_key = 7;
				}
			}
			{
				r.y -= r.h + options_y_margin;
				immediate_quad(r, background_color);
				r.z++;
				immediate_text(text_raise_key_bind, r, graphics_options_label_theme);
				r.z--;
				key_selection_rect.y = r.y;
				u16 flags = immediate_button(key_selection_rect, text_selected_raise_key, button_theme_centered, __LINE__);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
				{
					ui.key_bind_selection_is_openned = true;
					ui.selected_key = 8;
				}
			}
			{
				r.y -= r.h + options_y_margin;
				immediate_quad(r, background_color);
				r.z++;
				immediate_text(text_lower_key_bind, r, graphics_options_label_theme);
				r.z--;
				key_selection_rect.y = r.y;
				u16 flags = immediate_button(key_selection_rect, text_selected_lower_key, button_theme_centered, __LINE__);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
				{
					ui.key_bind_selection_is_openned = true;
					ui.selected_key = 9;
				}
			}
			{
				r.y -= r.h + options_y_margin;
				immediate_quad(r, background_color);
				r.z++;
				immediate_text(text_increase_fov_key_bind, r, graphics_options_label_theme);
				r.z--;
				key_selection_rect.y = r.y;
				u16 flags = immediate_button(key_selection_rect, text_selected_increase_fov_key, button_theme_centered, __LINE__);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
				{
					ui.key_bind_selection_is_openned = true;
					ui.selected_key = 10;
				}
			}
			{
				r.y -= r.h + options_y_margin;
				immediate_quad(r, background_color);
				r.z++;
				immediate_text(text_decrease_fov_key_bind, r, graphics_options_label_theme);
				r.z--;
				key_selection_rect.y = r.y;
				u16 flags = immediate_button(key_selection_rect, text_selected_decrease_fov_key, button_theme_centered, __LINE__);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
				{
					ui.key_bind_selection_is_openned = true;
					ui.selected_key = 11;
				}
			}

			if (ui.key_bind_selection_is_openned)
			{
				v4 transparent_color(0.0f);
				transparent_color.a = 0.65f;

				background_rect.z = r.z + 100;
				popup_rect.z = r.z + 110;
				cross_rect.z = r.z + 110;

				v4 text_background_color{ 0.80f, 0.50f, 0.30f, 1.0f };


				immediate_quad(background_rect, transparent_color);
				immediate_quad(popup_rect, text_background_color);
				popup_rect.z++;
				immediate_text(text_select_a_key, popup_rect, label_theme);

				flags = immediate_button(cross_rect, text_cross, button_theme_centered, __LINE__);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
					ui.key_bind_selection_is_openned = false;

			}
		}
		else if (ui.current_options_submenus == Options_Submenus::Graphics)
		{

			std::string text_res = "Window Resolution : ";

			Rect r;
			r.x = 400;
			r.y = 400;
			r.w = width_in_pixels - r.x - 50;
			r.h = 50;
			v4 background_color{ 0.65f, 0.65f, 0.65f, 1.0f };
			immediate_quad(r, background_color);
			r.z++;
			immediate_text(text_res, r, graphics_options_label_theme);
			r.z--;

			Rect res_rect;
			res_rect.w = 200;
			res_rect.h = 50;
			res_rect.x = width_in_pixels - res_rect.w - 50;
			res_rect.y = r.y;
			res_rect.z = r.z + 1;

			std::vector<std::string> items{
				"1920 x 1080",
				"1600 x 900",
				"1280 x 1024",
				"1280 x 720",
				"800  x 600"
			};

			u64 selected_item = 3;
			u16 flags = immediate_drop_down_list(res_rect, items, selected_item, drop_down_list_theme, __LINE__);
			if (flags & DROP_DOWN_LIST_STATE_FLAGS_PRESSED)
				std::cout << "drop down list is Pressed\n";
			if (flags & DROP_DOWN_LIST_STATE_FLAGS_RELEASED)
				std::cout << "drop down list is Released\n";
			if (flags & DROP_DOWN_LIST_STATE_FLAGS_ITEM_CHANGED)
			{
				std::cout << "Active drop down list item is changed\n";
				//Change The settings;
				{/*Temp*/
					auto& window = main_application->GetWindow();
					if (selected_item == 0)
						window.set_resolution(1920, 1080);
					else if (selected_item == 1)
						window.set_resolution(1600, 900);
					else if (selected_item == 2)
						window.set_resolution(1280, 1024);
					else if (selected_item == 3)
						window.set_resolution(1280, 720);
					else if (selected_item == 4)
						window.set_resolution(800, 600);
				}
			}
		}
		else if (ui.current_options_submenus == Options_Submenus::Audio)
		{

		}
		else if (ui.current_options_submenus == Options_Submenus::Others)
		{

		}
	}
	void credits_screen(Main_Menu_UI& ui)
	{
		auto& window = main_application->GetWindow();
		u64 width_in_pixels = window.GetWidth();
		u64 height_in_pixels = window.GetHeight();

		if (Input::IsKeyPressed(KeyCode::Escape))
			ui.current_menu = Menus::MainMenu;

		Label_Theme label_theme;
		label_theme.font = buffer_data.default_font;
		label_theme.font_size_in_pixel = 85;
		label_theme.color = { 0.9f, 0.9f, 0.9f, 1.0f };

		Rect title_rect;
		title_rect.x = 0;
		title_rect.y = height_in_pixels - 200;
		title_rect.w = width_in_pixels;
		title_rect.h = 200;

		std::string title = "Credits";
		std::string name_1 = "Emre Can Haciosmanoglu";
		std::string name_2 = "Vice President: Muhammed Talha Demir";
		std::string back = "Back";

		immediate_text(title, title_rect, label_theme);

		label_theme.font_size_in_pixel = 60;
		title_rect.y = height_in_pixels - 400;
		title_rect.h = 150;
		immediate_text(name_1, title_rect, label_theme);

		label_theme.font_size_in_pixel = 7;
		title_rect.y = 0;
		title_rect.h = 50;
		label_theme.flags = FontFlags::RightAligned | FontFlags::BottomAligned;
		immediate_text(name_2, title_rect, label_theme);

		label_theme.font_size_in_pixel = buffer_data.default_font_size_in_pixel;
		label_theme.flags = 0;
		Button_Theme button_theme;
		button_theme.label_theme = &label_theme;
		button_theme.background_color = { 0.80f, 0.50f, 0.30f, 1.0f };
		button_theme.background_color_over = { 0.90f, 0.70f, 0.55f, 1.0f };
		button_theme.background_color_pressed = { 0.95f, 0.85f, 0.70f, 1.0f };

		Rect button_rect;
		button_rect.x = 80;
		button_rect.y = 80;
		button_rect.w = 250;
		button_rect.h = 40;

		u16 flags = immediate_button(button_rect, back, button_theme, __LINE__);
		if (flags & BUTTON_STATE_FLAGS_PRESSED)
			std::cout << "Back is Pressed\n";
		if (flags & BUTTON_STATE_FLAGS_RELEASED)
		{
			std::cout << "Back is Released\n";
			if (flags & BUTTON_STATE_FLAGS_OVER)
				ui.current_menu = Menus::MainMenu;
		}
	}
}