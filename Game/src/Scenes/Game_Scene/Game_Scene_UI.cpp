#include "canpch.h"
#include "Game_Scene_UI.h"
#include "Scenes/GameScene.h"

#include "Can/EntryPoint.h"
#include "Can/Renderer/RenderCommand.h"
#include "Can/Renderer/Renderer2D.h"

#include "Can/Camera/Perspective_Camera_Controller.h"
#include "Can/Renderer/Object.h"


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
	void init_game_scene_ui_layer(Game_Scene_UI& ui, GameScene& game_scene)
	{
		init_orthographic_camera_controller(ui.camera_controller, 16.0f / 9.0f, 10.0f, false);
		auto ptr = &ui.camera_controller;
		buffer_data.camera_controller = ptr;
		ui.font = load_font("assets/fonts/DancingScript/DancingScript-Regular.ttf");
		ui.game_scene_camera = &game_scene.camera_controller.camera;


		/*Label_Themes*/ {
			ui.label_theme_button.color = { 0.1f, 0.1f, 0.1f, 1.0f };
			ui.label_theme_button.font = buffer_data.default_font;
			ui.label_theme_button.font_size_in_pixel = 30;

			ui.label_theme_text.color = { 0.9f, 0.9f, 0.9f, 1.0f };
			ui.label_theme_text.font = buffer_data.default_font;
			ui.label_theme_text.font_size_in_pixel = 20;
		}
		/*Button_Themes*/ {
			ui.button_theme_cross.label_theme = &ui.label_theme_button;
			ui.button_theme_cross.background_color = { 0.80f, 0.50f, 0.30f, 1.0f };
			ui.button_theme_cross.background_color_over = { 0.90f, 0.70f, 0.55f, 1.0f };
			ui.button_theme_cross.background_color_pressed = { 0.95f, 0.85f, 0.70f, 1.0f };

			ui.button_theme_thumb.label_theme = &ui.label_theme_button;
			ui.button_theme_thumb.background_color = { 0.80f, 0.50f, 0.30f, 1.0f };
			ui.button_theme_thumb.background_color_over = { 0.90f, 0.70f, 0.55f, 1.0f };
			ui.button_theme_thumb.background_color_pressed = { 0.95f, 0.85f, 0.70f, 1.0f };

			ui.button_theme_track.label_theme = &ui.label_theme_button;
			ui.button_theme_track.background_color = { 0.70f, 0.50f, 0.40f, 1.0f };
			ui.button_theme_track.background_color_over = { 0.80f, 0.70f, 0.60f, 1.0f };
			ui.button_theme_track.background_color_pressed = { 0.90f, 0.85f, 0.80f, 1.0f };
		}
		/*Slider_Themes*/ {
			ui.slider_theme_horizontal.track_theme = &ui.button_theme_track;
			ui.slider_theme_horizontal.thumb_theme = &ui.button_theme_thumb;
			ui.slider_theme_horizontal.flags = SLIDER_THEME_FLAGS_CLAMP_VALUE;
			ui.slider_theme_horizontal.flags |= SLIDER_THEME_FLAGS_THUMB_IS_INSIDE;
			ui.slider_theme_horizontal.flags |= SLIDER_THEME_FLAGS_IS_HORIZONTAL;

			ui.slider_theme_vertical.track_theme = &ui.button_theme_track;
			ui.slider_theme_vertical.thumb_theme = &ui.button_theme_thumb;
			ui.slider_theme_vertical.flags = SLIDER_THEME_FLAGS_CLAMP_VALUE;
			ui.slider_theme_vertical.flags |= SLIDER_THEME_FLAGS_THUMB_IS_INSIDE;
		}
		/*Sub_Region_Themes*/ {
			ui.sub_region_theme_details.background_color = { 0.9f, 0.9f, 0.9f, 1.0f };
			ui.sub_region_theme_details.horizontal_slider_theme = &ui.slider_theme_horizontal;
			ui.sub_region_theme_details.vertical_slider_theme = &ui.slider_theme_vertical;
			ui.sub_region_theme_details.flags |= SUB_REGION_THEME_FLAGS_HORIZONTALLY_SCROLLABLE;
			ui.sub_region_theme_details.flags |= SUB_REGION_THEME_FLAGS_VERTICALLY_SCROLLABLE;
		}
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

		update_screen(ui);
		draw_screen(ui);

		immediate_flush();
		return ui.force_update;
		return false;
	}

	bool on_game_scene_ui_layer_key_released(void* p, Event::KeyReleasedEvent& event)
	{
		Game_Scene_UI& ui = *((Game_Scene_UI*)p);

		KeyCode key_code = event.GetKeyCode();
		if (key_code == KeyCode::Escape)
		{
			if (ui.focus_object != nullptr)
			{
				ui.focus_object = nullptr;
				return true;
			}
		}
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

	void update_screen(Game_Scene_UI& ui)
	{
		auto& window = main_application->GetWindow();
		u32 width_in_pixels = window.GetWidth();
		u32 height_in_pixels = window.GetHeight();

		if (ui.focus_object)
		{
			v4 position_on_screen = ui.game_scene_camera->view_projection * ui.focus_object->transform * v4(0.0f, 0.0f, ui.focus_object->prefab->boundingBoxM.z, 1.0f);
			ui.menu_x = glm::clamp(u32(glm::clamp(position_on_screen.x / position_on_screen.w + 1.0f, 0.0f,2.0f) * width_in_pixels * 0.5f), 0U, width_in_pixels - 200);
			ui.menu_y = glm::clamp(u32(glm::clamp(position_on_screen.y / position_on_screen.w + 1.0f, 0.0f, 2.0f) * height_in_pixels * 0.5f) - 200, 0U, height_in_pixels - 200);
		}
	}

	void draw_screen(Game_Scene_UI& ui)
	{
		auto& window = main_application->GetWindow();
		u64 width_in_pixels = window.GetWidth();
		u64 height_in_pixels = window.GetHeight();

		const s32 track_width = 20;
		const s32 sub_region_button_margin = 5;


		if (ui.focus_object != nullptr)
		{
			Rect rect_sub_region;
			rect_sub_region.w = 200;
			rect_sub_region.h = 200;
			rect_sub_region.x = ui.menu_x; //ui.mouse_pos_x;
			rect_sub_region.y = ui.menu_y; //(height_in_pixels - ui.mouse_pos_y) - rect_sub_region.h;

			Rect rect_button_cross;
			rect_button_cross.w = 40;
			rect_button_cross.h = 40;
			rect_button_cross.x = rect_sub_region.x + rect_sub_region.w - rect_button_cross.w;
			rect_button_cross.y = rect_sub_region.y + rect_sub_region.h - rect_button_cross.h;


			std::string text_x = "X";
			immediate_quad(rect_sub_region, ui.sub_region_theme_details.background_color);

			u16 flags = immediate_button(rect_button_cross, text_x, ui.button_theme_cross, __LINE__, true);
			if (flags & BUTTON_STATE_FLAGS_PRESSED)
				std::cout << "Close is Pressed\n";
			if (flags & BUTTON_STATE_FLAGS_RELEASED)
			{
				std::cout << "Close is Released\n";
				ui.focus_object = nullptr;
				return;
			}
		}
	}
	}
}