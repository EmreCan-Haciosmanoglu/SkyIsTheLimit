#pragma once
#include "Can/Layers/Layer.h"

#include "Can/Camera/Orthographic_Camera_Controller.h"

namespace Can
{
	struct Main_Menu_UI : public Layer::Layer
	{
		Main_Menu_UI() {}

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		virtual void OnUpdate(TimeStep ts) override;
		virtual void OnEvent(Event::Event& event) override;


		Orthographic_Camera_Controller camera_controller;
	};

	void init_main_menu_ui_layer(Main_Menu_UI& ui);

	void on_main_menu_ui_layer_attach(Main_Menu_UI& ui);
	void on_main_menu_ui_layer_detach(Main_Menu_UI& ui);

	void on_main_menu_ui_layer_update(Main_Menu_UI& ui, TimeStep ts);
	void on_main_menu_ui_layer_event(Main_Menu_UI& ui, Event::Event& event);
}