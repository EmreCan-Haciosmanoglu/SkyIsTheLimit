#pragma once

#include "Can/Renderer/Object.h"

namespace Can
{
	class GameScene;
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
		bool snapped = false;
		
		glm::vec3 location{ 0.0f, 0.0f, 0.0f};
		
		Junction* junction = nullptr;
		End* end = nullptr;
		RoadSegment* roadSegment = nullptr;
		
		float T = 0.0f;
	};

	class RoadManager
	{
	public:
		RoadManager(GameScene* scene);
		~RoadManager();

		void OnUpdate(glm::vec3& prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);
		void OnUpdate_Straight(glm::vec3& prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);
		void OnUpdate_QuadraticCurve(glm::vec3& prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);
		void OnUpdate_CubicCurve(glm::vec3& prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);
		void OnUpdate_Destruction(glm::vec3& prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);

		void DrawStraightGuidelines(const glm::vec3& pointA, const glm::vec3& pointB);
		void DrawCurvedGuidelines(const std::array<glm::vec3, 4>& curvePoints);
		
		bool CheckStraightRoadRoadCollision(const std::array<std::array<glm::vec2, 3>, 2>& polygon);
		void CheckStraightRoadBuildingCollision(const std::array<std::array<glm::vec2, 3>, 2>& polygon);
		void CheckStraightRoadTreeCollision(const std::array<std::array<glm::vec2, 3>, 2>& polygon);
		
		// need template
		bool CheckRoadRoadCollision(const std::array<std::array<glm::vec2, 3>, 2>& box, const std::array<std::array<glm::vec2, 3>, (10 - 1) * 2>& polygon);
		void CheckRoadBuildingCollision(const std::array<std::array<glm::vec2, 3>, 2>& box, const std::array<std::array<glm::vec2, 3>, (10 - 1) * 2>& polygon);
		void CheckRoadTreeCollision(const std::array<std::array<glm::vec2, 3>, 2>& box, const std::array<std::array<glm::vec2, 3>, (10 - 1) * 2>& polygon);

		bool OnMousePressed(MouseCode button);
		bool OnMousePressed_Straight();
		bool OnMousePressed_QuadraticCurve();
		bool OnMousePressed_CubicCurve();
		bool OnMousePressed_Destruction();

		void SetType(size_t type);
		inline size_t GetType() { return m_Type; }

		void SetConstructionMode(RoadConstructionMode mode);

		inline const RoadConstructionMode GetConstructionMode() const { return m_ConstructionMode; }
		inline RoadConstructionMode GetConstructionMode() { return m_ConstructionMode; }

		inline const std::vector<RoadSegment*>& GetRoadSegments() const { return m_RoadSegments; }
		inline std::vector<RoadSegment*>& GetRoadSegments() { return m_RoadSegments; }

		void AddRoadSegment(const std::array<glm::vec3, 4>& curvePoints);
		void RemoveRoadSegment(RoadSegment* roadSegment);

		SnapInformation CheckSnapping(const glm::vec3& prevLocation);

		void ResetStates();
	public:

		std::array<bool, 5> snapOptions = { true, false, false, true, false };
		// 0 : Roads
		// 1 : Length
		// 2 : Height
		// 3 : Angle
		// 4 : Grid

		std::array<bool, 3> restrictions = { false, false, false };
		// 0 : Small Angle
		// 1 : Short Length
		// 2 : Collision

		std::array<uint8_t, 4> cubicCurveOrder = { 0, 1, 2, 3 };

	private:
		GameScene* m_Scene = nullptr;
		RoadConstructionMode m_ConstructionMode = RoadConstructionMode::None;

		std::vector<RoadSegment*> m_RoadSegments{};
		std::vector<Junction*> m_Junctions{};		// Change this to node instead?
		std::vector<End*> m_Ends{};					// Change this to node instead?

		int m_ConstructionPhase = 0;
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
		RoadSegment* m_StartSnappedRoadSegment = nullptr;
		float m_StartSnappedRoadSegmentT = 0.0f;

		// End Snap
		Junction* m_EndSnappedJunction = nullptr;
		End* m_EndSnappedEnd = nullptr;
		RoadSegment* m_EndSnappedRoadSegment = nullptr;
		float m_EndSnappedRoadSegmentT = 0.0f;

		// Destruction Snap
		Junction* m_DestructionSnappedJunction = nullptr;
		End* m_DestructionSnappedEnd = nullptr;
		RoadSegment* m_DestructionSnappedRoadSegment = nullptr;

		std::vector<std::vector<Object*>> m_Guidelines{};
		std::vector<size_t> m_GuidelinesInUse{};
		Object* m_GuidelinesStart = nullptr; // End /? Object
		Object* m_GuidelinesEnd = nullptr;

		bool b_ConstructionRestricted = false;

		size_t m_Type = 0;
	};
}