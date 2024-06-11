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

namespace Can
{
	/*Anom*/ namespace {
		void draw_building_panel(Game_Scene_UI& ui)
		{
			auto app = GameApp::instance;
			auto& window = main_application->GetWindow();
			u64 width_in_pixels = window.GetWidth();
			u64 height_in_pixels = window.GetHeight();

			constexpr s32 building_panel_margins{ 20 };
			constexpr s32 button_margin{ 10 };
			constexpr s32 button_size{ 30 };
			constexpr s32 menu_item_size{ 120 };

			Button_Theme button_theme;
			/*Button_Themes*/ {
				button_theme.label_theme = &ui.label_theme_button;
				button_theme.background_color = v4{ 221.0f / 255.0f, 155.0f / 255.0f, 247.0f / 255.0f, 1.0f };
				button_theme.background_color_over = v4{ 231.0f / 255.0f, 175.0f / 255.0f, 252.0f / 255.0f, 1.0f };
				button_theme.background_color_pressed = v4{ 241.0f / 255.0f, 195.0f / 255.0f, 255.0f / 255.0f, 1.0f };
			}

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

			Rect add_rect;
			add_rect.w = button_size;
			add_rect.h = button_size;
			add_rect.x = building_panel_rect.x + button_margin;
			add_rect.y = building_panel_rect.y + building_panel_rect.h - (button_size + button_margin);
			add_rect.z = building_panel_rect.z + 1;

			Rect remove_rect;
			remove_rect.w = button_size;
			remove_rect.h = button_size;
			remove_rect.x = add_rect.x + button_size + button_margin;
			remove_rect.y = add_rect.y;
			remove_rect.z = building_panel_rect.z + 1;

			Rect upgrade_rect;
			upgrade_rect.w = button_size;
			upgrade_rect.h = button_size;
			upgrade_rect.x = remove_rect.x + button_size + button_margin;
			upgrade_rect.y = remove_rect.y;
			upgrade_rect.z = building_panel_rect.z + 1;

			Rect cancel_rect;
			cancel_rect.w = button_size;
			cancel_rect.h = button_size;
			cancel_rect.x = upgrade_rect.x + button_size + button_margin;
			cancel_rect.y = upgrade_rect.y;
			cancel_rect.z = building_panel_rect.z + 1;

			immediate_quad(building_panel_rect, v4{ 255.0f / 255.0f, 166.0f / 255.0f, 158.0f / 255.0f, 1.0f });
			u16 flags = immediate_image_button(add_rect, button_theme, app->addTexture, __LINE__);
			if (flags & BUTTON_STATE_FLAGS_RELEASED)
			{
				ui.game_scene->SetConstructionMode(ConstructionMode::Building);
				ui.game_scene->m_BuildingManager.SetConstructionMode(BuildingConstructionMode::Construct);
			}

			flags = immediate_image_button(remove_rect, button_theme, app->removeTexture, __LINE__);
			if (flags & BUTTON_STATE_FLAGS_RELEASED)
			{
				ui.game_scene->SetConstructionMode(ConstructionMode::Building);
				ui.game_scene->m_BuildingManager.SetConstructionMode(BuildingConstructionMode::Destruct);
			}

			flags = immediate_image_button(upgrade_rect, button_theme, app->upgradeTexture, __LINE__);
			if (flags & BUTTON_STATE_FLAGS_RELEASED)
			{
				ui.game_scene->SetConstructionMode(ConstructionMode::Building);
				ui.game_scene->m_BuildingManager.SetConstructionMode(BuildingConstructionMode::Upgrade);
			}

			flags = immediate_image_button(cancel_rect, button_theme, app->cancelTexture, __LINE__);
			if (flags & BUTTON_STATE_FLAGS_RELEASED)
			{
				ui.game_scene->SetConstructionMode(ConstructionMode::Building);
				ui.game_scene->m_BuildingManager.SetConstructionMode(BuildingConstructionMode::None);
			}

			//immediate_begin_sub_region(sub_region_rect, sub_region_theme, __LINE__);
			immediate_quad(sub_region_rect, sub_region_theme.background_color);

			Rect menu_item_rect;
			menu_item_rect.w = menu_item_size;
			menu_item_rect.h = menu_item_size;
			menu_item_rect.x = sub_region_rect.x + button_margin;
			menu_item_rect.y = sub_region_rect.y + sub_region_rect.h - menu_item_rect.h - button_margin;
			menu_item_rect.z = sub_region_rect.z + 1;

			if (ui.draw_building_panel_inside_type == draw_building_panel_home)
			{
				flags = immediate_image_button(menu_item_rect, ui.button_theme_general_buildings, nullptr, __LINE__);
				menu_item_rect.z++;
				immediate_text("G", menu_item_rect, ui.label_theme_large_text);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
				{
					ui.draw_building_panel_inside_type = draw_building_panel_general;
				}
				menu_item_rect.x += menu_item_rect.w + button_margin;

				flags = immediate_image_button(menu_item_rect, ui.button_theme_special_buildings, nullptr, __LINE__);
				menu_item_rect.z++;
				immediate_text("S", menu_item_rect, ui.label_theme_large_text);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
				{
					ui.draw_building_panel_inside_type = draw_building_panel_special;
				}
			}
			else if (ui.draw_building_panel_inside_type == draw_building_panel_general)
			{
				flags = immediate_image_button(menu_item_rect, ui.button_theme_housing_buildings, nullptr, __LINE__);
				menu_item_rect.z++;
				immediate_text("H", menu_item_rect, ui.label_theme_large_text);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
				{
					ui.draw_building_panel_inside_type = draw_building_panel_housing;
				}
				menu_item_rect.x += menu_item_rect.w + button_margin;

				flags = immediate_image_button(menu_item_rect, ui.button_theme_commercial_buildings, nullptr, __LINE__);
				menu_item_rect.z++;
				immediate_text("B", menu_item_rect, ui.label_theme_large_text);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
				{
					ui.draw_building_panel_inside_type = draw_building_panel_commercial;
				}
				menu_item_rect.x += menu_item_rect.w + button_margin;

				flags = immediate_image_button(menu_item_rect, ui.button_theme_industry_buildings, nullptr, __LINE__);
				menu_item_rect.z++;
				immediate_text("I", menu_item_rect, ui.label_theme_large_text);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
				{
					ui.draw_building_panel_inside_type = draw_building_panel_industry;
				}
				menu_item_rect.x += menu_item_rect.w + button_margin;

				flags = immediate_image_button(menu_item_rect, ui.button_theme_back, nullptr, __LINE__);
				menu_item_rect.z++;
				immediate_text("B", menu_item_rect, ui.label_theme_large_text);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
				{
					ui.draw_building_panel_inside_type = draw_building_panel_home;
				}
			}
			else if (ui.draw_building_panel_inside_type == draw_building_panel_special)
			{
				flags = immediate_image_button(menu_item_rect, ui.button_theme_hospital_buildings, nullptr, __LINE__);
				menu_item_rect.z++;
				immediate_text("H", menu_item_rect, ui.label_theme_large_text);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
				{
					ui.draw_building_panel_inside_type = draw_building_panel_hospital;
				}
				menu_item_rect.x += menu_item_rect.w + button_margin;

				flags = immediate_image_button(menu_item_rect, ui.button_theme_police_buildings, nullptr, __LINE__);
				menu_item_rect.z++;
				immediate_text("P", menu_item_rect, ui.label_theme_large_text);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
				{
					ui.draw_building_panel_inside_type = draw_building_panel_police;
				}
				menu_item_rect.x += menu_item_rect.w + button_margin;

				flags = immediate_image_button(menu_item_rect, ui.button_theme_fire_station_buildings, nullptr, __LINE__);
				menu_item_rect.z++;
				immediate_text("F", menu_item_rect, ui.label_theme_large_text);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
				{
					ui.draw_building_panel_inside_type = draw_building_panel_fire_station;
				}
				menu_item_rect.x += menu_item_rect.w + button_margin;

				flags = immediate_image_button(menu_item_rect, ui.button_theme_back, nullptr, __LINE__);
				menu_item_rect.z++;
				immediate_text("B", menu_item_rect, ui.label_theme_large_text);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
				{
					ui.draw_building_panel_inside_type = draw_building_panel_home;
				}
			}
			else if (ui.draw_building_panel_inside_type == draw_building_panel_housing)
			{
				for (u64 i = 0; i < ui.housing_building_tumbnail_image_files.size(); i++)
				{
					flags = immediate_image_button(menu_item_rect, button_theme, ui.housing_building_tumbnail_image_files[i], __LINE__ * 10000 + i, false);
					if (flags & BUTTON_STATE_FLAGS_RELEASED)
					{
						ui.game_scene->SetConstructionMode(ConstructionMode::Building);
						auto mode = ui.game_scene->m_BuildingManager.GetConstructionMode();
						if (mode == BuildingConstructionMode::None || mode == BuildingConstructionMode::Destruct)
							ui.game_scene->m_BuildingManager.SetConstructionMode(BuildingConstructionMode::Construct);
						ui.game_scene->m_BuildingManager.SetType(i);
					}
					menu_item_rect.x += menu_item_rect.w + button_margin;
				}

				flags = immediate_image_button(menu_item_rect, ui.button_theme_back, nullptr, __LINE__);
				menu_item_rect.z++;
				immediate_text("B", menu_item_rect, ui.label_theme_large_text);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
				{
					ui.draw_building_panel_inside_type = draw_building_panel_general;
				}
			}
			else if (ui.draw_building_panel_inside_type == draw_building_panel_commercial)
			{
				for (u64 i = 0; i < ui.commercial_building_tumbnail_image_files.size(); i++)
				{
					flags = immediate_image_button(menu_item_rect, button_theme, ui.commercial_building_tumbnail_image_files[i], __LINE__ * 10000 + i, false);
					if (flags & BUTTON_STATE_FLAGS_RELEASED)
					{
					}
					menu_item_rect.x += menu_item_rect.w + button_margin;
				}

				flags = immediate_image_button(menu_item_rect, ui.button_theme_back, nullptr, __LINE__);
				menu_item_rect.z++;
				immediate_text("B", menu_item_rect, ui.label_theme_large_text);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
				{
					ui.draw_building_panel_inside_type = draw_building_panel_general;
				}
			}
			else if (ui.draw_building_panel_inside_type == draw_building_panel_industry)
			{
				for (u64 i = 0; i < ui.industry_building_tumbnail_image_files.size(); i++)
				{
					flags = immediate_image_button(menu_item_rect, button_theme, ui.industry_building_tumbnail_image_files[i], __LINE__ * 10000 + i, false);
					if (flags & BUTTON_STATE_FLAGS_RELEASED)
					{
					}
					menu_item_rect.x += menu_item_rect.w + button_margin;
				}

				flags = immediate_image_button(menu_item_rect, ui.button_theme_back, nullptr, __LINE__);
				menu_item_rect.z++;
				immediate_text("B", menu_item_rect, ui.label_theme_large_text);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
				{
					ui.draw_building_panel_inside_type = draw_building_panel_general;
				}
			}
			else if (ui.draw_building_panel_inside_type == draw_building_panel_hospital)
			{
				for (u64 i = 0; i < ui.hospital_building_tumbnail_image_files.size(); i++)
				{
					flags = immediate_image_button(menu_item_rect, button_theme, ui.hospital_building_tumbnail_image_files[i], __LINE__ * 10000 + i, false);
					if (flags & BUTTON_STATE_FLAGS_RELEASED)
					{
					}
					menu_item_rect.x += menu_item_rect.w + button_margin;
				}

				flags = immediate_image_button(menu_item_rect, ui.button_theme_back, nullptr, __LINE__);
				menu_item_rect.z++;
				immediate_text("B", menu_item_rect, ui.label_theme_large_text);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
				{
					ui.draw_building_panel_inside_type = draw_building_panel_special;
				}
			}
			else if (ui.draw_building_panel_inside_type == draw_building_panel_police)
			{
				for (u64 i = 0; i < ui.police_building_tumbnail_image_files.size(); i++)
				{
					flags = immediate_image_button(menu_item_rect, button_theme, ui.police_building_tumbnail_image_files[i], __LINE__ * 10000 + i, false);
					if (flags & BUTTON_STATE_FLAGS_RELEASED)
					{
					}
					menu_item_rect.x += menu_item_rect.w + button_margin;
				}

				flags = immediate_image_button(menu_item_rect, ui.button_theme_back, nullptr, __LINE__);
				menu_item_rect.z++;
				immediate_text("B", menu_item_rect, ui.label_theme_large_text);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
				{
					ui.draw_building_panel_inside_type = draw_building_panel_special;
				}
			}
			else if (ui.draw_building_panel_inside_type == draw_building_panel_fire_station)
			{
				for (u64 i = 0; i < ui.fire_station_building_tumbnail_image_files.size(); i++)
				{
					flags = immediate_image_button(menu_item_rect, button_theme, ui.fire_station_building_tumbnail_image_files[i], __LINE__ * 10000 + i, false);
					if (flags & BUTTON_STATE_FLAGS_RELEASED)
					{
					}
					menu_item_rect.x += menu_item_rect.w + button_margin;
				}

				flags = immediate_image_button(menu_item_rect, ui.button_theme_back, nullptr, __LINE__);
				menu_item_rect.z++;
				immediate_text("B", menu_item_rect, ui.label_theme_large_text);
				if (flags & BUTTON_STATE_FLAGS_RELEASED)
				{
					ui.draw_building_panel_inside_type = draw_building_panel_special;
				}
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
			ui.button_theme_back.background_color = { 0.50f, 0.20f, 0.40f, 1.0f };
			ui.button_theme_back.background_color_over = { 0.60f, 0.30f, 0.50f, 1.0f };
			ui.button_theme_back.background_color_pressed = { 0.70f, 0.40f, 0.50f, 1.0f };

			ui.button_theme_general_buildings.label_theme = &ui.label_theme_button;
			ui.button_theme_general_buildings.background_color = { 0.50f, 0.70f, 0.40f, 1.0f };
			ui.button_theme_general_buildings.background_color_over = { 0.70f, 0.80f, 0.60f, 1.0f };
			ui.button_theme_general_buildings.background_color_pressed = { 0.85f, 0.90f, 0.80f, 1.0f };

			ui.button_theme_special_buildings.label_theme = &ui.label_theme_button;
			ui.button_theme_special_buildings.background_color = { 0.90f, 0.40f, 0.30f, 1.0f };
			ui.button_theme_special_buildings.background_color_over = { 0.95f, 0.50f, 0.40f, 1.0f };
			ui.button_theme_special_buildings.background_color_pressed = { 1.00f, 0.60f, 0.50f, 1.0f };

			ui.button_theme_housing_buildings.label_theme = &ui.label_theme_button;
			ui.button_theme_housing_buildings.background_color = { 0.70f, 0.50f, 0.40f, 1.0f };
			ui.button_theme_housing_buildings.background_color_over = { 0.80f, 0.70f, 0.60f, 1.0f };
			ui.button_theme_housing_buildings.background_color_pressed = { 0.90f, 0.85f, 0.80f, 1.0f };

			ui.button_theme_commercial_buildings.label_theme = &ui.label_theme_button;
			ui.button_theme_commercial_buildings.background_color = { 0.50f, 0.70f, 0.40f, 1.0f };
			ui.button_theme_commercial_buildings.background_color_over = { 0.70f, 0.80f, 0.60f, 1.0f };
			ui.button_theme_commercial_buildings.background_color_pressed = { 0.85f, 0.90f, 0.80f, 1.0f };

			ui.button_theme_industry_buildings.label_theme = &ui.label_theme_button;
			ui.button_theme_industry_buildings.background_color = { 0.90f, 0.40f, 0.30f, 1.0f };
			ui.button_theme_industry_buildings.background_color_over = { 0.95f, 0.50f, 0.40f, 1.0f };
			ui.button_theme_industry_buildings.background_color_pressed = { 1.00f, 0.60f, 0.50f, 1.0f };

			ui.button_theme_hospital_buildings.label_theme = &ui.label_theme_button;
			ui.button_theme_hospital_buildings.background_color = { 0.70f, 0.50f, 0.40f, 1.0f };
			ui.button_theme_hospital_buildings.background_color_over = { 0.80f, 0.70f, 0.60f, 1.0f };
			ui.button_theme_hospital_buildings.background_color_pressed = { 0.90f, 0.85f, 0.80f, 1.0f };

			ui.button_theme_police_buildings.label_theme = &ui.label_theme_button;
			ui.button_theme_police_buildings.background_color = { 0.50f, 0.70f, 0.40f, 1.0f };
			ui.button_theme_police_buildings.background_color_over = { 0.70f, 0.80f, 0.60f, 1.0f };
			ui.button_theme_police_buildings.background_color_pressed = { 0.85f, 0.90f, 0.80f, 1.0f };

			ui.button_theme_fire_station_buildings.label_theme = &ui.label_theme_button;
			ui.button_theme_fire_station_buildings.background_color = { 0.90f, 0.40f, 0.30f, 1.0f };
			ui.button_theme_fire_station_buildings.background_color_over = { 0.95f, 0.50f, 0.40f, 1.0f };
			ui.button_theme_fire_station_buildings.background_color_pressed = { 1.00f, 0.60f, 0.50f, 1.0f };
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
		namespace fs = std::filesystem;
		std::string s = fs::current_path().string();
		std::string pathh = s + "\\assets\\objects\\Houses";
		auto files = Helper::GetFiles(pathh, "Thumbnail_", ".png");
		for (auto& f : files)
			ui.housing_building_tumbnail_image_files.push_back(Texture2D::Create(f));
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
			v4 position_on_screen = ui.game_scene_camera->view_projection * ui.focus_object->transform * v4(0.0f, 0.0f, ui.focus_object->prefab->boundingBoxM.z, 1.0f);
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
	}

	void draw_screen(Game_Scene_UI& ui)
	{
		auto app = GameApp::instance;
		auto& window = main_application->GetWindow();
		u64 width_in_pixels = window.GetWidth();
		u64 height_in_pixels = window.GetHeight();

		constexpr s32 track_width{ 20 };
		constexpr s32 sub_region_button_margin{ 5 };
		constexpr s32 building_panel_margins{ 20 };
		constexpr s32 button_margin{ 10 };
		constexpr s32 button_size{ 30 };


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


			std::string text_x = "X";
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