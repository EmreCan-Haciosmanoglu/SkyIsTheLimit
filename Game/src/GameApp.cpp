#include "GameApp.h"
#include "Can/EntryPoint.h"

#include "Helper.h"

Can::Application* Can::CreateApplication()
{
	return new GameApp();
}

namespace Can
{
	GameApp::GameApp()
		: testScene(new TestScene(this))
		, uiScene(new UIScene(this))
	{
		terrainPrefab = Helper::GetPrefabForTerrain("assets/objects/flat_land.png");
		//terrainPrefab = Helper::GetPrefabForTerrain("assets/objects/heightmap_smallest.png");

		LoadRoads();
		LoadJunctions();
		LoadEnds();

		PushLayer(testScene);
		PushOverlay(uiScene);
	}

	GameApp::~GameApp()
	{
	}
	void GameApp::LoadRoads()
	{
#define TEMP_ROAD_SHADER_FILE_PATH "assets/shaders/Object.glsl"
		namespace fs = std::filesystem;
		std::string s = fs::current_path().string();
		std::string path = s + "\\assets\\objects\\roads";

		std::vector<std::string> roadobjfiles = Helper::GetFiles(path, "road", ".obj");
		std::vector<std::string> roadpngfiles = Helper::GetFiles(path, "road", ".png");

		size_t roadCount = roadobjfiles.size();
		for (size_t i = 0; i < roadCount; i++)
			roads.push_back(new Prefab(roadobjfiles[i], TEMP_ROAD_SHADER_FILE_PATH, roadpngfiles[i]));
	}
	void GameApp::LoadJunctions()
	{
#define TEMP_JUNCTION_SHADER_FILE_PATH "assets/shaders/Object.glsl"
		namespace fs = std::filesystem;
		std::string s = fs::current_path().string();
		std::string path = s + "\\assets\\objects\\roads";

		std::vector<std::string> junctionobjfiles = Helper::GetFiles(path, "junction", ".obj");
		std::vector<std::string> junctionpngfiles = Helper::GetFiles(path, "junction", ".png");

		size_t junctionCount = junctionobjfiles.size();
		for (size_t i = 0; i < junctionCount; i++)
			junctions.push_back(new Prefab(junctionobjfiles[i], TEMP_JUNCTION_SHADER_FILE_PATH, junctionpngfiles[i]));
	}
	void GameApp::LoadEnds()
	{
#define TEMP_END_SHADER_FILE_PATH "assets/shaders/Object.glsl"
		namespace fs = std::filesystem;
		std::string s = fs::current_path().string();
		std::string path = s + "\\assets\\objects\\roads";

		std::vector<std::string> endobjfiles = Helper::GetFiles(path, "end", ".obj");
		std::vector<std::string> endpngfiles = Helper::GetFiles(path, "end", ".png");

		size_t endCount = endobjfiles.size();
		for (size_t i = 0; i < endCount; i++)
			ends.push_back(new Prefab(endobjfiles[i], TEMP_END_SHADER_FILE_PATH, endpngfiles[i]));
	}
}