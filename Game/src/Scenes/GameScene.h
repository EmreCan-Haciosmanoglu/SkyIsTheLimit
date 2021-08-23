#pragma once
#include "Can/Layers/Layer.h"
#include "Can/Events/Event.h"
#include "Can/Shadow/ShadowMapMasterRenderer.h"

#include "Managers/RoadManager.h"
#include "Managers/TreeManager.h"
#include "Managers/PersonManager.h"
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

	enum class SpeedMode
	{
		Pause  = 0,
		Normal = 1,
		TwoX   = 2,
		FourX  = 4
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
		const ConstructionMode& GetConstructionMode() const { return e_ConstructionMode; }

		void SetSpeedMode(SpeedMode mode);
		const SpeedMode& GetSpeedMode() const { return e_SpeedMode; }

	private:
		glm::vec3 GetRayCastedFromScreen();
		void MoveMe2AnotherFile(float ts);

	public:
		GameApp* MainApplication = nullptr;
		static GameScene* ActiveGameScene;
		Object* m_Terrain = nullptr;

		RoadManager m_RoadManager;
		TreeManager m_TreeManager;
		BuildingManager m_BuildingManager;
		CarManager m_CarManager;
		PersonManager m_PersonManager;

		ConstructionMode e_ConstructionMode = ConstructionMode::None;

		SpeedMode e_SpeedMode = SpeedMode::Normal;

	private:
		Perspective_Camera_Controller m_MainCameraController;

		ShadowMapMasterRenderer* m_ShadowMapMasterRenderer = nullptr;
		glm::vec3 m_LightPosition{ +0.0f, 1.0f, 0.0f };
		glm::vec3 m_LightDirection{ +1.0f, -1.0f, -1.0f };

	};
}