#pragma once

#include "Can/Renderer/Object.h"

namespace Can
{
	class GameScene;
	class Road;
	class RoadSegment;
	class Junction;
	class End;

	enum class RoadConstructionMode
	{
		None,
		Straight,
		QuadraticCurve,
		CubicCurve,
		Upgrade,
		Destruct
	};

	struct SnapInformation
	{
		bool snapped;
		glm::vec3 snapLocation;
		Junction* snappedJunction = nullptr;
		End* snappedEnd = nullptr;
		Road* snappedRoad = nullptr;
	};

	class RoadManager
	{
	public:
		RoadManager(GameScene* scene);
		~RoadManager();

		void OnUpdate(glm::vec3 prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);
		void OnUpdate_Straight(glm::vec3 prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);
		void OnUpdate_QuadraticCurve(glm::vec3 prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);
		void OnUpdate_CubicCurve(glm::vec3 prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);
		void OnUpdate_Destruction(glm::vec3 prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);

		bool OnMousePressed(MouseCode button, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);
		bool OnMousePressed_Straight(const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);
		bool OnMousePressed_QuadraticCurve(const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);
		bool OnMousePressed_CubicCurve(const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);
		bool OnMousePressed_Destruction();

		void SetType(size_t type);
		inline size_t GetType() { return m_Type; }

		void SetConstructionMode(RoadConstructionMode mode);

		inline const RoadConstructionMode GetConstructionMode() const { return m_ConstructionMode; }
		inline RoadConstructionMode GetConstructionMode() { return m_ConstructionMode; }

		inline const std::vector<Road*>& GetRoads() const { return m_Roads; }
		inline std::vector<Road*>& GetRoads() { return m_Roads; }

		void Remove(Road* road);

		SnapInformation CheckSnapping(const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);

		void ResetStates();
	public:

		std::array<bool, 5> snapOptions = { true, true, false, true, false };
		// 0 : Roads
		// 1 : Length
		// 2 : Height
		// 3 : Angle
		// 4 : Grid

		std::array<bool, 3> restrictions = { true, true, true };
		// 0 : Small Angle
		// 1 : Short Length
		// 2 : Collision

		std::array<uint8_t, 4> cubicCurveOrder = { 0, 1, 2, 3 };

	private:
		GameScene* m_Scene = nullptr;
		RoadConstructionMode m_ConstructionMode = RoadConstructionMode::None;

		std::vector<Road*> m_Roads{};
		std::vector<RoadSegment*> m_RoadSegments{};
		std::vector<Junction*> m_Junctions{};
		std::vector<End*> m_Ends{};

		int m_ConstructionPhase = 0;
		bool b_ConstructionStarted = false;
		bool b_ConstructionEnded = false;
		bool b_ConstructionStartSnapped = false;
		bool b_ConstructionEndSnapped = false;

		// Transforms
		std::array<glm::vec3, 4> m_ConstructionPositions = { 
			glm::vec3(0.0f), 
			glm::vec3(0.0f),
			glm::vec3(0.0f),
			glm::vec3(0.0f)
		};

		// Start Snap
		Junction* m_StartSnappedJunction = nullptr;
		End* m_StartSnappedEnd = nullptr;
		Road* m_StartSnappedRoad = nullptr;

		// End Snap
		Junction* m_EndSnappedJunction = nullptr;
		End* m_EndSnappedEnd = nullptr;
		Road* m_EndSnappedRoad = nullptr;

		// Destruction Snap
		Junction* m_DestructionSnappedJunction = nullptr;
		End* m_DestructionSnappedEnd = nullptr;
		Road* m_DestructionSnappedRoad = nullptr;

		std::vector<std::vector<Object*>> m_Guidelines{};
		std::vector<size_t> m_GuidelinesInUse{};
		Object* m_GuidelinesStart = nullptr; // End /? Object
		Object* m_GuidelinesEnd = nullptr;

		bool b_ConstructionRestricted = false;

		size_t m_Type = 0;
	};
}