#pragma once

#include "Main_Menu_UI.h"

namespace Can
{
	struct Main_Menu
	{
		Main_Menu_UI ui_layer {};
	};

	void init_main_menu(class GameApp& app, Main_Menu& main_menu);
	void load_main_menu(class GameApp& app, Main_Menu& main_menu);

	void unload_main_menu(class GameApp& app, Main_Menu& main_menu);
	void deinit_main_menu(class GameApp& app, Main_Menu& main_menu);
}