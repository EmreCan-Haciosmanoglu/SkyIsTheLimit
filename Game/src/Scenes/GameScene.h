#pragma once
#include "Can/Layers/Layer.h"
#include "Can/Events/Event.h"
#include "Can/Shadow/ShadowMapMasterRenderer.h"

#include "Managers/RoadManager.h"
#include "Managers/TreeManager.h"
#include "Managers/BuildingManager.h"
#include "Managers/CarManager.h"

namespace Can
{
	class GameApp;

	enum class ConstructionMode
	{
		Road,
		Building,
		Tree,
		Car,
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

		void SetConstructionMode(ConstructionMode mode);
		const ConstructionMode& GetConstructionMode() { return e_ConstructionMode; }

	private:
		glm::vec3 GetRayCastedFromScreen();

	public:
		GameApp* MainApplication = nullptr;
		static GameScene* ActiveGameScene;
		Object* m_Terrain = nullptr;

		RoadManager m_RoadManager;
		TreeManager m_TreeManager;
		BuildingManager m_BuildingManager;
		CarManager m_CarManager;

		ConstructionMode e_ConstructionMode = ConstructionMode::None;

	private:
		Camera::Controller::Perspective m_MainCameraController;


		ShadowMapMasterRenderer* m_ShadowMapMasterRenderer = nullptr;
		glm::vec3 m_LightPosition{ +0.0f, 1.0f, 0.0f };
		glm::vec3 m_LightDirection{ +1.0f, -1.0f, -1.0f };

	};
}