#pragma once

#include "Can/Renderer/Object.h"

namespace Can
{
	class GameScene;
	class RoadSegment;
	class RoadNode;

	enum class RoadConstructionMode
	{
		None,
		Straight,
		QuadraticCurve,
		CubicCurve,
		Change,
		Destruct
	};

	struct SnapInformation
	{
		bool snapped = false;
		v3 location{ 0.0f, 0.0f, 0.0f };
		s64 segment = -1;
		s64 node = -1;
		f32 T = 0.0f;
		s8 elevation_type = 0;
	};

	class RoadManager
	{
	public:
		RoadManager(GameScene* scene);
		~RoadManager();

		void OnUpdate(v3& prevLocation, f32 ts);
		void OnUpdate_Straight(v3& prevLocation, f32 ts);
		void OnUpdate_QuadraticCurve(v3& prevLocation, f32 ts);
		void OnUpdate_CubicCurve(v3& prevLocation, f32 ts);
		void OnUpdate_Change(v3& prevLocation, f32 ts);
		void OnUpdate_Destruction(v3& prevLocatio, f32 ts);

		void DrawStraightGuidelines(const v3& pointA, const v3& pointB, s8 eA, s8 eB);
		void DrawCurvedGuidelines(const std::array<v3, 4>& curvePoints);

		bool CheckStraightRoadRoadCollision(const std::array<std::array<v2, 3>, 2>& polygon);
		void CheckStraightRoadBuildingCollision(const std::array<std::array<v2, 3>, 2>& polygon);
		void CheckStraightRoadTreeCollision(const std::array<std::array<v2, 3>, 2>& polygon);

		// need template
		bool CheckRoadRoadCollision(const std::array<std::array<v2, 3>, 2>& box, const std::array<std::array<v2, 3>, (10 - 1) * 2>& polygon);
		void CheckRoadBuildingCollision(const std::array<std::array<v2, 3>, 2>& box, const std::array<std::array<v2, 3>, (10 - 1) * 2>& polygon);
		void CheckRoadTreeCollision(const std::array<std::array<v2, 3>, 2>& box, const std::array<std::array<v2, 3>, (10 - 1) * 2>& polygon);

		bool OnMousePressed(MouseCode button);
		bool OnMousePressed_Straight();
		bool OnMousePressed_QuadraticCurve();
		bool OnMousePressed_CubicCurve();
		bool OnMousePressed_Change();
		bool OnMousePressed_Destruction();

		void SetType(u64 type);
		inline u64 GetType() { return m_Type; }

		void SetConstructionMode(RoadConstructionMode mode);

		inline const RoadConstructionMode GetConstructionMode() const { return m_ConstructionMode; }
		inline RoadConstructionMode GetConstructionMode() { return m_ConstructionMode; }

		//inline const std::vector<RoadSegment*>& GetRoadSegments() const { return m_RoadSegments; }
		//inline std::vector<RoadSegment*>& GetRoadSegments() { return m_RoadSegments; }

		u64 AddRoadSegment(const std::array<v3, 4>& curvePoints, s8 elevation_type);
		void RemoveRoadSegment(u64 roadSIndex);

		SnapInformation CheckSnapping(const v3& prevLocation);

		void ResetStates();
	private:
		void SnapToGrid(v3& prevLocation);
		void SnapToRoad(v3& prevLocation, bool isStart);
		void SnapToHeight(const std::vector<u8>& indices, u8 index, v3& AB);
		void SnapToAngle(v3& AB, s64 snappedNode, s64 snappedRoadSegment, f32 snappedT);
		void ResetGuideLines();
		bool RestrictSmallAngles(v3 direction, s64 snappedNode, s64 snappedRoadSegment, f32 snappedT);

	public:

		u8 snapFlags = 0b01001;
#define SNAP_TO_ROAD   0b00001
#define SNAP_TO_LENGTH 0b00010
#define SNAP_TO_HEIGHT 0b00100
#define SNAP_TO_ANGLE  0b01000
#define SNAP_TO_GRID   0b10000

		u8 restrictionFlags = 0b111;
#define RESTRICT_SMALL_ANGLES 0x1
#define RESTRICT_SHORT_LENGTH 0x2
#define RESTRICT_COLLISIONS   0x4

		std::array<u8, 4> cubicCurveOrder = { 0, 1, 2, 3 };

		GameScene* m_Scene = nullptr;
		RoadConstructionMode m_ConstructionMode = RoadConstructionMode::None;

		std::vector<RoadSegment> m_Segments{};
		std::vector<RoadNode> m_Nodes{};

		int m_ConstructionPhase = 0;

		// Transforms
		std::array<v3, 4> m_ConstructionPositions = {
			v3(0.0f),
			v3(0.0f),
			v3(0.0f),
			v3(0.0f)
		};

		std::array<s8, 4> m_Elevationtypes = { 0, 0, 0, 0 };

		std::array<f32, 4> m_Elevations = { 0.0f, 0.0f, 0.0f, 0.0f };
		f32 m_CurrentElevation = 0.0f;

		// Start Snap
		bool b_ConstructionStartSnapped = false;
		s64 m_StartSnappedSegment = -1;
		s64 m_StartSnappedNode = -1;
		f32 m_StartSnappedT = 0.0f;

		// End Snap
		bool b_ConstructionEndSnapped = false;
		s64 m_EndSnappedSegment = -1;
		s64 m_EndSnappedNode = -1;
		f32 m_EndSnappedT = 0.0f;

		// Destruction Snap
		s64 m_DestructionSegment = -1;
		s64 m_DestructionNode = -1;

		// Selection
		s64 selected_road_segment = -1;
		s64 selected_road_node = -1;

		std::vector<std::vector<Object*>> m_GroundGuidelines{};
		std::vector<std::vector<Object*>> m_TunnelGuidelines{};
		std::vector<u64> m_GroundGuidelinesInUse{};
		std::vector<u64> m_TunnelGuidelinesInUse{};

		Object* m_GroundGuidelinesStart = nullptr;
		Object* m_GroundGuidelinesEnd = nullptr;

		Object* m_TunnelGuidelinesStart = nullptr;
		Object* m_TunnelGuidelinesEnd = nullptr;

		bool b_ConstructionRestricted = false;

		u64 m_Type = 0;
	};
}