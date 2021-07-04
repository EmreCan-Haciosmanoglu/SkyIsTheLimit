#pragma once
#include "Can.h"

#include "Types/RoadSegment.h"
#include "Scenes/GameScene.h"

namespace Can::Helper
{
#define TERRAIN_SCALE_DOWN 10.0f
#define COLOR_COUNT 5

	bool CheckBoundingBoxHit(const v3& rayStartPoint, const v3& ray, const v3& least, const v3& most);

	v2 CheckRotatedRectangleCollision(
		const v2& r1l,
		const v2& r1m,
		f32 rot1,
		const v2& pos1,
		const v2& r2l,
		const v2& r2m,
		f32 rot2,
		const v2& pos2
	);

	v3 GetRayHitPointOnTerrain(void* s, const v3& cameraPosition, const v3& cameraDirection);

	v3 RayPlaneIntersection(const v3& X, const v3& v, const v3& C, const v3& n);

	f32 DistanceBetweenLineSLineS(v2 p1, v2 p2, v2 p3, v2 p4);

	bool RayTriangleIntersection(const v3& camPos, const v3& ray, const v3& A, const v3& B, const v3& C, const v3& normal, v3& intersection);

	v2 RotateAPointAroundAPoint(const v2& p1, f32 angleInRadians, const v2& p2 = { 0.0f, 0.0f });

	void LevelTheTerrain(const v2& startIndex, const v2& endIndex, const v3& startCoord, const v3& endCoord, Object* terrain, f32 width, bool reset);
	void UpdateTheTerrain(const RoadSegment& rs, bool reset);

	Prefab* GetPrefabForTerrain(const std::string& texturePath);

	std::vector<std::string> GetFiles(const std::string& folder, const std::string& filter, const std::string& fileType);

	struct sort_by_angle
	{
		inline bool operator() (u64 roadSegment1, u64 roadSegment2)
		{
			auto& segments = GameScene::ActiveGameScene->m_RoadManager.m_Segments;
			RoadSegment& rs1 = segments[roadSegment1];
			RoadSegment& rs2 = segments[roadSegment2];
			f32 roadSegmentR1 = 0.002f;
			f32 roadSegmentR2 = 0.001f;
			if (rs1.EndNode == rs2.EndNode)
			{
				roadSegmentR1 = rs1.GetEndRotation().y;
				roadSegmentR2 = rs2.GetEndRotation().y;
			}
			else if (rs1.EndNode == rs2.StartNode)
			{
				roadSegmentR1 = rs1.GetEndRotation().y;
				roadSegmentR2 = rs2.GetStartRotation().y;
			}
			else if (rs1.StartNode == rs2.EndNode)
			{
				roadSegmentR1 = rs1.GetStartRotation().y;
				roadSegmentR2 = rs2.GetEndRotation().y;
			}
			else if (rs1.StartNode == rs2.StartNode)
			{
				roadSegmentR1 = rs1.GetStartRotation().y;
				roadSegmentR2 = rs2.GetStartRotation().y;
			}
			else
			{
				std::cout << "Why are you here" << std::endl;
			}
			roadSegmentR1 = std::fmod(roadSegmentR1 + glm::radians(360.0f), glm::radians(360.0f));
			roadSegmentR2 = std::fmod(roadSegmentR2 + glm::radians(360.0f), glm::radians(360.0f));
			return (roadSegmentR1 < roadSegmentR2);
		}
	};
}