#include "canpch.h"
#include "Main_Menu.h"

#include "GameApp.h"

namespace Can
{
	void init_main_menu(GameApp& app, Main_Menu& main_menu)
	{
		init_main_menu_ui_layer(main_menu.ui_layer);
	}
	void load_main_menu(GameApp& app, Main_Menu& main_menu)
	{
		app.PushLayer(&main_menu.ui_layer);
	}
	
	void unload_main_menu(GameApp& app, Main_Menu& main_menu)
	{
		app.PopLayer(&main_menu.ui_layer);
	}
	void deinit_main_menu(GameApp& app, Main_Menu& main_menu)
	{
	}
}