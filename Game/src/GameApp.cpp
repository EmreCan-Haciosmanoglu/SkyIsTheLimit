#include "GameApp.h"
#include "Can/EntryPoint.h"
#include "Can.h"

#include "Helper.h"

Can::Application* Can::CreateApplication(const Can::WindowProps& props)
{
	return new GameApp(props);
}

namespace Can
{
	GameApp* GameApp::instance = nullptr;

	GameApp::GameApp(const Can::WindowProps& props)
		: Application(props)
		, perspective_camera_controller(
			45.0f,
			16.0f / 9.0f,
			0.1f,
			1000.0f,
			v3{ 2.0f, -3.0f, 5.0f },
			v3{ 0.0f, -45.0f, 90.0f }
		)
	{
		instance = this;

		terrainPrefab = Helper::GetPrefabForTerrain("assets/objects/flat_land.png");
		terrainTexture = Texture2D::Create("assets/objects/flat_land.png");
		//terrainPrefab = Helper::GetPrefabForTerrain("assets/objects/flat_land_small.png");
		//terrainPrefab = Helper::GetPrefabForTerrain("assets/objects/heightmap_smallest.png");
		//terrainPrefab = Helper::GetPrefabForTerrain("assets/objects/heightmap.png");

		treeMap = Texture2D::Create("assets/textures/treeMap.png");
		addTexture = Texture2D::Create("assets/textures/Buttons/Add.png");
		saveTexture = Texture2D::Create("assets/textures/Buttons/Save.png");
		pauseTexture = Texture2D::Create("assets/textures/Buttons/Pause.png");
		removeTexture = Texture2D::Create("assets/textures/Buttons/Remove.png");
		cancelTexture = Texture2D::Create("assets/textures/Buttons/Cancel.png");
		changeTexture = Texture2D::Create("assets/textures/Buttons/Change.png");
		upgradeTexture = Texture2D::Create("assets/textures/Buttons/Upgrade.png");
		straightTexture = Texture2D::Create("assets/textures/Buttons/Straight.png");
		quadraticTexture = Texture2D::Create("assets/textures/Buttons/Quadratic.png");
		downgradeTexture = Texture2D::Create("assets/textures/Buttons/Downgrade.png");
		cubic1234Texture = Texture2D::Create("assets/textures/Buttons/Cubic1234.png");
		cubic1243Texture = Texture2D::Create("assets/textures/Buttons/Cubic1243.png");
		cubic1342Texture = Texture2D::Create("assets/textures/Buttons/Cubic1342.png");
		cubic1432Texture = Texture2D::Create("assets/textures/Buttons/Cubic1432.png");
		normalSpeedTexture = Texture2D::Create("assets/textures/Buttons/NormalSpeed.png");
		twoTimesSpeedTexture = Texture2D::Create("assets/textures/Buttons/TwoTimesSpeed.png");
		fourTimesSpeedTexture = Texture2D::Create("assets/textures/Buttons/FourTimesSpeed.png");

		load_road_types();
		LoadBuildings();
		LoadTrees();
		LoadCars();
		LoadPeople();



		init_main_menu(*this, main_menu);
		load_main_menu(*this, main_menu);
	}

	void GameApp::start_the_game(std::string& save_name, bool is_old_game)
	{
		unload_main_menu(*this, main_menu);
		deinit_main_menu(*this, main_menu);

		gameScene = new GameScene(this, save_name);
		PushLayer(gameScene);
		if (is_old_game)
			gameScene->load_the_game();

		uiScene = new UIScene(this);
		PushOverlay(uiScene);

		debugScene = new Debug(this);
		PushOverlay(debugScene);
	}

	GameApp::~GameApp()
	{
		//PopLayer(gameScene);
		//delete gameScene;
		//PopOverlay(uiScene);
		//delete uiScene;
		//PopOverlay(debugScene);
		//delete debugScene;

		//unload_main_menu(*this, main_menu);
	}

	void GameApp::load_road_types()
	{
#define TEMP_SHADER_FILE_PATH "assets/shaders/3DTexturedObject.glsl"

		namespace fs = std::filesystem;
		std::string current_path = fs::current_path().string();
		std::string path_to_roads = current_path.append("\\assets\\objects\\roads\\");
		std::string path_to_road_identifier_list = std::string(path_to_roads).append("list.txt");

		std::ifstream file(path_to_road_identifier_list);

		if (file.is_open())
		{
			char* name_key = "Name\0";
			char* asym_key = "Asym\0";
			char* zone_key = "Zoneable\0";
			char* has_median_key = "Has_Median\0";

			char* road_object_key = "Road_Object\0";
			char* road_texture_key = "Road_Texture\0";
			char* road_junction_object_key = "Road_Junc_Object\0";
			char* road_junction_texture_key = "Road_Junc_Texture\0";
			char* road_junction_mirror_object_key = "Road_Junc_Mirror_Object\0";
			char* road_junction_mirror_texture_key = "Road_Junc_Mirror_Texture\0";
			char* road_end_object_key = "Road_End_Object\0";
			char* road_end_texture_key = "Road_End_Texture\0";
			char* road_end_mirror_object_key = "Road_End_Mirror_Object\0";
			char* road_end_mirror_texture_key = "Road_End_Mirror_Texture\0";

			char* tunnel_object_key = "Tunnel_Object\0";
			char* tunnel_texture_key = "Tunnel_Texture\0";
			char* tunnel_junction_object_key = "Tunnel_Junc_Object\0";
			char* tunnel_junction_texture_key = "Tunnel_Junc_Texture\0";
			char* tunnel_junction_mirror_object_key = "Tunnel_Junc_Mirror_Object\0";
			char* tunnel_junction_mirror_texture_key = "Tunnel_Junc_Mirror_Texture\0";
			char* tunnel_end_object_key = "Tunnel_End_Object\0";
			char* tunnel_end_texture_key = "Tunnel_End_Texture\0";
			char* tunnel_end_mirror_object_key = "Tunnel_End_Mirror_Object\0";
			char* tunnel_end_mirror_texture_key = "Tunnel_End_Mirror_Texture\0";

			char* tunnel_entrance_object_key = "Tunnel_Entrance_Object\0";
			char* tunnel_entrance_texture_key = "Tunnel_Entrance_Texture\0";
			char* tunnel_entrance_mirror_object_key = "Tunnel_Entrance_Mirror_Object\0";
			char* tunnel_entrance_mirror_texture_key = "Tunnel_Entrance_Mirror_Texture\0";

			char* thumbnail_key = "Thumbnail\0";

			char* lanes_backward_key = "Lanes_Backward\0";
			char* lanes_forward_key = "Lanes_Forward\0";

			std::string line;
			while (std::getline(file, line))
			{
				// using printf() in all tests for consistency
				if (line != "START")
					continue;
				bool end_is_found = false;
				bool tunnel_is_found = false;

				std::string name;
				std::string asym;
				std::string zone;
				std::string has_median;

				std::string road_obj = path_to_roads;
				std::string road_png = path_to_roads;
				std::string road_junction_obj = path_to_roads;
				std::string road_junction_png = path_to_roads;
				std::string road_junction_mirror_obj = path_to_roads;
				std::string road_junction_mirror_png = path_to_roads;
				std::string road_end_obj = path_to_roads;
				std::string road_end_png = path_to_roads;
				std::string road_end_mirror_obj = path_to_roads;
				std::string road_end_mirror_png = path_to_roads;

				std::string tunnel_obj = path_to_roads;
				std::string tunnel_png = path_to_roads;
				std::string tunnel_junction_obj = path_to_roads;
				std::string tunnel_junction_png = path_to_roads;
				std::string tunnel_junction_mirror_obj = path_to_roads;
				std::string tunnel_junction_mirror_png = path_to_roads;
				std::string tunnel_end_obj = path_to_roads;
				std::string tunnel_end_png = path_to_roads;
				std::string tunnel_end_mirror_obj = path_to_roads;
				std::string tunnel_end_mirror_png = path_to_roads;

				std::string tunnel_entrance_obj = path_to_roads;
				std::string tunnel_entrance_png = path_to_roads;
				std::string tunnel_entrance_mirror_obj = path_to_roads;
				std::string tunnel_entrance_mirror_png = path_to_roads;

				std::string thumbnail_png = path_to_roads;

				std::vector<Lane> l_backward{};
				u64 l_backward_count = 0;
				std::vector<Lane> l_forward{};
				u64 l_forward_count = 0;

				// bool s for if key pair exist

				road_types.push_back(RoadType());
				RoadType& type = road_types[road_types.size() - 1];

				while (std::getline(file, line))
				{
					if (line == "END")
					{
						end_is_found = true;
						type.name = name;
						type.road = new Prefab(road_obj, TEMP_SHADER_FILE_PATH, road_png);
						type.road_junction = new Prefab(road_junction_obj, TEMP_SHADER_FILE_PATH, road_junction_png);
						type.road_end = new Prefab(road_end_obj, TEMP_SHADER_FILE_PATH, road_end_png);
						type.thumbnail = Texture2D::Create(thumbnail_png);
						type.zoneable = zone != "False";
						type.has_median = has_median != "False";
						if (asym != "False")
						{
							type.asymmetric = true;
							type.road_junction_mirror = new Prefab(road_junction_mirror_obj, TEMP_SHADER_FILE_PATH, road_junction_mirror_png);
							type.road_end_mirror = new Prefab(road_end_mirror_obj, TEMP_SHADER_FILE_PATH, road_end_mirror_png);
						}
						if (tunnel_is_found)
						{
							type.tunnel = new Prefab(tunnel_obj, TEMP_SHADER_FILE_PATH, tunnel_png);
							type.tunnel_junction = new Prefab(tunnel_junction_obj, TEMP_SHADER_FILE_PATH, tunnel_junction_png);
							type.tunnel_end = new Prefab(tunnel_end_obj, TEMP_SHADER_FILE_PATH, tunnel_end_png);
							type.tunnel_entrance = new Prefab(tunnel_entrance_obj, TEMP_SHADER_FILE_PATH, tunnel_entrance_png);
							if (asym != "False")
							{
								type.tunnel_junction_mirror = new Prefab(tunnel_junction_mirror_obj, TEMP_SHADER_FILE_PATH, tunnel_junction_mirror_png);
								type.tunnel_end_mirror = new Prefab(tunnel_end_mirror_obj, TEMP_SHADER_FILE_PATH, tunnel_end_mirror_png);
								type.tunnel_entrance_mirror = new Prefab(tunnel_entrance_mirror_obj, TEMP_SHADER_FILE_PATH, tunnel_entrance_mirror_png);
							}
							type.tunnel_length = type.tunnel->boundingBoxM.x - type.tunnel->boundingBoxL.x;
							type.tunnel_width = type.tunnel->boundingBoxM.y - type.tunnel->boundingBoxL.y;
							type.tunnel_height = type.tunnel->boundingBoxM.z - type.tunnel->boundingBoxL.z;
							type.tunnel_junction_length = type.tunnel_junction->boundingBoxM.x - type.tunnel_junction->boundingBoxL.x;
						}
						type.road_length = type.road->boundingBoxM.x - type.road->boundingBoxL.x;
						type.road_width = type.road->boundingBoxM.y - type.road->boundingBoxL.y;
						type.road_height = type.road->boundingBoxM.z - type.road->boundingBoxL.z;
						type.road_junction_length = type.road_junction->boundingBoxM.x - type.road_junction->boundingBoxL.x;
						if (l_backward_count > 0)
							type.lanes_backward = l_backward;
						if (l_forward_count > 0)
							type.lanes_forward = l_forward;
						/*clearing for next road*/ {
							name = "";
							asym = "";
							zone = "";
							has_median = "";

							road_obj = path_to_roads;
							road_png = path_to_roads;
							road_junction_obj = path_to_roads;
							road_junction_png = path_to_roads;
							road_junction_mirror_obj = path_to_roads;
							road_junction_mirror_png = path_to_roads;
							road_end_obj = path_to_roads;
							road_end_png = path_to_roads;
							road_end_mirror_obj = path_to_roads;
							road_end_mirror_png = path_to_roads;

							tunnel_obj = path_to_roads;
							tunnel_png = path_to_roads;
							tunnel_junction_obj = path_to_roads;
							tunnel_junction_png = path_to_roads;
							tunnel_junction_mirror_obj = path_to_roads;
							tunnel_junction_mirror_png = path_to_roads;
							tunnel_end_obj = path_to_roads;
							tunnel_end_png = path_to_roads;
							tunnel_end_mirror_obj = path_to_roads;
							tunnel_end_mirror_png = path_to_roads;

							tunnel_entrance_obj = path_to_roads;
							tunnel_entrance_png = path_to_roads;
							tunnel_entrance_mirror_obj = path_to_roads;
							tunnel_entrance_mirror_png = path_to_roads;

							thumbnail_png = path_to_roads;

							l_backward = std::vector<Lane>();
							l_forward = std::vector<Lane>();

							tunnel_is_found = false;
						}
					}
					std::string::iterator seperator = std::find(line.begin(), line.end(), ':');
					if (seperator == line.end())
						break;
					char* print_from = seperator._Unwrapped() + 1;
					if (std::equal(line.begin(), seperator, name_key))
						name = std::string(print_from);
					else if (std::equal(line.begin(), seperator, asym_key))
						asym = std::string(print_from);
					else if (std::equal(line.begin(), seperator, zone_key))
						zone = std::string(print_from);
					else if (std::equal(line.begin(), seperator, has_median_key))
						has_median = std::string(print_from);
					else if (std::equal(line.begin(), seperator, road_object_key))
						road_obj.append(print_from);
					else if (std::equal(line.begin(), seperator, road_texture_key))
						road_png.append(print_from);
					else if (std::equal(line.begin(), seperator, road_junction_object_key))
						road_junction_obj.append(print_from);
					else if (std::equal(line.begin(), seperator, road_junction_texture_key))
						road_junction_png.append(print_from);
					else if (std::equal(line.begin(), seperator, road_junction_mirror_object_key))
						road_junction_mirror_obj.append(print_from);
					else if (std::equal(line.begin(), seperator, road_junction_mirror_texture_key))
						road_junction_mirror_png.append(print_from);
					else if (std::equal(line.begin(), seperator, road_end_object_key))
						road_end_obj.append(print_from);
					else if (std::equal(line.begin(), seperator, road_end_texture_key))
						road_end_png.append(print_from);
					else if (std::equal(line.begin(), seperator, road_end_mirror_object_key))
						road_end_mirror_obj.append(print_from);
					else if (std::equal(line.begin(), seperator, road_end_mirror_texture_key))
						road_end_mirror_png.append(print_from);
					else if (std::equal(line.begin(), seperator, tunnel_object_key))
					{
						tunnel_obj.append(print_from);
						tunnel_is_found = true;
					}
					else if (std::equal(line.begin(), seperator, tunnel_texture_key))
						tunnel_png.append(print_from);
					else if (std::equal(line.begin(), seperator, tunnel_junction_object_key))
						tunnel_junction_obj.append(print_from);
					else if (std::equal(line.begin(), seperator, tunnel_junction_texture_key))
						tunnel_junction_png.append(print_from);
					else if (std::equal(line.begin(), seperator, tunnel_junction_mirror_object_key))
						tunnel_junction_mirror_obj.append(print_from);
					else if (std::equal(line.begin(), seperator, tunnel_junction_mirror_texture_key))
						tunnel_junction_mirror_png.append(print_from);
					else if (std::equal(line.begin(), seperator, tunnel_end_object_key))
						tunnel_end_obj.append(print_from);
					else if (std::equal(line.begin(), seperator, tunnel_end_texture_key))
						tunnel_end_png.append(print_from);
					else if (std::equal(line.begin(), seperator, tunnel_end_mirror_object_key))
						tunnel_end_mirror_obj.append(print_from);
					else if (std::equal(line.begin(), seperator, tunnel_end_mirror_texture_key))
						tunnel_end_mirror_png.append(print_from);
					else if (std::equal(line.begin(), seperator, tunnel_entrance_object_key))
						tunnel_entrance_obj.append(print_from);
					else if (std::equal(line.begin(), seperator, tunnel_entrance_texture_key))
						tunnel_entrance_png.append(print_from);
					else if (std::equal(line.begin(), seperator, tunnel_entrance_mirror_object_key))
						tunnel_entrance_mirror_obj.append(print_from);
					else if (std::equal(line.begin(), seperator, tunnel_entrance_mirror_texture_key))
						tunnel_entrance_mirror_png.append(print_from);
					else if (std::equal(line.begin(), seperator, thumbnail_key))
						thumbnail_png.append(print_from);
					else if (std::equal(line.begin(), seperator, lanes_backward_key))
					{
						std::stringstream ss;
						ss << std::string(print_from);

						ss >> l_backward_count;
						l_backward.reserve(l_backward_count);
						u64 lanes_read = 0;
						while (std::getline(file, line))
						{
							std::string::iterator first_seperator = std::find(line.begin(), line.end(), ':');
							std::string::iterator second_seperator = std::find(first_seperator + 1, line.end(), ':');
							Lane lane{};
							ss.clear();
							ss.str("");
							ss << std::string(line.begin(), first_seperator);
							ss >> lane.distance_from_center;

							ss.clear();
							ss.str("");
							ss << std::string(first_seperator + 1, second_seperator);
							ss >> lane.speed_limit;

							ss.clear();
							ss.str("");
							ss << std::string(second_seperator + 1, line.end());
							ss >> lane.width;
							l_backward.push_back(lane);
							lanes_read++;
							if (lanes_read >= l_backward_count)
								break;
						}
						
					}
					else if (std::equal(line.begin(), seperator, lanes_forward_key))
					{
						std::stringstream ss;
						ss << std::string(print_from);

						ss >> l_forward_count;
						l_backward.reserve(l_forward_count);
						u64 lanes_read = 0;
						while (std::getline(file, line))
						{
							std::string::iterator first_seperator = std::find(line.begin(), line.end(), ':');
							std::string::iterator second_seperator = std::find(first_seperator + 1, line.end(), ':');
							Lane lane{};
							ss.clear();
							ss.str("");
							ss << std::string(line.begin(), first_seperator);
							ss >> lane.distance_from_center;

							ss.clear();
							ss.str("");
							ss << std::string(first_seperator + 1, second_seperator);
							ss >> lane.speed_limit;

							ss.clear();
							ss.str("");
							ss << std::string(second_seperator + 1, line.end());
							ss >> lane.width;
							l_forward.push_back(lane);
							lanes_read++;
							if (lanes_read >= l_forward_count)
								break;
						}
					}
				}
			}
			file.close();
		}
	}

	void GameApp::LoadBuildings()
	{
		buildings = LoadPrefabs("\\assets\\objects\\houses", "House_");
	}
	void GameApp::LoadTrees()
	{
		trees = LoadPrefabs("\\assets\\objects\\trees", "Tree_");
	}
	void GameApp::LoadCars()
	{
		cars = LoadPrefabs("\\assets\\objects\\cars", "Car_");
	}
	void GameApp::LoadPeople()
	{
		people = LoadPrefabs("\\assets\\objects\\People", "Person_");
	}

	std::vector<Prefab*> GameApp::LoadPrefabs(const std::string& folder, const std::string& filter)
	{
#define TEMP_SHADER_FILE_PATH "assets/shaders/3DTexturedObject.glsl"
		std::vector<Prefab*> result;
		namespace fs = std::filesystem;
		std::string s = fs::current_path().string();
		std::string path = s.append(folder.c_str());

		std::vector<std::string> objfiles = Helper::GetFiles(path, filter, ".obj");
		std::vector<std::string> pngfiles = Helper::GetFiles(path, filter, ".png");

		size_t fileCount = objfiles.size();
		for (size_t i = 0; i < fileCount; i++)
			result.push_back(new Prefab(objfiles[i], TEMP_SHADER_FILE_PATH, pngfiles[i]));
		return result;
	}
}