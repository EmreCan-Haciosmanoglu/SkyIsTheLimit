#pragma once
#include "Can.h"

namespace Can
{
	struct Road {
		std::array<glm::vec3, 2> endPoints;
		Can::Object* m_RoadObject;
	};

	class GameApp;
	class TestScene : public Can::Layer::Layer
	{
	public:
		TestScene(GameApp* parent);
		virtual ~TestScene() = default;

		virtual void OnAttach() override {}
		virtual void OnDetach() override {}

		virtual void OnUpdate(Can::TimeStep ts) override;
		virtual void OnEvent(Can::Event::Event& event) override;

		bool OnMousePressed(Can::Event::MouseButtonPressedEvent& event);
	private:
		GameApp* m_Parent;
		Can::Camera::Controller::Perspective m_MainCameraController;
		bool b_Start = false;
		bool b_End = false;

		glm::vec2 m_Start = { -1, -1 };
		glm::vec2 m_End = { -1, -1 };

		glm::vec3 m_StartCoord = { -1.0f, -1.0f, -1.0f };
		glm::vec3 m_EndCoord = { -1.0f, -1.0f, -1.0f };

		glm::vec2 AB = { -1.0f, -1.0f};
		float mAB = 0.0f;

		std::vector<Road> m_Roads;
		std::vector<Can::Object*> m_RoadGuidelines;
	};
}