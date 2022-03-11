#pragma once

#include "Can/Renderer/Object.h"

namespace Can
{
	class GameScene;
	class RoadSegment;
	class RoadNode;

	enum class RoadConstructionMode :u8
	{
		None,
		Straight,
		QuadraticCurve,
		CubicCurve,
		Change,
		Destruct
	};

	enum class RoadSnapOptions
	{
		SNAP_TO_ROAD = 0b00001,
		SNAP_TO_LENGTH = 0b00010,
		SNAP_TO_HEIGHT = 0b00100,
		SNAP_TO_ANGLE = 0b01000,
		SNAP_TO_GRID = 0b10000
	};
	enum class RoadRestrictions
	{
		RESTRICT_SMALL_ANGLES = 0x1,
		RESTRICT_SHORT_LENGTH = 0x2,
		RESTRICT_COLLISIONS = 0x4
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

		bool check_road_road_collision(const std::array<v3, 2>& bounding_box, const std::vector<std::array<v3, 3>>& bounding_polygon);
		bool check_road_building_collision(class Building* building, const std::array<v3, 2>& bounding_box, const std::vector<std::array<v3, 3>>& bounding_polygon);
		bool check_road_tree_collision(Object* tree, const std::array<v3, 2>& bounding_box, const std::vector<std::array<v3, 3>>& bounding_polygon);

		void highlight_road_building_collisions(const std::array<v3, 2>& bounding_box, const std::vector<std::array<v3, 3>>& bounding_polygon);
		void highlight_road_tree_collisions(const std::array<v3, 2>& bounding_box, const std::vector<std::array<v3, 3>>& bounding_polygon);

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
		u8 RemoveRoadSegment(u64 roadSIndex, u64 roadNode);

		SnapInformation CheckSnapping(const v3& prevLocation);

		void ResetStates();
	private:
		void SnapToGrid(v3& prevLocation);
		void SnapToRoad(v3& prevLocation, bool isStart);
		void SnapToHeight(const std::vector<u8>& indices, u8 index, v3& AB);
		void SnapToAngle(v3& AB, s64 snappedNode, s64 snappedRoadSegment, f32 snappedT);
		void ResetGuideLines();
		bool RestrictSmallAngles(v2 direction, s64 snappedNode, s64 snappedRoadSegment, f32 snappedT);

	public:

		u8 snapFlags = 
			(u8)RoadSnapOptions::SNAP_TO_ROAD & 
			(u8)RoadSnapOptions::SNAP_TO_ANGLE;

		u8 restrictionFlags =
			(u8)RoadRestrictions::RESTRICT_COLLISIONS &
			(u8)RoadRestrictions::RESTRICT_SHORT_LENGTH &
			(u8)RoadRestrictions::RESTRICT_SMALL_ANGLES;

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