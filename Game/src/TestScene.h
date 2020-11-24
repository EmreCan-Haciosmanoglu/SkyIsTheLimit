#pragma once
#include "Can.h"
#include "Can/Noise/Perlin.h"
#include "Can/Shadow/ShadowMapMasterRenderer.h"

namespace Can
{
#define NOISE_WIDTH 500
#define NOISE_HEIGHT 500
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
	enum class TreeConstructionMode
	{
		None,
		Adding,
		Removing
	};

	enum class ConstructionMode
	{
		Road,
		Building,
		Tree
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
		virtual void OnImGuiRender() override {}


		virtual void OnUpdate(Can::TimeStep ts) override;
		void OnUpdate_RoadConstruction(glm::vec3 prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);
		void OnUpdate_RoadDestruction(glm::vec3 prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);
		void OnUpdate_BuildingConstruction(glm::vec3 prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);
		void OnUpdate_BuildingDestruction(glm::vec3 prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);
		void OnUpdate_TreeAdding(glm::vec3 prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);
		void OnUpdate_TreeRemoving(glm::vec3 prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);

		virtual void OnEvent(Can::Event::Event& event) override;
		bool OnMousePressed(Can::Event::MouseButtonPressedEvent& event);
		bool OnMousePressed_RoadConstruction(const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);
		bool OnMousePressed_RoadDestruction();
		bool OnMousePressed_BuildingConstruction();
		bool OnMousePressed_BuildingDestruction();
		bool OnMousePressed_TreeAdding();
		bool OnMousePressed_TreeRemoving();

		void SetSelectedConstructionRoad(size_t index);
		void SetSelectedConstructionBuilding(size_t index);
		void SetSelectedTree(size_t index);

		void DeleteSelectedRoad(Road* road);

		void SetConstructionMode(ConstructionMode mode);
		void SetRoadConstructionMode(RoadConstructionMode mode);
		void SetBuildingConstructionMode(BuildingConstructionMode mode);
		void SetTreeConstructionMode(TreeConstructionMode mode);

		void ResetStates();

	private:
		glm::vec3 GetRayCastedFromScreen();
		RoadSnapInformation DidRoadSnapped(const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);

	public:
		static std::array<bool, 5> roadSnapOptions;
		// 0 : Roads
		// 1 : Length
		// 2 : Height
		// 3 : Angle
		// 4 : Grid

		static std::array<bool, 2> buildingSnapOptions;
		// 0 : Roads
		// 1 : Buildings

		static std::array<bool, 3> roadRestrictionOptions;
		// 0 : Small Angle
		// 1 : Short Length
		// 2 : Collision

		static std::array<bool, 2> buildingRestrictionOptions;
		// 0 : Collision
		// 1 : Snapping to a road

		static std::array<bool, 1> treeRestrictionOptions;
		// 0 : Collision

		TreeConstructionMode m_TreeConstructionMode = TreeConstructionMode::None;
		RoadConstructionMode m_RoadConstructionMode = RoadConstructionMode::Construct;
		BuildingConstructionMode m_BuildingConstructionMode = BuildingConstructionMode::Construct;

		ConstructionMode m_ConstructionMode = ConstructionMode::Road;
		size_t m_RoadConstructionType = 0;
		size_t m_BuildingType = 0;
		size_t m_TreeType = 0;

	public:

		static std::vector<Road*> m_Roads;
		static std::vector<Junction*> m_Junctions;
		static std::vector<End*> m_Ends;
		static std::vector<Building*> m_Buildings;
		static std::vector<Object*> m_Trees;

	private:
		GameApp* m_Parent;
		Object* m_Terrain;
		Camera::Controller::Perspective m_MainCameraController;

		ShadowMapMasterRenderer* m_ShadowMapMasterRenderer = nullptr;
		glm::vec3 m_LightPosition{ +0.0f, 1.0f, 0.0f };
		glm::vec3 m_LightDirection{ +1.0f, -1.0f, -1.0f };

		bool b_RoadConstructionStarted = false;
		bool b_RoadConstructionEnded = false;
		bool b_RoadConstructionStartSnapped = false;
		bool b_RoadConstructionEndSnapped = false;

		// Tree Adding Transforms
		glm::vec3 m_TreeAddingCoordinate = glm::vec3(-1.0f);

		// Road Construction Transforms
		glm::vec3 m_RoadConstructionStartCoordinate = glm::vec3(-1.0f);
		glm::vec3 m_RoadConstructionEndCoordinate = glm::vec3(-1.0f);

		// Building Construction Transforms
		glm::vec3 m_BuildingConstructionCoordinate = glm::vec3(-1.0f);
		glm::vec3 m_BuildingConstructionRotation = glm::vec3(0.0f);

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

		// Tree Removing Snap
		Object* m_TreeRemovingSnappedTree = nullptr;


		std::vector<std::vector<Object*>> m_RoadGuidelines;
		std::vector<size_t> m_RoadGuidelinesInUse;
		Object* m_RoadGuidelinesStart = nullptr; // End /? Object
		Object* m_RoadGuidelinesEnd = nullptr;

		Object* m_BuildingGuideline = nullptr;
		Object* m_TreeGuideline = nullptr;

		bool b_ConstructionRestricted = false;

		Noise::Perlin2D<512, NOISE_WIDTH, NOISE_HEIGHT> m_Perlin2DNoise = Noise::Perlin2D<512, NOISE_WIDTH, NOISE_HEIGHT>();
	};
}
