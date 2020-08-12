#pragma once
#include "Can.h"

#include "Road.h"
#include "Junction.h"
#include "End.h"

namespace Can
{
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

		void SetSelectedConstructionRoad(size_t index);

	private:
		glm::vec3 GetRayCastedFromScreen();

	public:
		std::array<bool, 4> snapOptions = { true, false, false, false };

	private:
		GameApp* m_Parent;

		Object* m_Terrain;

		Can::Camera::Controller::Perspective m_MainCameraController;

		bool b_RoadConstructionStarted = false;
		bool b_RoadConstructionEnded = false;
		bool b_RoadConstructionStartSnapped = false;
		bool b_RoadConstructionEndSnapped = false;

		glm::vec3 m_RoadConstructionStartCoordinate = { -1.0f, -1.0f, -1.0f };
		glm::vec3 m_RoadConstructionEndCoordinate = { -1.0f, -1.0f, -1.0f };

		Junction* m_RoadConstructionStartSnappedJunction = nullptr;
		End* m_RoadConstructionStartSnappedEnd = nullptr;
		Road* m_RoadConstructionStartSnappedRoad = nullptr;

		Junction* m_RoadConstructionEndSnappedJunction = nullptr;
		End* m_RoadConstructionEndSnappedEnd = nullptr;
		Road* m_RoadConstructionEndSnappedRoad = nullptr;

		int m_RoadConstructionStartSnappedType = -1;
		int m_RoadConstructionEndSnappedType = -1;


		std::vector<Road*> m_Roads;
		std::vector<Junction*> m_Junctions;
		std::vector<End*> m_Ends;

		std::vector<std::vector<Object*>> m_RoadGuidelines;
		std::vector<size_t> m_RoadGuidelinesInUse;
		Object* m_RoadGuidelinesStart = nullptr; // End /? Object
		Object* m_RoadGuidelinesEnd = nullptr;

		size_t m_RoadConstructionType = 0;

	};
}
