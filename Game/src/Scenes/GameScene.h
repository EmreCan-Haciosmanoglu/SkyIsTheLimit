#pragma once
#include "Can/Layers/Layer.h"
#include "Can/Events/Event.h"

namespace Can
{
	class GameApp;

	class RoadManager;
	class TreeManager;
	class BuildingManager;

	enum class ConstructionMode
	{
		Road,
		Building,
		Tree,
		None
	};

	class GameScene : public Layer::Layer
	{
	public:
		GameScene(GameApp* application);
		virtual ~GameScene();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override {}

		virtual void OnUpdate(TimeStep ts) override;
		virtual void OnEvent(Event::Event& event) override;

		bool OnMousePressed(Event::MouseButtonPressedEvent& event);

	public:
		static GameScene* ActiveGameInstance;

		RoadManager* m_RoadBuilder;
		TreeManager* m_TreeBuilder;
		BuildingManager* m_BuildingManager;

	private:
		GameApp* MainApplication = nullptr;

		Camera::Controller::Perspective m_MainCameraController;

		ConstructionMode e_ConstructionMode = ConstructionMode::None;

		ShadowMapMasterRenderer* m_ShadowMapMasterRenderer = nullptr;
		glm::vec3 m_LightPosition{ +0.0f, 1.0f, 0.0f };
		glm::vec3 m_LightDirection{ +1.0f, -1.0f, -1.0f };

	};
}