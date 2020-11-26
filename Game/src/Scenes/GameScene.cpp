#include "canpch.h"
#include "GameScene.h"

#include "Managers/RoadManager.h"
//#include "Managers/TreeManager.h"
//#include "Managers/BuildingManager.h"

#include "GameApp.h"

namespace Can
{
	GameScene::GameScene(GameApp* application)
		: MainApplication(application)
		, m_MainCameraController(
			45.0f,
			1280.0f / 720.0f,
			0.1f,
			1000.0f,
			glm::vec3{ 10.0f, 5.0f, -5.0f },
			glm::vec3{ -60.0f, 0.0f, 0.0f }
		)
	{
	}
	GameScene::~GameScene()
	{
	}
	void GameScene::OnAttach()
	{
	}
	void GameScene::OnDetach()
	{
	}
	void GameScene::OnUpdate(TimeStep ts)
	{
	}
	void GameScene::OnEvent(Event::Event& event)
	{
	}
	bool GameScene::OnMousePressed(Event::MouseButtonPressedEvent& event)
	{
		return false;
	}
}
