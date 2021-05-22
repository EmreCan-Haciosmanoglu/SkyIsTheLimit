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
	GameApp::GameApp(const Can::WindowProps& props)
		:Application(props)
	{
		terrainPrefab = Helper::GetPrefabForTerrain("assets/objects/flat_land.png");
		//terrainPrefab = Helper::GetPrefabForTerrain("assets/objects/flat_land_small.png");
		//terrainPrefab = Helper::GetPrefabForTerrain("assets/objects/heightmap_smallest.png");
		//terrainPrefab = Helper::GetPrefabForTerrain("assets/objects/heightmap.png");

		treeMap = Texture2D::Create("assets/textures/treeMap.png");
		addTexture = Texture2D::Create("assets/textures/Buttons/Add.png");
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

		LoadRoadTypes();
		//LoadRoads();
		LoadBuildings();
		LoadTrees();
		LoadCars();

		gameScene = new GameScene(this);
		PushLayer(gameScene);

		uiScene = new UIScene(this);
		PushOverlay(uiScene);

		debugScene = new Debug(this);
		PushOverlay(debugScene);
	}

	GameApp::~GameApp()
	{
	}

	void GameApp::LoadRoadTypes()
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
			char* road_object_key = "Road_Object\0";
			char* road_texture_key = "Road_Texture\0";
			char* junction_object_key = "Junc_Object\0";
			char* junction_texture_key = "Junc_Texture\0";
			char* junction_mirror_object_key = "Junc_Mirror_Object\0";
			char* junction_mirror_texture_key = "Junc_Mirror_Texture\0";
			char* end_object_key = "End_Object\0";
			char* end_texture_key = "End_Texture\0";
			char* end_mirror_object_key = "End_Mirror_Object\0";
			char* end_mirror_texture_key = "End_Mirror_Texture\0";
			char* thumbnail_key = "Thumbnail\0";

			std::string line;
			while (std::getline(file, line))
			{
				// using printf() in all tests for consistency
				if (line != "START")
					continue;
				bool end_is_found = false;

				std::string name;
				std::string asym;
				std::string road_obj = path_to_roads;
				std::string road_png = path_to_roads;
				std::string junction_obj = path_to_roads;
				std::string junction_png = path_to_roads;
				std::string junction_mirror_obj = path_to_roads;
				std::string junction_mirror_png = path_to_roads;
				std::string end_obj = path_to_roads;
				std::string end_png = path_to_roads;
				std::string end_mirror_obj = path_to_roads;
				std::string end_mirror_png = path_to_roads;
				std::string thumbnail_png = path_to_roads;

				// bool s for if key pair exist

				RoadType type = RoadType();

				while (std::getline(file, line))
				{
					if (line == "END")
					{
						end_is_found = true;
						type.name = name;
						type.road = new Prefab(road_obj, TEMP_SHADER_FILE_PATH, road_png);
						type.junction = new Prefab(junction_obj, TEMP_SHADER_FILE_PATH, junction_png);
						type.end = new Prefab(end_obj, TEMP_SHADER_FILE_PATH, end_png);
						type.thumbnail = Texture2D::Create(thumbnail_png);
						if (asym != "False")
						{
							type.asymmetric = true;
							type.junction_mirror = new Prefab(junction_mirror_obj, TEMP_SHADER_FILE_PATH, junction_mirror_png);
							type.end_mirror = new Prefab(end_mirror_obj, TEMP_SHADER_FILE_PATH, end_mirror_png);
						}
						type.road_width = type.road->boundingBoxM.z - type.road->boundingBoxL.z;
						type.road_length = type.road->boundingBoxM.x - type.road->boundingBoxL.x;
						type.junction_length = type.junction->boundingBoxM.x - type.junction->boundingBoxL.x;
						road_types.push_back(type);
						/*clearing for next road*/{
							name = "";
							asym = "";
							road_obj = path_to_roads;
							road_png = path_to_roads;
							junction_obj = path_to_roads;
							junction_png = path_to_roads;
							junction_mirror_obj = path_to_roads;
							junction_mirror_png = path_to_roads;
							end_obj = path_to_roads;
							end_png = path_to_roads;
							end_mirror_obj = path_to_roads;
							end_mirror_png = path_to_roads;
							thumbnail_png = path_to_roads; 
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
					else if (std::equal(line.begin(), seperator, road_object_key))
						road_obj = road_obj.append(std::string(print_from));
					else if (std::equal(line.begin(), seperator, road_texture_key))
						road_png = road_png.append(std::string(print_from));
					else if (std::equal(line.begin(), seperator, junction_object_key))
						junction_obj = junction_obj.append(std::string(print_from));
					else if (std::equal(line.begin(), seperator, junction_texture_key))
						junction_png = junction_png.append(std::string(print_from));
					else if (std::equal(line.begin(), seperator, junction_mirror_object_key))
						junction_mirror_obj = junction_mirror_obj.append(std::string(print_from));
					else if (std::equal(line.begin(), seperator, junction_mirror_texture_key))
						junction_mirror_png = junction_mirror_png.append(std::string(print_from));
					else if (std::equal(line.begin(), seperator, end_object_key))
						end_obj = end_obj.append(std::string(print_from));
					else if (std::equal(line.begin(), seperator, end_texture_key))
						end_png = end_png.append(std::string(print_from));
					else if (std::equal(line.begin(), seperator, end_mirror_object_key))
						end_mirror_obj = end_mirror_obj.append(std::string(print_from));
					else if (std::equal(line.begin(), seperator, end_mirror_texture_key))
						end_mirror_png = end_mirror_png.append(std::string(print_from));
					else if (std::equal(line.begin(), seperator, thumbnail_key))
						thumbnail_png = thumbnail_png.append(std::string(print_from));
				}
			}
			file.close();
		}

		//std::vector<std::string> objfiles = Helper::GetFiles(path, filter, ".obj");
		//std::vector<std::string> pngfiles = Helper::GetFiles(path, filter, ".png");
		//
		//size_t fileCount = objfiles.size();
		//for (size_t i = 0; i < fileCount; i++)
		//	result.push_back(new Prefab(objfiles[i], TEMP_SHADER_FILE_PATH, pngfiles[i]));
	}

	/*void GameApp::LoadRoads()
	{
		std::vector<Prefab*> resultRoads = LoadPrefabs("\\assets\\objects\\roads", "road_");
		std::vector<Prefab*> resultJunctions = LoadPrefabs("\\assets\\objects\\roads", "junction_");
		std::vector<Prefab*> resultEnds = LoadPrefabs("\\assets\\objects\\roads", "end_");

		for (size_t i = 0; i < resultRoads.size(); i++)
			roads.push_back({ resultRoads[i], resultJunctions[i], resultEnds[i] });
	}*/

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