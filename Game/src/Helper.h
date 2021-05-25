#pragma once
#include "Can.h"

#include "Types/RoadSegment.h"
#include "Scenes/GameScene.h"

namespace Can::Helper
{
#define TERRAIN_SCALE_DOWN 10.0f
#define COLOR_COUNT 5

	bool CheckBoundingBoxHit(const glm::vec3& rayStartPoint, const glm::vec3& ray, const glm::vec3& least, const glm::vec3& most);

	glm::vec2 CheckRotatedRectangleCollision(
		const glm::vec2& r1l,
		const glm::vec2& r1m,
		float rot1,
		const glm::vec2& pos1,
		const glm::vec2& r2l,
		const glm::vec2& r2m,
		float rot2,
		const glm::vec2& pos2
	);

	glm::vec3 GetRayHitPointOnTerrain(void* s, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);

	glm::vec3 RayPlaneIntersection(const glm::vec3& X, const glm::vec3& v, const glm::vec3& C, const glm::vec3& n);

	float DistanceBetweenLineSLineS(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4);

	bool RayTriangleIntersection(const glm::vec3& camPos, const glm::vec3& ray, const glm::vec3& A, const glm::vec3& B, const glm::vec3& C, const glm::vec3& normal, glm::vec3& intersection);

	glm::vec2 RotateAPointAroundAPoint(const glm::vec2& p1, float angleInRadians, const glm::vec2& p2 = { 0.0f, 0.0f });

	void LevelTheTerrain(const glm::vec2& startIndex, const glm::vec2& endIndex, const glm::vec3& startCoord, const glm::vec3& endCoord, Object* terrain, float width);

	Prefab* GetPrefabForTerrain(const std::string& texturePath);

	std::vector<std::string> GetFiles(const std::string& folder, const std::string& filter, const std::string& fileType);

	struct sort_by_angle
	{
		inline bool operator() (u64 roadSegment1, u64 roadSegment2)
		{
			auto& segments = GameScene::ActiveGameScene->m_RoadManager.m_Segments;
			RoadSegment& rs1 = segments[roadSegment1];
			RoadSegment& rs2 = segments[roadSegment2];
			float roadSegmentR1 = 0.002f;
			float roadSegmentR2 = 0.001f;
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