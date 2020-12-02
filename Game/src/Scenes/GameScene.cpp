#include "canpch.h"
#include "GameScene.h"

#include "GameApp.h"
#include "Helper.h"

namespace Can
{
	GameScene* GameScene::ActiveGameScene = nullptr;

	GameScene::GameScene(GameApp* application)
		: MainApplication(application)
		, m_Terrain(new Object(MainApplication->terrainPrefab, MainApplication->terrainPrefab, { 0.0f, 0.0f, 0.0f, }, { 1.0f, 1.0f, 1.0f, }, { 0.0f, 0.0f, 0.0f, }))
		, m_RoadManager(this)
		, m_TreeManager(this)
		, m_BuildingManager(this)
		, m_MainCameraController(
			45.0f,
			1280.0f / 720.0f,
			0.1f,
			1000.0f,
			glm::vec3{ 10.0f, 5.0f, -5.0f },
			glm::vec3{ -60.0f, 0.0f, 0.0f }
		)
	{
		m_LightDirection = glm::normalize(m_LightDirection);
		m_ShadowMapMasterRenderer = new ShadowMapMasterRenderer(&m_MainCameraController);
	}
	GameScene::~GameScene()
	{
	}
	void GameScene::OnAttach()
	{
		ActiveGameScene = this;
	}
	void GameScene::OnDetach()
	{
	}
	void GameScene::OnUpdate(TimeStep ts)
	{
		m_MainCameraController.OnUpdate(ts);

		RenderCommand::SetClearColor({ 0.9f, 0.9f, 0.9f, 1.0f });
		RenderCommand::Clear();

		glm::vec3 camPos = m_MainCameraController.GetCamera().GetPosition();
		glm::vec3 forward = GetRayCastedFromScreen();

		glm::vec3 I = Helper::RayPlaneIntersection(camPos, forward, { 0.0f, 0.0f, 0.0f, }, { 0.0f, 1.0f, 0.0f, });

		if (isnan(I.x) == false)
		{
			switch (e_ConstructionMode)
			{
			case ConstructionMode::Road:
				m_RoadManager.OnUpdate(I, camPos, forward);
				break;
			case ConstructionMode::Building:
				m_BuildingManager.OnUpdate(I, camPos, forward);
				break;
			case ConstructionMode::Tree:
				m_TreeManager.OnUpdate(I, camPos, forward);
				break;
			case ConstructionMode::None:
				break;
			}
		}

		Renderer3D::BeginScene(m_MainCameraController.GetCamera());


		m_ShadowMapMasterRenderer->Render(m_LightDirection);

		Renderer3D::DrawObjects(
			m_ShadowMapMasterRenderer->GetLS(),
			m_ShadowMapMasterRenderer->GetShadowMap(),
			m_MainCameraController.GetCamera(),
			m_LightPosition
		);

		Renderer3D::EndScene();
		//m_Framebuffer->Unbind();
	}
	void GameScene::OnEvent(Event::Event& event)
	{
		m_MainCameraController.OnEvent(event);
		Can::Event::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Can::Event::MouseButtonPressedEvent>(CAN_BIND_EVENT_FN(GameScene::OnMousePressed));
	}
	bool GameScene::OnMousePressed(Event::MouseButtonPressedEvent& event)
	{
		MouseCode button = event.GetMouseButton();
		glm::vec3 camPos = m_MainCameraController.GetCamera().GetPosition();
		glm::vec3 forward = GetRayCastedFromScreen();


		glm::vec3 bottomPlaneCollisionPoint = Helper::RayPlaneIntersection(camPos, forward, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f });
		glm::vec3 topPlaneCollisionPoint = Helper::RayPlaneIntersection(camPos, forward, { 0.0f, 1.0f * COLOR_COUNT, 0.0f }, { 0.0f, 1.0f, 0.0f });

		bottomPlaneCollisionPoint.z *= -1;
		topPlaneCollisionPoint.z *= -1;

		if (!Helper::CheckBoundingBoxHit(camPos, forward, m_Terrain->prefab->boundingBoxL, m_Terrain->prefab->boundingBoxM))
			return false;

		switch (e_ConstructionMode)
		{
		case ConstructionMode::Road:
			m_RoadManager.OnMousePressed(button, camPos, forward);
			break;
		case ConstructionMode::Building:
			m_BuildingManager.OnMousePressed(button);
			break;
		case ConstructionMode::Tree:
			m_TreeManager.OnMousePressed(button);
			break;
		case ConstructionMode::None:
			break;
		}
		return false;
	}
	void GameScene::SetConstructionMode(ConstructionMode mode)
	{
		e_ConstructionMode = mode;
		ResetStates();
		switch (e_ConstructionMode)
		{
		case ConstructionMode::Road:
			m_RoadManager.SetConstructionMode(m_RoadManager.GetConstructionMode());
			break;
		case ConstructionMode::Building:
			m_BuildingManager.SetConstructionMode(m_BuildingManager.GetConstructionMode());
			break;
		case ConstructionMode::Tree:
			m_TreeManager.SetConstructionMode(m_TreeManager.GetConstructionMode());
			break;
		case ConstructionMode::None:
			break;
		default:
			break;
		}
	}
	void GameScene::ResetStates()
	{
		m_RoadManager.ResetStates();
		m_TreeManager.ResetStates();
		m_BuildingManager.ResetStates();
	}
	glm::vec3 GameScene::GetRayCastedFromScreen()
	{
		auto [mouseX, mouseY] = Can::Input::GetMousePos();
		Application& app = Application::Get();
		float w = (float)(app.GetWindow().GetWidth());
		float h = (float)(app.GetWindow().GetHeight());

		auto camera = m_MainCameraController.GetCamera();
		glm::vec3 camPos = camera.GetPosition();
		glm::vec3 camRot = camera.GetRotation();

		float fovyX = m_MainCameraController.GetFOV();
		float xoffSet = glm::degrees(glm::atan(glm::tan(glm::radians(fovyX)) * (((w / 2.0f) - mouseX) / (w / 2.0f))));
		float yoffSet = glm::degrees(glm::atan(((h - 2.0f * mouseY) * glm::sin(glm::radians(xoffSet))) / (w - 2.0f * mouseX)));

		glm::vec2 offsetDegrees = {
			xoffSet,
			yoffSet
		};

		glm::vec3 forward = {
			-glm::sin(glm::radians(camRot.y)) * glm::cos(glm::radians(camRot.x)),
			glm::sin(glm::radians(camRot.x)),
			-glm::cos(glm::radians(camRot.x)) * glm::cos(glm::radians(camRot.y))
		};
		glm::vec3 up = {
			glm::sin(glm::radians(camRot.x)) * glm::sin(glm::radians(camRot.y)),
			glm::cos(glm::radians(camRot.x)),
			glm::sin(glm::radians(camRot.x)) * glm::cos(glm::radians(camRot.y))
		};
		glm::vec3 right = {
			-glm::sin(glm::radians(camRot.y - 90.0f)),
			0,
			-glm::cos(glm::radians(camRot.y - 90.0f))
		};

		forward = glm::rotate(forward, glm::radians(offsetDegrees.x), up);
		right = glm::rotate(right, glm::radians(offsetDegrees.x), up);
		forward = glm::rotate(forward, glm::radians(offsetDegrees.y), right);
		return forward;
	}
}
