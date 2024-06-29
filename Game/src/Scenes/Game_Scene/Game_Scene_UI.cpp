#include "canpch.h"
#include "Game_Scene_UI.h"
#include "Scenes/GameScene.h"

#include "Can/EntryPoint.h"
#include "Can/Renderer/RenderCommand.h"
#include "Can/Renderer/Renderer2D.h"

#include "Can/Camera/Perspective_Camera_Controller.h"
#include "Can/Renderer/Object.h"

#include "Can/Font/FontFlags.h"
#include "GameApp.h"
#include "Helper.h"
#include "Building.h"

namespace Can
{
	/*Anom*/ namespace
	{
		void draw_building_panel(Game_Scene_UI& ui)
		{
			auto app{ GameApp::instance };
			auto& window{ main_application->GetWindow() };
			auto& building_types{ app->building_types };
			u64 width_in_pixels{ window.GetWidth() };
			u64 height_in_pixels{ window.GetHeight() };

			constexpr s32 building_panel_margins{ 10 };
			constexpr s32 button_margin{ 10 };
			constexpr s32 button_size{ 30 };
			constexpr s32 menu_item_size{ 128 };

			Sub_Region_Theme sub_region_theme;
			/*Sub_Region_Themes*/ {
				sub_region_theme.background_color = v4{ 201.0f / 255.0f, 235.0f / 255.0f, 227.0f / 255.0f, 1.0f };
				sub_region_theme.horizontal_slider_theme = &ui.slider_theme_horizontal;
				sub_region_theme.vertical_slider_theme = &ui.slider_theme_vertical;
				sub_region_theme.flags |= SUB_REGION_THEME_FLAGS_HORIZONTALLY_SCROLLABLE;
				//sub_region_theme.flags |= SUB_REGION_THEME_FLAGS_VERTICALLY_SCROLLABLE;
			}

			Rect building_panel_rect{};
			building_panel_rect.x = building_panel_margins;
			building_panel_rect.y = building_panel_margins;
			building_panel_rect.z = 1;
			building_panel_rect.w = width_in_pixels - (building_panel_margins << 1);
			building_panel_rect.h = 180;

			Rect sub_region_rect{};
			sub_region_rect.x = building_panel_margins + (width_in_pixels >> 2);
			sub_region_rect.y = building_panel_margins << 1;
			sub_region_rect.z = 2;
			sub_region_rect.w = width_in_pixels * 0.75f - (building_panel_margins * 3);
			sub_region_rect.h = building_panel_rect.h - (building_panel_margins << 1);

			Rect construction_mode_button_rect;
			construction_mode_button_rect.w = button_size;
			construction_mode_button_rect.h = button_size;
			construction_mode_button_rect.x = building_panel_rect.x + button_margin;
			construction_mode_button_rect.y = building_panel_rect.y + building_panel_rect.h - (button_size + button_margin);
			construction_mode_button_rect.z = building_panel_rect.z + 1;


			immediate_quad(building_panel_rect, v4{ 1.0f, 0.6f, 0.6f, 1.0f });

			u16 flags{ 0 };
			flags = immediate_image_button(construction_mode_button_rect, ui.button_theme_construction_modes, app->addTexture, __LINE__);
			if (flags & BUTTON_STATE_FLAGS_RELEASED)
			{
				ui.game_scene->SetConstructionMode(ConstructionMode::Building);
				ui.game_scene->m_BuildingManager.SetConstructionMode(BuildingConstructionMode::Construct);
			}

			construction_mode_button_rect.x += button_size + button_margin;
			flags = immediate_image_button(construction_mode_button_rect, ui.button_theme_construction_modes, app->removeTexture, __LINE__);
			if (flags & BUTTON_STATE_FLAGS_RELEASED)
			{
				ui.game_scene->SetConstructionMode(ConstructionMode::Building);
				ui.game_scene->m_BuildingManager.SetConstructionMode(BuildingConstructionMode::Destruct);
			}

			construction_mode_button_rect.x += button_size + button_margin;
			flags = immediate_image_button(construction_mode_button_rect, ui.button_theme_construction_modes, app->upgradeTexture, __LINE__);
			if (flags & BUTTON_STATE_FLAGS_RELEASED)
			{
				ui.game_scene->SetConstructionMode(ConstructionMode::Building);
				ui.game_scene->m_BuildingManager.SetConstructionMode(BuildingConstructionMode::Upgrade);
			}

			construction_mode_button_rect.x += button_size + button_margin;
			flags = immediate_image_button(construction_mode_button_rect, ui.button_theme_construction_modes, app->cancelTexture, __LINE__);
			if (flags & BUTTON_STATE_FLAGS_RELEASED)
			{
				ui.game_scene->SetConstructionMode(ConstructionMode::Building);
				ui.game_scene->m_BuildingManager.SetConstructionMode(BuildingConstructionMode::None);
			}

			//immediate_begin_sub_region(sub_region_rect, sub_region_theme, __LINE__);
			immediate_quad(sub_region_rect, v4{ 1.00f, 0.55f, 0.55f, 1.0f });

			Rect menu_item_rect;
			menu_item_rect.w = menu_item_size;
			menu_item_rect.h = menu_item_size;
			menu_item_rect.x = sub_region_rect.x + button_margin;
			menu_item_rect.y = sub_region_rect.y + sub_region_rect.h - menu_item_rect.h - button_margin;
			menu_item_rect.z = sub_region_rect.z + 1;

			if (ui.draw_building_panel_inside_type == Draw_Building_Panel::Home)
			{
				flags = immediate_image_button(menu_item_rect, ui.button_theme_back, nullptr, __LINE__);
				menu_item_rect.z++;
				immediate_text("-", menu_item_rect, ui.label_theme_large_text);

				menu_item_rect.x += menu_item_rect.w + button_margin;
				flags = immediate_image_button(menu_item_rect, ui.button_theme_sub_menus, nullptr, __LINE__);
				menu_item_rect.z++;
				immediate_text("G", menu_item_rect, ui.label_theme_large_text);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
					ui.draw_building_panel_inside_type = Draw_Building_Panel::General;

				menu_item_rect.x += menu_item_rect.w + button_margin;
				flags = immediate_image_button(menu_item_rect, ui.button_theme_sub_menus, nullptr, __LINE__);
				menu_item_rect.z++;
				immediate_text("S", menu_item_rect, ui.label_theme_large_text);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
					ui.draw_building_panel_inside_type = Draw_Building_Panel::Special;
			}
			else if (ui.draw_building_panel_inside_type == Draw_Building_Panel::General)
			{
				flags = immediate_image_button(menu_item_rect, ui.button_theme_back, nullptr, __LINE__);
				menu_item_rect.z++;
				immediate_text("B", menu_item_rect, ui.label_theme_large_text);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
					ui.draw_building_panel_inside_type = Draw_Building_Panel::Home;

				menu_item_rect.x += menu_item_rect.w + button_margin;
				flags = immediate_image_button(menu_item_rect, ui.button_theme_sub_menus, app->housesTexture, __LINE__);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
					ui.draw_building_panel_inside_type = Draw_Building_Panel::House;

				menu_item_rect.x += menu_item_rect.w + button_margin;
				flags = immediate_image_button(menu_item_rect, ui.button_theme_sub_menus, nullptr, __LINE__);
				menu_item_rect.z++;
				immediate_text("R", menu_item_rect, ui.label_theme_large_text);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
					ui.draw_building_panel_inside_type = Draw_Building_Panel::Residential;

				menu_item_rect.x += menu_item_rect.w + button_margin;
				flags = immediate_image_button(menu_item_rect, ui.button_theme_sub_menus, nullptr, __LINE__);
				menu_item_rect.z++;
				immediate_text("C", menu_item_rect, ui.label_theme_large_text);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
					ui.draw_building_panel_inside_type = Draw_Building_Panel::Commercial;

				menu_item_rect.x += menu_item_rect.w + button_margin;
				flags = immediate_image_button(menu_item_rect, ui.button_theme_sub_menus, nullptr, __LINE__);
				menu_item_rect.z++;
				immediate_text("I", menu_item_rect, ui.label_theme_large_text);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
					ui.draw_building_panel_inside_type = Draw_Building_Panel::Industrial;

				menu_item_rect.x += menu_item_rect.w + button_margin;
				flags = immediate_image_button(menu_item_rect, ui.button_theme_sub_menus, nullptr, __LINE__);
				menu_item_rect.z++;
				immediate_text("O", menu_item_rect, ui.label_theme_large_text);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
					ui.draw_building_panel_inside_type = Draw_Building_Panel::Office;
			}
			else if (ui.draw_building_panel_inside_type == Draw_Building_Panel::Special)
			{
				flags = immediate_image_button(menu_item_rect, ui.button_theme_back, nullptr, __LINE__);
				menu_item_rect.z++;
				immediate_text("B", menu_item_rect, ui.label_theme_large_text);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
					ui.draw_building_panel_inside_type = Draw_Building_Panel::Home;

				menu_item_rect.x += menu_item_rect.w + button_margin;
				flags = immediate_image_button(menu_item_rect, ui.button_theme_sub_menus, app->hospitalsTexture, __LINE__);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
					ui.draw_building_panel_inside_type = Draw_Building_Panel::Hospital;

				menu_item_rect.x += menu_item_rect.w + button_margin;
				flags = immediate_image_button(menu_item_rect, ui.button_theme_sub_menus, nullptr, __LINE__);
				menu_item_rect.z++;
				immediate_text("P", menu_item_rect, ui.label_theme_large_text);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
					ui.draw_building_panel_inside_type = Draw_Building_Panel::Police_Station;

				menu_item_rect.x += menu_item_rect.w + button_margin;
				flags = immediate_image_button(menu_item_rect, ui.button_theme_sub_menus, nullptr, __LINE__);
				menu_item_rect.z++;
				immediate_text("G", menu_item_rect, ui.label_theme_large_text);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
					ui.draw_building_panel_inside_type = Draw_Building_Panel::Garbage_Collection_Center;
			}
			else if (ui.draw_building_panel_inside_type == Draw_Building_Panel::House)
			{
				flags = immediate_image_button(menu_item_rect, ui.button_theme_back, nullptr, __LINE__);
				menu_item_rect.z++;
				immediate_text("B", menu_item_rect, ui.label_theme_large_text);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
					ui.draw_building_panel_inside_type = Draw_Building_Panel::General;

				for (u64 i{ 0 }; i < building_types.size(); ++i)
				{
					auto& building_type{ building_types[i] };
					if (building_type.group != Building_Group::House) continue;

					menu_item_rect.x += menu_item_rect.w + button_margin;
					flags = immediate_image_button(menu_item_rect, ui.button_theme_buildings, building_type.thumbnail, __LINE__ * 10000 + i, false);
					if (flags & BUTTON_STATE_FLAGS_RELEASED)
					{
						ui.game_scene->SetConstructionMode(ConstructionMode::Building);
						auto mode = ui.game_scene->m_BuildingManager.GetConstructionMode();
						if (mode == BuildingConstructionMode::None || mode == BuildingConstructionMode::Destruct)
							ui.game_scene->m_BuildingManager.SetConstructionMode(BuildingConstructionMode::Construct);
						ui.game_scene->m_BuildingManager.SetType(i);
					}
				}
			}
			else if (ui.draw_building_panel_inside_type == Draw_Building_Panel::Residential)
			{
				flags = immediate_image_button(menu_item_rect, ui.button_theme_back, nullptr, __LINE__);
				menu_item_rect.z++;
				immediate_text("B", menu_item_rect, ui.label_theme_large_text);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
					ui.draw_building_panel_inside_type = Draw_Building_Panel::General;

			}
			else if (ui.draw_building_panel_inside_type == Draw_Building_Panel::Commercial)
			{
				flags = immediate_image_button(menu_item_rect, ui.button_theme_back, nullptr, __LINE__);
				menu_item_rect.z++;
				immediate_text("B", menu_item_rect, ui.label_theme_large_text);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
					ui.draw_building_panel_inside_type = Draw_Building_Panel::General;

				for (u64 i{ 0 }; i < building_types.size(); ++i)
				{
					auto& building_type{ building_types[i] };
					if (building_type.group != Building_Group::Commercial) continue;

					menu_item_rect.x += menu_item_rect.w + button_margin;
					flags = immediate_image_button(menu_item_rect, ui.button_theme_buildings, building_type.thumbnail, __LINE__ * 10000 + i, false);
					if (flags & BUTTON_STATE_FLAGS_RELEASED)
					{
						ui.game_scene->SetConstructionMode(ConstructionMode::Building);
						auto mode = ui.game_scene->m_BuildingManager.GetConstructionMode();
						if (mode == BuildingConstructionMode::None || mode == BuildingConstructionMode::Destruct)
							ui.game_scene->m_BuildingManager.SetConstructionMode(BuildingConstructionMode::Construct);
						ui.game_scene->m_BuildingManager.SetType(i);
					}
				}
			}
			else if (ui.draw_building_panel_inside_type == Draw_Building_Panel::Industrial)
			{
				flags = immediate_image_button(menu_item_rect, ui.button_theme_back, nullptr, __LINE__);
				menu_item_rect.z++;
				immediate_text("B", menu_item_rect, ui.label_theme_large_text);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
					ui.draw_building_panel_inside_type = Draw_Building_Panel::General;
			}
			else if (ui.draw_building_panel_inside_type == Draw_Building_Panel::Office)
			{
				flags = immediate_image_button(menu_item_rect, ui.button_theme_back, nullptr, __LINE__);
				menu_item_rect.z++;
				immediate_text("B", menu_item_rect, ui.label_theme_large_text);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
					ui.draw_building_panel_inside_type = Draw_Building_Panel::General;
			}
			else if (ui.draw_building_panel_inside_type == Draw_Building_Panel::Hospital)
			{
				flags = immediate_image_button(menu_item_rect, ui.button_theme_back, nullptr, __LINE__);
				menu_item_rect.z++;
				immediate_text("B", menu_item_rect, ui.label_theme_large_text);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
					ui.draw_building_panel_inside_type = Draw_Building_Panel::Special;

				for (u64 i{ 0 }; i < building_types.size(); ++i)
				{
					auto& building_type{ building_types[i] };
					if (building_type.group != Building_Group::Hospital) continue;

					menu_item_rect.x += menu_item_rect.w + button_margin;
					flags = immediate_image_button(menu_item_rect, ui.button_theme_buildings, building_type.thumbnail, __LINE__ * 10000 + i, false);
					if (flags & BUTTON_STATE_FLAGS_RELEASED)
					{
						ui.game_scene->SetConstructionMode(ConstructionMode::Building);
						auto mode = ui.game_scene->m_BuildingManager.GetConstructionMode();
						if (mode == BuildingConstructionMode::None || mode == BuildingConstructionMode::Destruct)
							ui.game_scene->m_BuildingManager.SetConstructionMode(BuildingConstructionMode::Construct);
						ui.game_scene->m_BuildingManager.SetType(i);
					}
				}
			}
			else if (ui.draw_building_panel_inside_type == Draw_Building_Panel::Police_Station)
			{
				flags = immediate_image_button(menu_item_rect, ui.button_theme_back, nullptr, __LINE__);
				menu_item_rect.z++;
				immediate_text("B", menu_item_rect, ui.label_theme_large_text);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
					ui.draw_building_panel_inside_type = Draw_Building_Panel::Special;
			}
			else if (ui.draw_building_panel_inside_type == Draw_Building_Panel::Garbage_Collection_Center)
			{
				flags = immediate_image_button(menu_item_rect, ui.button_theme_back, nullptr, __LINE__);
				menu_item_rect.z++;
				immediate_text("B", menu_item_rect, ui.label_theme_large_text);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
					ui.draw_building_panel_inside_type = Draw_Building_Panel::Special;
			}

			//immediate_end_sub_region(track_width);
		}
	}

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
		ui.game_scene = &game_scene;
		ui.game_scene_camera = &game_scene.camera_controller.camera;


		/*Label_Themes*/ {
			ui.label_theme_title.color = { 0.05f, 0.05f, 0.05f, 1.0f };
			ui.label_theme_title.font = buffer_data.default_font;
			ui.label_theme_title.font_size_in_pixel = 20;
			ui.label_theme_title.flags = FontFlags::LeftAligned;

			ui.label_theme_left_alinged_small_black_text.color = { 0.1f, 0.1f, 0.1f, 1.0f };
			ui.label_theme_left_alinged_small_black_text.font = buffer_data.default_font;
			ui.label_theme_left_alinged_small_black_text.font_size_in_pixel = 16;
			ui.label_theme_left_alinged_small_black_text.flags = FontFlags::LeftAligned;

			ui.label_theme_left_alinged_xsmall_black_text.color = { 0.1f, 0.1f, 0.1f, 1.0f };
			ui.label_theme_left_alinged_xsmall_black_text.font = buffer_data.default_font;
			ui.label_theme_left_alinged_xsmall_black_text.font_size_in_pixel = 14;
			ui.label_theme_left_alinged_xsmall_black_text.flags = FontFlags::LeftAligned;

			ui.label_theme_left_alinged_xsmall_red_text.color = { 1.0f, 0.3f, 0.2f, 1.0f };
			ui.label_theme_left_alinged_xsmall_red_text.font = buffer_data.default_font;
			ui.label_theme_left_alinged_xsmall_red_text.font_size_in_pixel = 14;
			ui.label_theme_left_alinged_xsmall_red_text.flags = FontFlags::LeftAligned;

			ui.label_theme_button.color = { 0.1f, 0.1f, 0.1f, 1.0f };
			ui.label_theme_button.font = buffer_data.default_font;
			ui.label_theme_button.font_size_in_pixel = 30;

			ui.label_theme_camera_mode_button.color = { 0.1f, 0.1f, 0.1f, 1.0f };
			ui.label_theme_camera_mode_button.font = buffer_data.default_font;
			ui.label_theme_camera_mode_button.font_size_in_pixel = 20;

			ui.label_theme_text.color = { 0.9f, 0.9f, 0.9f, 1.0f };
			ui.label_theme_text.font = buffer_data.default_font;
			ui.label_theme_text.font_size_in_pixel = 20;

			ui.label_theme_large_text.color = { 0.05f, 0.05f, 0.05f, 1.0f };
			ui.label_theme_large_text.font = buffer_data.default_font;
			ui.label_theme_large_text.font_size_in_pixel = 50;
		}
		/*Button_Themes*/ {
			ui.button_theme_cross.label_theme = &ui.label_theme_button;
			ui.button_theme_cross.background_color = { 0.80f, 0.50f, 0.30f, 1.0f };
			ui.button_theme_cross.background_color_over = { 0.90f, 0.70f, 0.55f, 1.0f };
			ui.button_theme_cross.background_color_pressed = { 0.95f, 0.85f, 0.70f, 1.0f };

			ui.button_theme_camera_mode.label_theme = &ui.label_theme_camera_mode_button;
			ui.button_theme_camera_mode.background_color = { 0.89f, 0.50f, 0.30f, 1.0f };
			ui.button_theme_camera_mode.background_color_over = { 0.96f, 0.70f, 0.55f, 1.0f };
			ui.button_theme_camera_mode.background_color_pressed = { 0.98f, 0.85f, 0.70f, 1.0f };

			ui.button_theme_thumb.label_theme = &ui.label_theme_button;
			ui.button_theme_thumb.background_color = { 0.80f, 0.50f, 0.30f, 1.0f };
			ui.button_theme_thumb.background_color_over = { 0.90f, 0.70f, 0.55f, 1.0f };
			ui.button_theme_thumb.background_color_pressed = { 0.95f, 0.85f, 0.70f, 1.0f };

			ui.button_theme_track.label_theme = &ui.label_theme_button;
			ui.button_theme_track.background_color = { 0.70f, 0.50f, 0.40f, 1.0f };
			ui.button_theme_track.background_color_over = { 0.80f, 0.70f, 0.60f, 1.0f };
			ui.button_theme_track.background_color_pressed = { 0.90f, 0.85f, 0.80f, 1.0f };

			ui.button_theme_back.label_theme = &ui.label_theme_button;
			ui.button_theme_back.background_color = { 0.60f, 0.20f, 0.30f, 1.0f };
			ui.button_theme_back.background_color_over = { 0.75f, 0.30f, 0.40f, 1.0f };
			ui.button_theme_back.background_color_pressed = { 0.90f, 0.40f, 0.50f, 1.0f };

			ui.button_theme_sub_menus.label_theme = &ui.label_theme_button;
			ui.button_theme_sub_menus.background_color = v4{ 0.90f, 0.90f, 0.90f, 1.0f };
			ui.button_theme_sub_menus.background_color_over = v4{ 0.95f, 0.95f, 0.95f, 1.0f };
			ui.button_theme_sub_menus.background_color_pressed = v4{ 1.00f, 1.00f, 1.00f, 1.0f };
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
			ui.selected_building = nullptr;
		}
		return false;
	}
	bool on_game_scene_ui_layer_mouse_pressed(void* p, Event::MouseButtonPressedEvent& event)
	{
		Game_Scene_UI& ui = *((Game_Scene_UI*)p);

		MouseCode key_code = event.GetMouseButton();
		if (key_code == MouseCode::Button0)
		{
		}
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
			v4 position_on_screen{ ui.game_scene_camera->view_projection * ui.focus_object->transform * v4(0.0f, 0.0f, ui.focus_object->prefab->boundingBoxM.z, 1.0f) };
			ui.rect_sub_region.x = glm::clamp(
				u32(glm::clamp(position_on_screen.x / position_on_screen.w + 1.0f, 0.0f, 2.0f) * width_in_pixels * 0.5f),
				0U,
				width_in_pixels - ui.rect_sub_region.w
			);
			ui.rect_sub_region.y = glm::clamp(
				u32(glm::clamp(position_on_screen.y / position_on_screen.w + 1.0f, 0.0f, 2.0f) * height_in_pixels * 0.5f) - ui.rect_sub_region.h,
				0U,
				height_in_pixels - ui.rect_sub_region.h
			);
		}

		if (ui.selected_building)
		{
			auto obj{ ui.selected_building->object };
			v4 position_on_screen{ ui.game_scene_camera->view_projection * obj->transform * v4(0.0f, 0.0f, obj->prefab->boundingBoxM.z, 1.0f) };
			ui.rect_selected_building_detail_panel.x = glm::clamp(
				(u32)(glm::clamp(position_on_screen.x / position_on_screen.w + 1.0f, 0.0f, 2.0f) * width_in_pixels * 0.5f),
				0U,
				width_in_pixels - ui.rect_selected_building_detail_panel.w
			);
			ui.rect_selected_building_detail_panel.y = glm::clamp(
				(u32)(glm::clamp(position_on_screen.y / position_on_screen.w + 1.0f, 0.0f, 2.0f) * height_in_pixels * 0.5f) - ui.rect_selected_building_detail_panel.h,
				0U,
				height_in_pixels - ui.rect_selected_building_detail_panel.h
			);
			ui.rect_selected_building_detail_panel_background.x = ui.rect_selected_building_detail_panel.x - 1;
			ui.rect_selected_building_detail_panel_background.y = ui.rect_selected_building_detail_panel.y - 1;
		}
	}

	void draw_screen(Game_Scene_UI& ui)
	{
		auto app{ GameApp::instance };
		auto& building_types{ app->building_types };
		const std::string text_x{ "X" };
		if (ui.focus_object != nullptr)
		{
			Rect rect_button_cross;
			rect_button_cross.w = 40;
			rect_button_cross.h = 40;
			rect_button_cross.x = ui.rect_sub_region.x + ui.rect_sub_region.w - rect_button_cross.w;
			rect_button_cross.y = ui.rect_sub_region.y + ui.rect_sub_region.h - rect_button_cross.h;
			rect_button_cross.z = 1;

			Rect rect_button_fpc;
			rect_button_fpc.w = 60;
			rect_button_fpc.h = 30;
			rect_button_fpc.x = ui.rect_sub_region.x + 10;
			rect_button_fpc.y = ui.rect_sub_region.y + ui.rect_sub_region.h - rect_button_fpc.h - 10;
			rect_button_fpc.z = 1;

			Rect rect_button_tpc;
			rect_button_tpc.w = rect_button_fpc.w;
			rect_button_tpc.h = rect_button_fpc.h;
			rect_button_tpc.x = rect_button_fpc.x + rect_button_fpc.w + 10;
			rect_button_tpc.y = rect_button_fpc.y;
			rect_button_tpc.z = rect_button_fpc.z;


			std::string text_fpc = "FPC";
			std::string text_tpc = "TPC";
			//immediate_begin_sub_region(rect_sub_region, ui.sub_region_theme_details, __LINE__);
			immediate_quad(ui.rect_sub_region, ui.sub_region_theme_details.background_color);

			u16 flags = immediate_button(rect_button_cross, text_x, ui.button_theme_cross, __LINE__, true);
			if (flags & BUTTON_STATE_FLAGS_PRESSED)
				std::cout << "Close is Pressed\n";
			if (flags & BUTTON_STATE_FLAGS_RELEASED)
			{
				std::cout << "Close is Released\n";
				ui.focus_object = nullptr;
				return;
			}

			flags = immediate_button(rect_button_fpc, text_fpc, ui.button_theme_camera_mode, __LINE__, true);
			if (flags & BUTTON_STATE_FLAGS_PRESSED)
				std::cout << "FPC is Pressed\n";
			if (flags & BUTTON_STATE_FLAGS_RELEASED)
			{
				std::cout << "FPC is Released\n";
				//set mode to FPC
				ui.game_scene->camera_controller.follow_object = ui.focus_object;
				ui.game_scene->camera_controller.mode = Mode::FollowFirstPerson;
				ui.focus_object = nullptr;
				return;
			}

			flags = immediate_button(rect_button_tpc, text_tpc, ui.button_theme_camera_mode, __LINE__, true);
			if (flags & BUTTON_STATE_FLAGS_PRESSED)
				std::cout << "TPC is Pressed\n";
			if (flags & BUTTON_STATE_FLAGS_RELEASED)
			{
				std::cout << "TPC is Released\n";
				//set mode to TPC
				ui.game_scene->camera_controller.follow_object = ui.focus_object;
				ui.game_scene->camera_controller.mode = Mode::FollowThirdPerson;
				ui.focus_object = nullptr;
				return;
			}

			//immediate_end_sub_region(track_width);
		}

		if (ui.selected_building != nullptr)
		{
			auto& building{ ui.selected_building };
			auto& building_type{ building_types[building->type] };

			constexpr s32 title_left_margin{ 10 };

			constexpr v4 color_white{ 0.9f, 0.9f, 0.9f, 1.0f };
			constexpr v4 color_black{ 0.1f, 0.1f, 0.1f, 1.0f };
			constexpr v4 color_red{ 1.0f, 0.1f, 0.2f, 1.0f };
			constexpr v4 color_green{ 0.1f, 1.0f, 0.2f, 1.0f };

			constexpr v4 color_very_happy{ 39.0f / 255.0f, 167.0f / 255.0f, 56.0f / 255.0f, 1.0f };
			constexpr v4 color_happy{ 188.0f / 255.0f, 235.0f / 255.0f, 0.0f / 255.0f, 1.0f };
			constexpr v4 color_moderately_happy{ 255.0f / 255.0f, 210.0f / 255.0f, 0.0f / 255.0f, 1.0f };
			constexpr v4 color_unhappy{ 254.0f / 255.0f, 147.0f / 255.0f, 7.0f / 255.0f, 1.0f };
			constexpr v4 color_angry{ 228.0f / 255.0f, 6.0f / 255.0f, 19.0f / 255.0f, 1.0f };

			const std::string total_occupants_key{ "Total Occupants" };
			const std::string total_workers_key{ "Total Workers" };

			const std::string uneducated_key{ "Uneducated" };
			const std::string elementary_school_key{ "Elementary School" };
			const std::string high_school_key{ "High School" };
			const std::string associate_s_key{ "Associate's" };
			const std::string bachelor_s_key{ "Bachelor's" };
			const std::string master_key{ "Master" };
			const std::string doctorate_key{ "Doctorate" };

			const std::string employment_key{ "Employment" };
			const std::string currently_working_key{ "Currently Working" };
			const std::string working_distribution_key{ "(Driving)/(At Building)" };
			const std::string work_car_used_key{ "Work Car Used" };
			const std::string total_crime_reported_key{ "Total Crime Reported" };
			const std::string occupants_key{ "Occupants" };
			const std::string workers_key{ "Workers" };
			const std::string health_key{ "Health" };
			const std::string homes_health_avg_key{ "Homes Health Avg" };
			const std::string work_places_health_key{ "Work Places Health Avg" };
			const std::string homes_garbage_key{ "Homes Garbage" };
			const std::string work_places_garbage_avg_key{ "Work Places Garbage" };
			const std::string electricity_key{ "Electricity" };
			const std::string garbage_key{ "Garbage" };
			const std::string garbage_capacity_key{ "Garbage Capacity" };
			const std::string water_key{ "Water" };
			const std::string water_waste_key{ "Water Waste" };
			const std::string police_key{ "Police" };
			const std::string happiness_key{ "Happiness" };
			const std::string patients_key{ "Patients" };
			const std::string inpatients_key{ "Inpatients" };
			const std::string complainants_key{ "Complainants" };
			const std::string prisoners_key{ "Prisoners" };

			Rect rect_button_cross;
			rect_button_cross.w = 40;
			rect_button_cross.h = 40;
			rect_button_cross.x = ui.rect_selected_building_detail_panel.x + ui.rect_selected_building_detail_panel.w - rect_button_cross.w;
			rect_button_cross.y = ui.rect_selected_building_detail_panel.y + ui.rect_selected_building_detail_panel.h - rect_button_cross.h;
			rect_button_cross.z = ui.rect_selected_building_detail_panel.z + 1;

			immediate_quad(ui.rect_selected_building_detail_panel_background, color_black);
			immediate_quad(ui.rect_selected_building_detail_panel, ui.sub_region_theme_details.background_color);
			u16 flags{ 0 };
			flags = immediate_button(rect_button_cross, text_x, ui.button_theme_cross, __LINE__, true);
			if (flags & BUTTON_STATE_FLAGS_PRESSED)
				std::cout << "Close is Pressed\n";
			if (flags & BUTTON_STATE_FLAGS_RELEASED)
			{
				std::cout << "Close is Released\n";
				building = nullptr;
				return;
			}

			Rect rect_building_name;
			rect_building_name.w = ui.rect_selected_building_detail_panel.w - title_left_margin;
			rect_building_name.h = 40;
			rect_building_name.x = ui.rect_selected_building_detail_panel.x + title_left_margin;
			rect_building_name.y = ui.rect_selected_building_detail_panel.y + ui.rect_selected_building_detail_panel.h - rect_building_name.h;
			rect_building_name.z = ui.rect_selected_building_detail_panel.z + 1;

			Rect rect_key;
			rect_key.w = 150;
			rect_key.h = 20;
			rect_key.x = ui.rect_selected_building_detail_panel.x + title_left_margin;
			rect_key.y = rect_building_name.y - (rect_key.h + 20);
			rect_key.z = ui.rect_selected_building_detail_panel.z + 1;

			Rect rect_value;
			rect_value.w = 50;
			rect_value.h = rect_key.h;
			rect_value.x = rect_key.x + rect_key.w;
			rect_value.y = rect_key.y;
			rect_value.z = rect_key.z;

			Rect rect_needs_key;
			rect_needs_key.w = 150;
			rect_needs_key.h = 20;
			rect_needs_key.x = ui.rect_selected_building_detail_panel.x + ui.rect_selected_building_detail_panel.w * 0.5 + title_left_margin;
			rect_needs_key.y = rect_key.y;
			rect_needs_key.z = rect_key.z;

			Rect rect_needs_value;
			rect_needs_value.x = rect_needs_key.x + rect_needs_key.w;
			rect_needs_value.y = rect_needs_key.y;
			rect_needs_value.z = rect_needs_key.z;
			rect_needs_value.w = ui.rect_selected_building_detail_panel.w - (rect_needs_value.x - ui.rect_selected_building_detail_panel.x) - 50;
			rect_needs_value.h = rect_needs_key.h;

			Rect rect_needs_value_inside;
			rect_needs_value_inside.x = rect_needs_value.x + 1;
			rect_needs_value_inside.y = rect_needs_value.y + 1;
			rect_needs_value_inside.z = rect_needs_value.z + 1;
			rect_needs_value_inside.w = rect_needs_value.w - 2;
			rect_needs_value_inside.h = rect_needs_value.h - 2;

			Rect rect_needs_value_inside_positive;
			rect_needs_value_inside_positive.x = rect_needs_value_inside.x;
			rect_needs_value_inside_positive.y = rect_needs_value_inside.y;
			rect_needs_value_inside_positive.z = rect_needs_value_inside.z + 1;
			rect_needs_value_inside_positive.w = rect_needs_value_inside.w;
			rect_needs_value_inside_positive.h = rect_needs_value_inside.h;

			Rect rect_left_needs_key;
			rect_left_needs_key.w = 200;
			rect_left_needs_key.h = 20;
			rect_left_needs_key.x = ui.rect_selected_building_detail_panel.x + title_left_margin;
			rect_left_needs_key.y = rect_key.y;
			rect_left_needs_key.z = rect_key.z;

			Rect rect_left_needs_value;
			rect_left_needs_value.x = rect_left_needs_key.x + rect_left_needs_key.w;
			rect_left_needs_value.y = rect_left_needs_key.y;
			rect_left_needs_value.z = rect_left_needs_key.z;
			rect_left_needs_value.w = (s32)((f32)ui.rect_selected_building_detail_panel.w * 0.5f) - (rect_left_needs_value.x - ui.rect_selected_building_detail_panel.x) - 5;
			rect_left_needs_value.h = rect_left_needs_key.h;

			Rect rect_left_needs_value_inside;
			rect_left_needs_value_inside.x = rect_left_needs_value.x + 1;
			rect_left_needs_value_inside.y = rect_left_needs_value.y + 1;
			rect_left_needs_value_inside.z = rect_left_needs_value.z + 1;
			rect_left_needs_value_inside.w = rect_left_needs_value.w - 2;
			rect_left_needs_value_inside.h = rect_left_needs_value.h - 2;

			Rect rect_left_needs_value_inside_positive;
			rect_left_needs_value_inside_positive.x = rect_left_needs_value_inside.x;
			rect_left_needs_value_inside_positive.y = rect_left_needs_value_inside.y;
			rect_left_needs_value_inside_positive.z = rect_left_needs_value_inside.z + 1;
			rect_left_needs_value_inside_positive.w = rect_left_needs_value_inside.w;
			rect_left_needs_value_inside_positive.h = rect_left_needs_value_inside.h;

			immediate_text(building->name, rect_building_name, ui.label_theme_title);

			/*left panel*/ {

				if (building_type.group == Building_Group::House)
				{
					auto& occupants{ building->people };
					auto total_occupants_value{ std::format(": {}/{}", occupants.size(), building_type.capacity) };
					immediate_text(total_occupants_key, rect_key, ui.label_theme_left_alinged_small_black_text);
					immediate_text(total_occupants_value, rect_value, ui.label_theme_left_alinged_small_black_text);

					rect_key.y -= rect_key.h + 20;
					rect_value.y = rect_key.y;
					auto has_job{ 0 };
					for (auto occupant : occupants)
						if (occupant->work)
							++has_job;
					auto employment_value{ std::format(": {}/{}", has_job, occupants.size()) };
					immediate_text(employment_key, rect_key, ui.label_theme_left_alinged_small_black_text);
					immediate_text(employment_value, rect_value, ui.label_theme_left_alinged_small_black_text);


					rect_key.y -= rect_key.h + 10;
					rect_value.y = rect_key.y;
					immediate_text(occupants_key, rect_key, ui.label_theme_left_alinged_small_black_text);

					Rect rect_gender;
					rect_gender.w = 20;
					rect_gender.h = rect_gender.w;
					rect_gender.x = rect_key.x;
					rect_gender.y = rect_key.y - (rect_key.h + 5);
					rect_gender.z = rect_key.z;

					Rect rect_name;
					rect_name.w = 150;
					rect_name.h = rect_gender.h;
					rect_name.x = rect_gender.x + rect_gender.w + 5;
					rect_name.y = rect_gender.y;
					rect_name.z = rect_gender.z;
					for (auto occupant : occupants)
					{
						immediate_image(rect_gender, GameApp::instance->cancelTexture);
						immediate_text(occupant->firstName, rect_name, ui.label_theme_left_alinged_xsmall_black_text);
						rect_gender.y -= rect_gender.h + 5;
						rect_name.y = rect_gender.y;
					}
				}
				else if (building_type.group == Building_Group::Hospital)
				{
					auto& buildings{ ui.game_scene->m_BuildingManager.m_Buildings };

					u16 used_work_cars{ 5 };
					u16 total_work_cars{ 15 };
					auto work_car_used_value{ std::format(": {}/{}", used_work_cars, total_work_cars) };
					immediate_text(work_car_used_key, rect_left_needs_key, ui.label_theme_left_alinged_small_black_text);
					immediate_text(work_car_used_value, rect_left_needs_value, ui.label_theme_left_alinged_small_black_text);

					rect_key.y -= rect_key.h + 5;
					rect_value.y = rect_key.y;
					rect_left_needs_key.y = rect_key.y;
					rect_left_needs_value.y = rect_key.y;
					rect_left_needs_value_inside.y = rect_key.y + 1;
					rect_left_needs_value_inside_positive.y = rect_key.y + 1;

					u32 total_homes{ 0 };
					u32 total_work_places{ 0 };
					f32 total_health_homes{ 0 };
					f32 total_health_work_places{ 0 };
					for (auto b : buildings)
					{
						if (building_type.group == Building_Group::House || building_type.group == Building_Group::Residential)
						{
							++total_homes;
							total_health_homes += b->current_health / b->max_health;
						}
						else
						{
							++total_work_places;
							total_health_work_places += b->current_health / b->max_health;
						}
					}

					f32 ratio{ total_health_homes / total_homes };
					v4 color_homes_health_avg{ Math::lerp(color_red, color_green, ratio) };
					rect_left_needs_value_inside_positive.w = (s32)((f32)(rect_left_needs_value.w - 2) * ratio);
					immediate_text(homes_health_avg_key, rect_left_needs_key, ui.label_theme_left_alinged_small_black_text);
					immediate_quad(rect_left_needs_value, color_black);
					immediate_quad(rect_left_needs_value_inside, color_white);
					immediate_quad(rect_left_needs_value_inside_positive, color_homes_health_avg);

					rect_key.y -= rect_key.h + 5;
					rect_value.y = rect_key.y;
					rect_left_needs_key.y = rect_key.y;
					rect_left_needs_value.y = rect_key.y;
					rect_left_needs_value_inside.y = rect_key.y + 1;
					rect_left_needs_value_inside_positive.y = rect_key.y + 1;

					ratio = total_health_work_places / total_work_places;
					v4 color_work_places_health_avg{ Math::lerp(color_red, color_green, ratio) };
					rect_left_needs_value_inside_positive.w = (s32)((f32)(rect_left_needs_value.w - 2) * ratio);
					immediate_text(work_places_health_key, rect_left_needs_key, ui.label_theme_left_alinged_small_black_text);
					immediate_quad(rect_left_needs_value, color_black);
					immediate_quad(rect_left_needs_value_inside, color_white);
					immediate_quad(rect_left_needs_value_inside_positive, color_work_places_health_avg);

					rect_left_needs_key.y -= rect_left_needs_key.h + 10;
					rect_left_needs_value.y = rect_left_needs_key.y;

					auto patients_value{ std::format(": {}/{}", building->visitors.size(), building_type.visitor_capacity) };
					immediate_text(patients_key, rect_left_needs_key, ui.label_theme_left_alinged_small_black_text);
					immediate_text(patients_value, rect_left_needs_value, ui.label_theme_left_alinged_small_black_text);

					rect_left_needs_key.y -= rect_left_needs_key.h + 10;
					rect_left_needs_value.y = rect_left_needs_key.y;

					auto inpatients_value{ std::format(": {}/{}", building->visitors.size(), building_type.stay_visitor_capacity) };
					immediate_text(inpatients_key, rect_left_needs_key, ui.label_theme_left_alinged_small_black_text);
					immediate_text(inpatients_value, rect_left_needs_value, ui.label_theme_left_alinged_small_black_text);
				}
				else if (building_type.group == Building_Group::Police_Station)
				{
					auto& buildings{ ui.game_scene->m_BuildingManager.m_Buildings };

					u16 used_work_cars{ 5 };
					u16 total_work_cars{ 15 };
					auto work_car_used_value{ std::format(": {}/{}", used_work_cars, total_work_cars) };
					immediate_text(work_car_used_key, rect_left_needs_key, ui.label_theme_left_alinged_small_black_text);
					immediate_text(work_car_used_value, rect_left_needs_value, ui.label_theme_left_alinged_small_black_text);

					rect_left_needs_key.y -= rect_left_needs_key.h + 5;
					rect_left_needs_value.y = rect_left_needs_key.y;

					u32 total_crime_reported{ 0 };
					for (auto b : buildings)
						total_crime_reported += b->crime_reported;

					auto total_crime_reported_value{ std::format(": {}", total_crime_reported) };
					immediate_text(total_crime_reported_key, rect_left_needs_key, ui.label_theme_left_alinged_small_black_text);
					immediate_text(total_crime_reported_value, rect_left_needs_value, ui.label_theme_left_alinged_small_black_text);

					rect_left_needs_key.y -= rect_left_needs_key.h + 10;
					rect_left_needs_value.y = rect_left_needs_key.y;

					auto complainants_value{ std::format(": {}/{}", building->visitors.size(), building_type.visitor_capacity) };
					immediate_text(complainants_key, rect_left_needs_key, ui.label_theme_left_alinged_small_black_text);
					immediate_text(complainants_value, rect_left_needs_value, ui.label_theme_left_alinged_small_black_text);

					rect_left_needs_key.y -= rect_left_needs_key.h + 10;
					rect_left_needs_value.y = rect_left_needs_key.y;

					auto prisoners_value{ std::format(": {}/{}", building->visitors.size(), building_type.stay_visitor_capacity) };
					immediate_text(prisoners_key, rect_left_needs_key, ui.label_theme_left_alinged_small_black_text);
					immediate_text(prisoners_value, rect_left_needs_value, ui.label_theme_left_alinged_small_black_text);
				}
				else if (building_type.group == Building_Group::Garbage_Collection_Center)
				{
					auto& buildings{ ui.game_scene->m_BuildingManager.m_Buildings };

					u16 used_work_cars{ 5 };
					u16 total_work_cars{ 15 };
					auto work_car_used_value{ std::format(": {}/{}", used_work_cars, total_work_cars) };
					immediate_text(work_car_used_key, rect_left_needs_key, ui.label_theme_left_alinged_small_black_text);
					immediate_text(work_car_used_value, rect_left_needs_value, ui.label_theme_left_alinged_small_black_text);

					rect_left_needs_key.y -= rect_left_needs_key.h + 5;
					rect_left_needs_value.y = rect_left_needs_key.y;
					rect_left_needs_value_inside.y = rect_left_needs_key.y + 1;
					rect_left_needs_value_inside_positive.y = rect_left_needs_key.y + 1;

					u32 total_homes{ 0 };
					u32 total_work_places{ 0 };
					f32 total_garbage_homes{ 0 };
					f32 total_garbage_work_places{ 0 };
					for (auto b : buildings)
					{
						if (building_type.group == Building_Group::House)
						{
							++total_homes;
							total_garbage_homes += b->current_garbage / b->garbage_capacity;
						}
						else
						{
							++total_work_places;
							total_garbage_work_places += b->current_garbage / b->garbage_capacity;
						}
					}

					f32 ratio{ total_garbage_homes / total_homes };
					v4 color_homes_garbage_avg{ Math::lerp(color_red, color_green, ratio) };
					rect_left_needs_value_inside_positive.w = (s32)((f32)(rect_left_needs_value.w - 2) * ratio);
					immediate_text(homes_garbage_key, rect_left_needs_key, ui.label_theme_left_alinged_small_black_text);
					immediate_quad(rect_left_needs_value, color_black);
					immediate_quad(rect_left_needs_value_inside, color_white);
					immediate_quad(rect_left_needs_value_inside_positive, color_homes_garbage_avg);

					rect_left_needs_key.y -= rect_left_needs_key.h + 5;
					rect_left_needs_value.y = rect_left_needs_key.y;
					rect_left_needs_value_inside.y = rect_left_needs_key.y + 1;
					rect_left_needs_value_inside_positive.y = rect_left_needs_key.y + 1;

					ratio = total_garbage_work_places / total_work_places;
					v4 color_work_places_garbage_avg{ Math::lerp(color_red, color_green, ratio) };
					rect_left_needs_value_inside_positive.w = (s32)((f32)(rect_left_needs_value.w - 2) * ratio);
					immediate_text(work_places_garbage_avg_key, rect_left_needs_key, ui.label_theme_left_alinged_small_black_text);
					immediate_quad(rect_left_needs_value, color_black);
					immediate_quad(rect_left_needs_value_inside, color_white);
					immediate_quad(rect_left_needs_value_inside_positive, color_work_places_garbage_avg);

					rect_left_needs_key.y -= rect_left_needs_key.h + 10;
					rect_left_needs_value.y = rect_left_needs_key.y;
					rect_left_needs_value_inside.y = rect_left_needs_key.y + 1;
					rect_left_needs_value_inside_positive.y = rect_left_needs_key.y + 1;

					f32 used_garbage_capacity{ 5.2f };
					f32 garbage_capacity{ 15.0f };
					auto garbage_capacity_value{ std::format(": {} t / {} t", used_garbage_capacity, garbage_capacity) };
					immediate_text(garbage_capacity_key, rect_left_needs_key, ui.label_theme_left_alinged_small_black_text);
					immediate_text(garbage_capacity_value, rect_left_needs_value, ui.label_theme_left_alinged_small_black_text);


					rect_left_needs_key.y -= rect_left_needs_key.h + 5;
					rect_left_needs_value.y = rect_left_needs_key.y;
					rect_left_needs_value_inside.y = rect_left_needs_key.y + 1;
					rect_left_needs_value_inside_positive.y = rect_left_needs_key.y + 1;

					ratio = used_garbage_capacity / garbage_capacity;
					v4 color_garbage_usage{ Math::lerp(color_green, color_red, ratio) };
					rect_left_needs_value_inside_positive.w = (s32)((f32)(rect_left_needs_value.w - 2) * ratio);
					immediate_text(garbage_capacity_key, rect_left_needs_key, ui.label_theme_left_alinged_small_black_text);
					immediate_quad(rect_left_needs_value, color_black);
					immediate_quad(rect_left_needs_value_inside, color_white);
					immediate_quad(rect_left_needs_value_inside_positive, color_garbage_usage);
				}
				else
				{
					auto& workers{ building->people };
					rect_key.w = 200;
					rect_value.x = rect_key.x + rect_key.w;

					u16 working_uneducated{ 0 };
					u16 working_elementary_school{ 0 };
					u16 working_high_school{ 0 };
					u16 working_associate_s{ 0 };
					u16 working_bachelor_s{ 0 };
					u16 working_master{ 0 };
					u16 working_doctorate{ 0 };
					for (auto worker : workers)
					{
						switch (worker->education)
						{
						case PersonEducationLevel::Uneducated:
						{
							++working_uneducated;
							break;
						}
						case PersonEducationLevel::Elementary_School:
						{
							++working_elementary_school;
							break;
						}
						case PersonEducationLevel::High_School:
						{
							++working_high_school;
							break;
						}
						case PersonEducationLevel::Associate_s:
						{
							++working_associate_s;
							break;
						}
						case PersonEducationLevel::Bachelor_s:
						{
							++working_bachelor_s;
							break;
						}
						case PersonEducationLevel::Master:
						{
							++working_master;
							break;
						}
						case PersonEducationLevel::Doctorate:
						{
							++working_doctorate;
							break;
						}
						default:
							assert(false, "Unimplemented PersonEducationLevel");
							break;
						}
					}
					auto uneducated_value{ std::format(": {}/{}", working_uneducated, building_type.needed_uneducated) };
					immediate_text(uneducated_key, rect_key, ui.label_theme_left_alinged_small_black_text);
					immediate_text(uneducated_value, rect_value, ui.label_theme_left_alinged_small_black_text);

					rect_key.y -= rect_key.h + 5;
					rect_value.y = rect_key.y;

					auto elementary_school_value{ std::format(": {}/{}", working_elementary_school, building_type.needed_elementary_school) };
					immediate_text(elementary_school_key, rect_key, ui.label_theme_left_alinged_small_black_text);
					immediate_text(elementary_school_value, rect_value, ui.label_theme_left_alinged_small_black_text);

					rect_key.y -= rect_key.h + 5;
					rect_value.y = rect_key.y;

					auto high_school_value{ std::format(": {}/{}", working_high_school, building_type.needed_high_school) };
					immediate_text(high_school_key, rect_key, ui.label_theme_left_alinged_small_black_text);
					immediate_text(high_school_value, rect_value, ui.label_theme_left_alinged_small_black_text);

					rect_key.y -= rect_key.h + 5;
					rect_value.y = rect_key.y;

					auto associate_s_value{ std::format(": {}/{}", working_associate_s, building_type.needed_associate_s) };
					immediate_text(associate_s_key, rect_key, ui.label_theme_left_alinged_small_black_text);
					immediate_text(associate_s_value, rect_value, ui.label_theme_left_alinged_small_black_text);

					rect_key.y -= rect_key.h + 5;
					rect_value.y = rect_key.y;

					auto bachelor_s_value{ std::format(": {}/{}", working_bachelor_s, building_type.needed_bachelor_s) };
					immediate_text(bachelor_s_key, rect_key, ui.label_theme_left_alinged_small_black_text);
					immediate_text(bachelor_s_value, rect_value, ui.label_theme_left_alinged_small_black_text);

					rect_key.y -= rect_key.h + 5;
					rect_value.y = rect_key.y;

					auto master_value{ std::format(": {}/{}", working_master, building_type.needed_master) };
					immediate_text(master_key, rect_key, ui.label_theme_left_alinged_small_black_text);
					immediate_text(master_value, rect_value, ui.label_theme_left_alinged_small_black_text);

					rect_key.y -= rect_key.h + 5;
					rect_value.y = rect_key.y;

					auto doctorate_value{ std::format(": {}/{}", working_doctorate, building_type.needed_doctorate) };
					immediate_text(doctorate_key, rect_key, ui.label_theme_left_alinged_small_black_text);
					immediate_text(doctorate_value, rect_value, ui.label_theme_left_alinged_small_black_text);

					rect_key.y -= rect_key.h + 10;
					rect_value.y = rect_key.y;

					auto total_workers_value{ std::format(": {}/{}", workers.size(), building_type.capacity) };
					immediate_text(total_workers_key, rect_key, ui.label_theme_left_alinged_small_black_text);
					immediate_text(total_workers_value, rect_value, ui.label_theme_left_alinged_small_black_text);

					rect_key.y -= rect_key.h + 5;
					rect_value.y = rect_key.y;

					auto currently_at_work_building{ 0 };
					auto currently_driving_for_work{ 0 };
					auto currently_working{ 0 };
					for (auto worker : workers)
					{
						// TODO: also walking to work car and comming back to work place from work car
						if (worker->status == PersonStatus::AtWork)
						{
							++currently_at_work_building;
							++currently_working;
						}
						else if (worker->status == PersonStatus::DrivingForWork)
						{
							++currently_driving_for_work;
							++currently_working;
						}
					}
					auto currently_working_value{ std::format(": {}/{}", currently_working, workers.size()) };
					immediate_text(currently_working_key, rect_key, ui.label_theme_left_alinged_small_black_text);
					immediate_text(currently_working_value, rect_value, ui.label_theme_left_alinged_small_black_text);

					rect_key.y -= rect_key.h + 5;
					rect_value.y = rect_key.y;

					u16 used_work_cars = 5;
					u16 total_work_cars = 15;
					auto work_car_used_value{ std::format(": {}/{}", used_work_cars, total_work_cars) };
					immediate_text(work_car_used_key, rect_key, ui.label_theme_left_alinged_small_black_text);
					immediate_text(work_car_used_value, rect_value, ui.label_theme_left_alinged_small_black_text);

					rect_key.y -= rect_key.h + 5;
					rect_value.y = rect_key.y;

					auto working_distribution_value{ std::format(": {}/{}", currently_at_work_building, currently_driving_for_work) };
					immediate_text(working_distribution_key, rect_key, ui.label_theme_left_alinged_small_black_text);
					immediate_text(working_distribution_value, rect_value, ui.label_theme_left_alinged_small_black_text);


					rect_key.y -= rect_key.h + 10;
					rect_value.y = rect_key.y;
					immediate_text(workers_key, rect_key, ui.label_theme_left_alinged_small_black_text);

					Rect rect_status;
					rect_status.w = 20;
					rect_status.h = rect_status.w;
					rect_status.x = rect_key.x;
					rect_status.y = rect_key.y - (rect_key.h + 5);
					rect_status.z = rect_key.z;

					Rect rect_name;
					rect_name.w = 150;
					rect_name.h = rect_status.h;
					rect_name.x = rect_status.x + rect_status.w + 5;
					rect_name.y = rect_status.y;
					rect_name.z = rect_status.z;
					for (auto worker : workers)
					{
						switch (worker->status)
						{
						case PersonStatus::AtHome:
						case PersonStatus::Walking:
						case PersonStatus::WalkingDead:
						case PersonStatus::Driving:
						{
							immediate_image(rect_status, GameApp::instance->cancelTexture);
							break;
						}
						case PersonStatus::AtWork:
						case PersonStatus::DrivingForWork:
						{
							immediate_tinted_image(rect_status, GameApp::instance->changeTexture, color_green);
							break;
						}
						default:
							assert(false, "Unimplemented PlayerStatus");
							break;
						}
						immediate_text(worker->firstName, rect_name, ui.label_theme_left_alinged_xsmall_black_text);
						rect_status.y -= rect_status.h + 5;
						rect_name.y = rect_status.y;
					}
				}
			}
			/*right panel*/ {
				auto& occupants{ building->people };
				// Health
				f32 ratio{};
				if (building_type.group != Building_Group::Hospital)
				{
					ratio = building->current_health / building->max_health;
					v4 color_health{ Math::lerp(color_red, color_green, ratio) };
					rect_needs_value_inside_positive.w = (s32)((f32)(rect_needs_value.w - 2) * ratio);
					immediate_text(health_key, rect_needs_key, ui.label_theme_left_alinged_small_black_text);
					immediate_quad(rect_needs_value, color_black);
					immediate_quad(rect_needs_value_inside, color_white);
					immediate_quad(rect_needs_value_inside_positive, color_health);
				}
				else
				{
					rect_needs_key.y += rect_needs_key.h + 20;
				}

				// Electric
				rect_needs_key.y -= rect_needs_key.h + 20;
				rect_needs_value.y = rect_needs_key.y;
				rect_needs_value_inside.y = rect_needs_key.y + 1;
				rect_needs_value_inside_positive.y = rect_needs_key.y + 1;

				auto& electricity_need{ building->electricity_need };
				auto& electricity_provided{ building->electricity_provided };
				const auto electricity_value{ std::format("{} kwh / {} kwh", electricity_need, electricity_provided) };
				immediate_text(electricity_key, rect_needs_key, ui.label_theme_left_alinged_small_black_text);
				if (electricity_need <= electricity_provided)
					immediate_text(electricity_value, rect_needs_value, ui.label_theme_left_alinged_xsmall_black_text);
				else
					immediate_text(electricity_value, rect_needs_value, ui.label_theme_left_alinged_xsmall_red_text);

				// Garbage
				if (building_type.group != Building_Group::Garbage_Collection_Center)
				{
					rect_needs_key.y -= rect_needs_key.h + 20;
					rect_needs_value.y = rect_needs_key.y;
					rect_needs_value_inside.y = rect_needs_key.y + 1;
					rect_needs_value_inside_positive.y = rect_needs_key.y + 1;

					ratio = building->current_garbage / building->garbage_capacity;
					v4 color_garbage{ Math::lerp(color_green, color_red, ratio) };
					rect_needs_value_inside_positive.w = (s32)((f32)(rect_needs_value.w - 2) * ratio);
					immediate_text(garbage_key, rect_needs_key, ui.label_theme_left_alinged_small_black_text);
					immediate_quad(rect_needs_value, color_black);
					immediate_quad(rect_needs_value_inside, color_white);
					immediate_quad(rect_needs_value_inside_positive, color_garbage);
				}

				// Water
				rect_needs_key.y -= rect_needs_key.h + 20;
				rect_needs_value.y = rect_needs_key.y;
				rect_needs_value_inside.y = rect_needs_key.y + 1;
				rect_needs_value_inside_positive.y = rect_needs_key.y + 1;

				auto& water_need{ building->water_need };
				auto& water_provided{ building->water_provided };
				const auto water_value{ std::format("{} lpd / {} lpd", water_need, water_provided) };
				immediate_text(water_key, rect_needs_key, ui.label_theme_left_alinged_small_black_text);
				if (water_need <= water_provided)
					immediate_text(water_value, rect_needs_value, ui.label_theme_left_alinged_xsmall_black_text);
				else
					immediate_text(water_value, rect_needs_value, ui.label_theme_left_alinged_xsmall_red_text);

				// Water Waste
				rect_needs_key.y -= rect_needs_key.h + 20;
				rect_needs_value.y = rect_needs_key.y;
				rect_needs_value_inside.y = rect_needs_key.y + 1;
				rect_needs_value_inside_positive.y = rect_needs_key.y + 1;

				auto& water_waste_need{ building->water_waste_need };
				auto& water_waste_provided{ building->water_waste_provided };
				const auto water_waste_value{ std::format("{} lpd / {} lpd", water_waste_need, water_waste_provided) };
				immediate_text(water_waste_key, rect_needs_key, ui.label_theme_left_alinged_small_black_text);
				if (water_waste_need <= water_waste_provided)
					immediate_text(water_waste_value, rect_needs_value, ui.label_theme_left_alinged_xsmall_black_text);
				else
					immediate_text(water_waste_value, rect_needs_value, ui.label_theme_left_alinged_xsmall_red_text);

				// Police
				if (building_type.group != Building_Group::Police_Station)
				{
					rect_needs_key.y -= rect_needs_key.h + 20;
					rect_needs_value.y = rect_needs_key.y;
					rect_needs_value_inside.y = rect_needs_key.y + 1;
					rect_needs_value_inside_positive.y = rect_needs_key.y + 1;

					const auto police_value{ std::format("{} crime reported", building->crime_reported) };
					immediate_text(police_key, rect_needs_key, ui.label_theme_left_alinged_small_black_text);
					if (building->crime_reported > 0)
						immediate_text(police_value, rect_needs_value, ui.label_theme_left_alinged_xsmall_red_text);
					else
						immediate_text(police_value, rect_needs_value, ui.label_theme_left_alinged_xsmall_black_text);
				}

				// Happiness
				rect_needs_key.y -= rect_needs_key.h + 20;
				rect_needs_value.y = rect_needs_key.y;
				rect_needs_value_inside.y = rect_needs_key.y + 1;
				rect_needs_value_inside_positive.y = rect_needs_key.y + 1;
				rect_needs_value_inside_positive.x = rect_needs_value_inside.x + rect_needs_value_inside.w - rect_needs_value_inside_positive.h;

				immediate_text(happiness_key, rect_needs_key, ui.label_theme_left_alinged_small_black_text);
				f32 happiness = 0.0f;
				for (auto occupant : occupants)
					happiness += occupant->happiness;
				happiness /= occupants.size() == 0 ? 1 : occupants.size();
				if (happiness > 0.8f)
				{
					std::string happiness_value{ "Very happy!!" };
					immediate_text(happiness_value, rect_needs_value_inside, ui.label_theme_left_alinged_small_black_text);
					immediate_quad(rect_needs_value_inside_positive, color_very_happy);
				}
				else if (happiness > 0.6f)
				{
					std::string happiness_value{ "Happy!" };
					immediate_text(happiness_value, rect_needs_value_inside, ui.label_theme_left_alinged_small_black_text);
					immediate_quad(rect_needs_value_inside_positive, color_happy);
				}
				else if (happiness > 0.4f)
				{
					std::string happiness_value{ "Moderately happy." };
					immediate_text(happiness_value, rect_needs_value_inside, ui.label_theme_left_alinged_small_black_text);
					immediate_quad(rect_needs_value_inside_positive, color_moderately_happy);
				}
				else if (happiness > 0.2f)
				{
					std::string happiness_value{ "Unhappy!" };
					immediate_text(happiness_value, rect_needs_value_inside, ui.label_theme_left_alinged_small_black_text);
					immediate_quad(rect_needs_value_inside_positive, color_unhappy);
				}
				else
				{
					std::string happiness_value{ "Angry!!" };
					immediate_text(happiness_value, rect_needs_value_inside, ui.label_theme_left_alinged_small_black_text);
					immediate_quad(rect_needs_value_inside_positive, color_angry);
				}
			}
		}

		if (ui.draw_building_panel)
		{
			draw_building_panel(ui);
		}
	}

	bool on_game_scene_ui_layer_mouse_released(void* p, Event::MouseButtonReleasedEvent& event)
	{
		return false;
	}
}