#pragma once
#include "Can.h"

namespace Can
{
	class Road;
	class Junction;
	class End;

	struct Road {
		glm::vec3 startPos;
		glm::vec3 endPos;
		Can::Object* object = nullptr;
		Junction* startJunction = nullptr;
		Junction* endJunction = nullptr;
		End* startEnd = nullptr;
		End* endEnd = nullptr;
	};

	struct sort_with_angle
	{
		inline bool operator() (const Road* road1, const Road* road2)
		{
			glm::vec3 R0R1_1;
			glm::vec3 R0R1_2;
			if (
				road1->endJunction != nullptr &&
				road2->endJunction != nullptr &&
				road1->endJunction == road2->endJunction
				)
			{
				R0R1_1 = road1->startPos - road1->endPos;
				R0R1_2 = road2->startPos - road2->endPos;
			}
			else if (
				road1->endJunction != nullptr &&
				road2->startJunction != nullptr &&
				road1->endJunction == road2->startJunction
				)
			{
				R0R1_1 = road1->startPos - road1->endPos;
				R0R1_2 = road2->endPos - road2->startPos;
			}
			else if (
				road1->startJunction != nullptr &&
				road2->endJunction != nullptr &&
				road1->startJunction == road2->endJunction
				)
			{
				R0R1_1 = road1->endPos - road1->startPos;
				R0R1_2 = road2->startPos - road2->endPos;
			}
			else if (
				road1->startJunction != nullptr &&
				road2->startJunction != nullptr &&
				road1->startJunction == road2->startJunction
				)
			{
				R0R1_1 = road1->endPos - road1->startPos;
				R0R1_2 = road2->endPos - road2->startPos;
			}
			else
			{
				R0R1_1 = { 0.01f, 1.0f, 0.01f };
				R0R1_2 = { 0.01f, 1.0f, 0.01f };
			}

			float ed1 = R0R1_1.x <= 0.0f ? 180.0f : 0.0f;
			float ed2 = R0R1_2.x <= 0.0f ? 180.0f : 0.0f;

			float angleR0R1_1 = std::fmod(glm::degrees(glm::atan(-R0R1_1.z / R0R1_1.x)) + ed1 + 360.0f, 360.0f);
			float angleR0R1_2 = std::fmod(glm::degrees(glm::atan(-R0R1_2.z / R0R1_2.x)) + ed2 + 360.0f, 360.0f);

			return (angleR0R1_1 < angleR0R1_2);
		}
	};

	struct Junction {
		glm::vec3 position;
		Can::Object* object = nullptr;
		std::vector<Road*> connectedRoads;
	};
	struct End {
		glm::vec3 position;
		glm::vec2 rotation;
		Can::Object* object = nullptr;
		Road* connectedRoad = nullptr;
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

		glm::vec3 GetRayCastedFromScreen();

	private:
		GameApp* m_Parent;
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

		std::vector<Can::Object*> m_RoadGuidelines;
		std::vector<Can::Object*> m_JunctionGuidelines;
		Can::Object* m_RoadGuidelinesStart = nullptr;
		Can::Object* m_RoadGuidelinesEnd = nullptr;



	public:
		Can::Object* roadPrefab;
		Can::Object* endPrefab;
		Can::Object* JunctionPrefab;
		float roadPrefabWidth = 0.0f;
		float roadPrefabLength = 0.0f;
	};
}