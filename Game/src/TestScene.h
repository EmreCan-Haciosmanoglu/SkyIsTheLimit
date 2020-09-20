#pragma once
#include "Can.h"

namespace Can
{
	class GameApp;

	class Road;
	class Junction;
	class End;

	class Building;

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
		void OnUpdate_BuildingConstruction(glm::vec3 prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);
		void OnUpdate_BuildingDestruction(glm::vec3 prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);

		virtual void OnEvent(Can::Event::Event& event) override;

		bool OnMousePressed(Can::Event::MouseButtonPressedEvent& event);
		bool OnMousePressed_RoadConstruction(const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);
		bool OnMousePressed_RoadDestruction();
		bool OnMousePressed_BuildingConstruction();
		bool OnMousePressed_BuildingDestruction();

		void SetSelectedConstructionRoad(size_t index);
		void DeleteSelectedRoad(Road* road);

		void SetSelectedConstructionBuilding(size_t index);

		void SetConstructionMode(ConstructionMode mode);
		void SetRoadConstructionMode(RoadConstructionMode mode);
		void SetBuildingConstructionMode(BuildingConstructionMode mode);

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

		std::array<bool, 2> buildingSnapOptions = { true, true };
		// 0 : Roads
		// 1 : Buildings

		std::array<bool, 5> roadRestrictionOptions = { true, true, true, true, true };
		// 0 : Small Angle
		// 1 : Short Length
		// 2 : Collision with other roads
		// 3 : Collision with buildings
		// 4 : Collision with other objects

		std::array<bool, 4> buildingRestrictionOptions = { true, true, true, true };
		// 0 : Collision with roads
		// 1 : Collision with other buildings
		// 2 : Collision with other objects
		// 3 : Snapping to a road

		RoadConstructionMode m_RoadConstructionMode = RoadConstructionMode::Construct;
		BuildingConstructionMode m_BuildingConstructionMode = BuildingConstructionMode::Construct;
		ConstructionMode m_ConstructionMode = ConstructionMode::Road;
		size_t m_RoadConstructionType = 0;
		size_t m_BuildingType = 0;

	public:

		static std::vector<Road*> m_Roads;
		static std::vector<Junction*> m_Junctions;
		static std::vector<End*> m_Ends;
		static std::vector<Building*> m_Buildings;

	private:
		GameApp* m_Parent;
		Object* m_Terrain;
		Camera::Controller::Perspective m_MainCameraController;

		bool b_RoadConstructionStarted = false;
		bool b_RoadConstructionEnded = false;
		bool b_RoadConstructionStartSnapped = false;
		bool b_RoadConstructionEndSnapped = false;

		// Road Construction Transforms
		glm::vec3 m_RoadConstructionStartCoordinate = { -1.0f, -1.0f, -1.0f };
		glm::vec3 m_RoadConstructionEndCoordinate = { -1.0f, -1.0f, -1.0f };

		// Building Construction Transforms
		glm::vec3 m_BuildingConstructionCoordinate = { -1.0f, -1.0f, -1.0f };
		glm::vec3 m_BuildingConstructionRotation = { -1.0f, -1.0f, -1.0f };

		// Road Construction Start Snap
		Junction* m_RoadConstructionStartSnappedJunction = nullptr;
		End* m_RoadConstructionStartSnappedEnd = nullptr;
		Road* m_RoadConstructionStartSnappedRoad = nullptr;

		// Road Construction End Snap
		Junction* m_RoadConstructionEndSnappedJunction = nullptr;
		End* m_RoadConstructionEndSnappedEnd = nullptr;
		Road* m_RoadConstructionEndSnappedRoad = nullptr;

		// Road Destruction Snap
		Junction* m_RoadDestructionSnappedJunction = nullptr;
		End* m_RoadDestructionSnappedEnd = nullptr;
		Road* m_RoadDestructionSnappedRoad = nullptr;

		// Building Construction Snap
		Road* m_BuildingConstructionSnappedRoad = nullptr;

		// Building Destruction Snap
		Building* m_BuildingDestructionSnappedBuilding = nullptr;

		std::vector<std::vector<Object*>> m_RoadGuidelines;
		std::vector<size_t> m_RoadGuidelinesInUse;
		Object* m_RoadGuidelinesStart = nullptr; // End /? Object
		Object* m_RoadGuidelinesEnd = nullptr;

		Object* m_BuildingGuideline = nullptr;

		bool b_ConstructionRestricted = false;
	};
}
