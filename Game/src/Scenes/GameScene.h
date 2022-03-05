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
		GameScene(GameApp* application, std::string& save_name);
		virtual ~GameScene();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override {}

		virtual bool OnUpdate(TimeStep ts) override;
		virtual void OnEvent(Event::Event& event) override;

		bool OnMousePressed(Event::MouseButtonPressedEvent& event);

		void SetConstructionMode(ConstructionMode mode);
		const ConstructionMode& GetConstructionMode() const { return e_ConstructionMode; }

		void SetSpeedMode(SpeedMode mode);
		const SpeedMode& GetSpeedMode() const { return e_SpeedMode; }

		void save_the_game();
		void load_the_game();

	private:
		glm::vec3 GetRayCastedFromScreen();
		void MoveMe2AnotherFile(float ts);

	public:
		GameApp* MainApplication = nullptr;
		std::string save_name;
		static GameScene* ActiveGameScene;
		Object* m_Terrain = nullptr;

		RoadManager m_RoadManager;
		TreeManager m_TreeManager;
		BuildingManager m_BuildingManager;
		CarManager m_CarManager;

		ConstructionMode e_ConstructionMode = ConstructionMode::None;

		SpeedMode e_SpeedMode = SpeedMode::Normal;

	private:
		Perspective_Camera_Controller camera_controller;


		ShadowMapMasterRenderer* m_ShadowMapMasterRenderer = nullptr;
		glm::vec3 m_LightPosition{ +0.0f, 0.0f, 1.0f };
		glm::vec3 m_LightDirection{ +1.0f, -1.0f, -1.0f };

	};
}