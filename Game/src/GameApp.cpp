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
	{
		terrainPrefab = Helper::GetPrefabForTerrain("assets/objects/flat_land.png");
		//terrainPrefab = Helper::GetPrefabForTerrain("assets/objects/flat_land_small.png");
		//terrainPrefab = Helper::GetPrefabForTerrain("assets/objects/heightmap_smallest.png");
		//terrainPrefab = Helper::GetPrefabForTerrain("assets/objects/heightmap.png");

		CombinePrefabs();

		uiScene = new UIScene(this);
		debugScene = new Debug(this);
		testScene = new TestScene(this);
		PushLayer(testScene);
		PushOverlay(uiScene);
		PushOverlay(debugScene);
	}

	GameApp::~GameApp()
	{
	}

	void GameApp::CombinePrefabs()
	{
		std::vector<Prefab*> resultRoads = LoadRoadPrefabs();
		std::vector<Prefab*> resultJunctions = LoadJunctionPrefabs();
		std::vector<Prefab*> resultEnds = LoadEndPrefabs();

		for (size_t i = 0; i < resultRoads.size(); i++)
			roads.push_back({ resultRoads[i], resultJunctions[i], resultEnds[i] });
	}

	std::vector<Prefab*> GameApp::LoadRoadPrefabs()
	{
#define TEMP_ROAD_SHADER_FILE_PATH "assets/shaders/Object.glsl"
		std::vector<Prefab*> result;
		namespace fs = std::filesystem;
		std::string s = fs::current_path().string();
		std::string path = s + "\\assets\\objects\\roads";

		std::vector<std::string> roadobjfiles = Helper::GetFiles(path, "road_", ".obj");
		std::vector<std::string> roadpngfiles = Helper::GetFiles(path, "road_", ".png");

		size_t roadCount = roadobjfiles.size();
		for (size_t i = 0; i < roadCount; i++)
			result.push_back(new Prefab(roadobjfiles[i], TEMP_ROAD_SHADER_FILE_PATH, roadpngfiles[i]));
		return result;
	}
	std::vector<Prefab*> GameApp::LoadJunctionPrefabs()
	{
#define TEMP_JUNCTION_SHADER_FILE_PATH "assets/shaders/Object.glsl"
		std::vector<Prefab*> result;
		namespace fs = std::filesystem;
		std::string s = fs::current_path().string();
		std::string path = s + "\\assets\\objects\\roads";

		std::vector<std::string> junctionobjfiles = Helper::GetFiles(path, "junction_", ".obj");
		std::vector<std::string> junctionpngfiles = Helper::GetFiles(path, "junction_", ".png");

		size_t junctionCount = junctionobjfiles.size();
		for (size_t i = 0; i < junctionCount; i++)
			result.push_back(new Prefab(junctionobjfiles[i], TEMP_JUNCTION_SHADER_FILE_PATH, junctionpngfiles[i]));
		return result;
	}
	std::vector<Prefab*> GameApp::LoadEndPrefabs()
	{
#define TEMP_END_SHADER_FILE_PATH "assets/shaders/Object.glsl"
		std::vector<Prefab*> result;
		namespace fs = std::filesystem;
		std::string s = fs::current_path().string();
		std::string path = s + "\\assets\\objects\\roads";

		std::vector<std::string> endobjfiles = Helper::GetFiles(path, "end_", ".obj");
		std::vector<std::string> endpngfiles = Helper::GetFiles(path, "end_", ".png");

		size_t endCount = endobjfiles.size();
		for (size_t i = 0; i < endCount; i++)
			result.push_back(new Prefab(endobjfiles[i], TEMP_END_SHADER_FILE_PATH, endpngfiles[i]));
		return result;
	}
}