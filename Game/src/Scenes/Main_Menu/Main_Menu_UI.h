#pragma once
#include "Can/Layers/Layer.h"

#include "Can/Camera/Orthographic_Camera_Controller.h"


namespace Can
{
	enum class Menus
	{
		MainMenu = 0,
		Options,
		Credits
	};

	enum class Options_Submenus
	{
		Controls = 0,
		Key_Bindings,
		Graphics,
		Audio,
		Others,
	};

	struct Main_Menu_UI : public Layer::Layer
	{
		Main_Menu_UI() {}

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		virtual void OnUpdate(TimeStep ts) override;
		virtual void OnEvent(Event::Event& event) override;
		bool OnKeyReleased(Event::KeyReleasedEvent& event);

		void* font = nullptr;
		Orthographic_Camera_Controller camera_controller;

		Menus current_menu = Menus::MainMenu;
		Options_Submenus current_options_submenus = Options_Submenus::Controls;

		f32 fpc_sensitivity = 4.0f;
		f32 tdc_sensitivity = 4.0f;
		f32 field_of_view_angle = 45.0f;

		bool key_bind_selection_is_openned = false;
		bool key_is_pressed = false;

		u16 selected_key = 0;

	};

	void init_main_menu_ui_layer(Main_Menu_UI& ui);

	void on_main_menu_ui_layer_attach(Main_Menu_UI& ui);
	void on_main_menu_ui_layer_detach(Main_Menu_UI& ui);

	void on_main_menu_ui_layer_update(Main_Menu_UI& ui, TimeStep ts);
	bool on_main_menu_ui_layer_key_released(Main_Menu_UI& ui, Event::KeyReleasedEvent& event);

	void main_menu_screen(Main_Menu_UI& ui);
	void options_screen(Main_Menu_UI& ui);
	void credits_screen(Main_Menu_UI& ui);
}