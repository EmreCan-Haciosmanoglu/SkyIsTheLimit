#include "canpch.h"
#include "UIScene.h"
#include "GameApp.h"
#include "SceneEntity.h"
#include "Can/ECS/UI/Panel.h"
#include "Can/ECS/UI/Button.h"

namespace Can
{
	UIScene::UIScene(GameApp* parent)
		: m_Parent(parent)
		, m_ZoomLevel(10.0f)
		, m_AspectRatio(1280.0f / 720.0f)
		, m_CameraController(
			m_AspectRatio,
			m_ZoomLevel,
			false
		)
		, m_Scene(new SceneEntity())
	{
		float width = m_AspectRatio * m_ZoomLevel * 2.0f;
		float height = m_ZoomLevel * 2.0f;

		Panel* panelForRoads = new Panel(
			{ 0.5f, height * (4.0f / 5.0f) - 0.5f, 0.0001f },
			{ width - 1.0f, height * (1.0f / 5.0f) },
			{ 221.0f / 255.0f, 255.0f / 255.0f, 247.0f / 255.0f, 1.0f },
			[]() {std::cout << "You clicked the panel that is for Roads!" << std::endl; }
		);
		Button* buttonForRoads = new Button(
			{ 1.0f, -1.5f, 0.00011f },
			{ 5.0f, 1.0f },
			{ 147.0f / 255.0f, 225.0f / 255.0f, 216.0f / 255.0f, 1.0f },
			[]() {std::cout << "You clicked the Button inside the panel that is for Roads!" << std::endl; }
		);
		panelForRoads->AddAChild(buttonForRoads);

		Panel* panelForBuildings = new Panel(
			{ 0.5f, height * (4.0f / 5.0f) - 0.5f, 0.0002f },
			{ width - 1.0f, height * (1.0f / 5.0f) },
			{ 255.0f / 255.0f, 166.0f / 255.0f, 158.0f / 255.0f, 1.0f },
			[]() {std::cout << "You clicked the panel that is for Buildings!" << std::endl; }
		);

		Panel* panelForNeeds = new Panel(
			{ 0.5f, 1.5f, 0.0003f },
			{ width * (1.0f / 10.0f) - 0.5f, height * (3.0f / 5.0f) - 1.0f },
			{ 170.0f / 255.0f, 68.0f / 255.0f, 101.0f / 255.0f, 1.0f },
			[]() {std::cout << "You clicked the panel that is for Needs!" << std::endl; }
		);

		m_Scene->AddAChild(panelForRoads);
		m_Scene->AddAChild(panelForBuildings);
		m_Scene->AddAChild(panelForNeeds);
	}

	UIScene::~UIScene()
	{
	}

	void UIScene::OnUpdate(Can::TimeStep ts)
	{
		//Can::RenderCommand::SetClearColor({ 0.9f, 0.9f, 0.9f, 1.0f });
		//Can::RenderCommand::Clear();

		float widthHalf = m_AspectRatio * m_ZoomLevel;
		float heightHalf = m_ZoomLevel;

		Can::Renderer2D::BeginScene(m_CameraController.GetCamera());
		m_Scene->Draw({ -widthHalf, heightHalf, 0.0f });
		Can::Renderer2D::EndScene();
	}
	void UIScene::OnEvent(Can::Event::Event& event)
	{
		Can::Event::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Can::Event::MouseButtonPressedEvent>(CAN_BIND_EVENT_FN(UIScene::OnMousePressed));

	}
	bool UIScene::OnMousePressed(Can::Event::MouseButtonPressedEvent& event)
	{
		float cameraWidth = m_AspectRatio * m_ZoomLevel * 2.0f;
		float cameraHeight = m_ZoomLevel * 2.0f;

		float mouseX = Can::Input::GetMouseX();
		float mouseY = Can::Input::GetMouseY();
		Application& app = Application::Get();
		float w = app.GetWindow().GetWidth();
		float h = app.GetWindow().GetHeight();

		glm::vec2 clickPos = { cameraWidth * (mouseX / w), cameraHeight * (mouseY / h) };
		for (auto it = m_Scene->children.end(); it != m_Scene->children.begin();)
		{
			event.m_Handled = (*--it)->OnClickEvent(clickPos);
			if (event.m_Handled)
				return true;
		}
		return false;
	}
}