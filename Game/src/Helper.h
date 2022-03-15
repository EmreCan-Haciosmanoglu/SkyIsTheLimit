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

	void UpdateTheTerrain(const std::vector < std::array<v3, 3>>& polygon, bool reset);
	void UpdateTheTerrain(RoadSegment* rs, bool reset);

	Prefab* GetPrefabForTerrain(const std::string& texturePath);

	std::vector<std::string> GetFiles(const std::string& folder, const std::string& filter, const std::string& fileType);

	void name_me_normals(u64 w, u64 h, u64 min_x, u64 max_x, u64 min_y, u64 max_y, f32* vertices);
	void name_me_cutting(u64 w, u64 h, v3 AB, v3 current_point, f32* vertices);
	std::array<u64, 4> name_me_digging(u64 w, u64 h, const std::vector<std::array<v3, 3>>& polygon, f32* vertices, bool reset);

	std::string trim_path_and_extension(std::string& path);

	std::vector<u64> get_path(u64 start, u8 dist);
	std::vector<u64> get_path(u64 start, u64 end);

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
	struct sort_by_distance
	{
		inline bool operator() (std::pair<u64, std::vector<u64>> path1, std::pair<u64, std::vector<u64>> path2)
		{
			return path1 < path2;
		}
	};
}