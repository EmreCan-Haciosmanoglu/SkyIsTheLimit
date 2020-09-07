#pragma once
#include "Can.h"

#include "Road.h"
#include "Junction.h"
#include "End.h"

namespace Can
{
	enum class RoadConstructionMode
	{
		None,
		Construct,
		Upgrade,
		Destruct
	};
	enum class BuildingConstructionMode
	{
		None,
		Construct,
		Upgrade,
		Destruct
	};
	enum class ConstructionMode
	{
		Road,
		Building
	};

	struct RoadSnapInformation
	{
		bool snapped;
		glm::vec3 snapLocation;
		Junction* snappedJunction = nullptr;
		End* snappedEnd = nullptr;
		Road* snappedRoad = nullptr;
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
		void OnUpdate_RoadConstruction(glm::vec3 prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);
		void OnUpdate_RoadDestruction(glm::vec3 prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);

		virtual void OnEvent(Can::Event::Event& event) override;

		bool OnMousePressed(Can::Event::MouseButtonPressedEvent& event);
		bool OnMousePressed_RoadConstruction(const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);
		bool OnMousePressed_RoadDestruction();


		void SetSelectedConstructionRoad(size_t index);
		void DeleteSelectedRoad(Road* road);

		void SetSelectedConstructionBuilding(size_t index) { m_BuildingType = index; }
		
		void SetConstructionMode(ConstructionMode mode);
		void SetRoadConstructionMode(RoadConstructionMode mode);

		void ResetStates();

	private:
		glm::vec3 GetRayCastedFromScreen();
		RoadSnapInformation DidRoadSnapped(const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);

	public:
		std::array<bool, 5> roadSnapOptions = { true, true, false, true, false };
		// 0 : Roads
		// 1 : Length
		// 2 : Height
		// 3 : Angle
		// 4 : Grid

		std::array<bool, 2> buildingSnapOptions = { true, false };
		
		std::array<bool, 5> roadRestrictionOptions = { true, true, true, true, true };
		// 0 : Small Angle
		// 1 : Short Length
		// 2 : Collision with other roads
		// 3 : Collision with buildings
		// 4 : Collision with other objects

		std::array<bool, 3> buildingRestrictionOptions = { false, false, false };
		RoadConstructionMode m_RoadConstructionMode = RoadConstructionMode::Construct;
		BuildingConstructionMode m_BuildingConstructionMode = BuildingConstructionMode::Construct;
		ConstructionMode m_ConstructionMode = ConstructionMode::Road;
		size_t m_RoadConstructionType = 0;
		size_t m_BuildingType = 0;

	private:
		GameApp* m_Parent;
		Object* m_Terrain;
		Camera::Controller::Perspective m_MainCameraController;

		bool b_RoadConstructionStarted = false;
		bool b_RoadConstructionEnded = false;
		bool b_RoadConstructionStartSnapped = false;
		bool b_RoadConstructionEndSnapped = false;

		glm::vec3 m_RoadConstructionStartCoordinate = { -1.0f, -1.0f, -1.0f };
		glm::vec3 m_RoadConstructionEndCoordinate = { -1.0f, -1.0f, -1.0f };

		// Construction Start Snap
		Junction* m_RoadConstructionStartSnappedJunction = nullptr;
		End* m_RoadConstructionStartSnappedEnd = nullptr;
		Road* m_RoadConstructionStartSnappedRoad = nullptr;

		// Construction End Snap
		Junction* m_RoadConstructionEndSnappedJunction = nullptr;
		End* m_RoadConstructionEndSnappedEnd = nullptr;
		Road* m_RoadConstructionEndSnappedRoad = nullptr;

		// Destruction Snap
		Junction* m_RoadDestructionSnappedJunction = nullptr;
		End* m_RoadDestructionSnappedEnd = nullptr;
		Road* m_RoadDestructionSnappedRoad = nullptr;

		std::vector<Road*> m_Roads;
		std::vector<Junction*> m_Junctions;
		std::vector<End*> m_Ends;

		std::vector<std::vector<Object*>> m_RoadGuidelines;
		std::vector<size_t> m_RoadGuidelinesInUse;
		Object* m_RoadGuidelinesStart = nullptr; // End /? Object
		Object* m_RoadGuidelinesEnd = nullptr;

		bool b_ConstructionRestricted = false;
	};
}
