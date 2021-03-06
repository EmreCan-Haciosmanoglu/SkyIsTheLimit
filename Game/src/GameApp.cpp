#include "GameApp.h"
#include "Can/EntryPoint.h"
#include "Can.h"

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

		LoadRoads();
		LoadBuildings();

		testScene = new TestScene(this);
		PushLayer(testScene);

		uiScene = new UIScene(this);
		PushOverlay(uiScene);
		//PushLayer(uiScene);

		debugScene = new Debug(this);
		PushOverlay(debugScene);
	}

	GameApp::~GameApp()
	{
	}

	void GameApp::LoadRoads()
	{
		std::vector<Prefab*> resultRoads = LoadPrefabs("\\assets\\objects\\roads", "road_");
		std::vector<Prefab*> resultJunctions = LoadPrefabs("\\assets\\objects\\roads", "junction_");
		std::vector<Prefab*> resultEnds = LoadPrefabs("\\assets\\objects\\roads", "end_");

		for (size_t i = 0; i < resultRoads.size(); i++)
			roads.push_back({ resultRoads[i], resultJunctions[i], resultEnds[i] });
	}

	void GameApp::LoadBuildings()
	{
		buildings = LoadPrefabs("\\assets\\objects\\houses", "House_");
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
		return std::vector<Prefab*>();
	}
}