#pragma once
#include "Can.h"

#include "Road.h"
#include "Junction.h"
#include "End.h"

namespace Can::Helper
{
	bool CheckBoundingBoxHit(const glm::vec3& rayStartPoint, const glm::vec3& ray, const glm::vec3& least, const glm::vec3& most);

	glm::vec3 RayPlaneIntersection(const glm::vec3& X, const glm::vec3& v, const glm::vec3& C, const glm::vec3& n);

	glm::vec2 LineSLineSIntersection(glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3);

	bool RayTriangleIntersection(const glm::vec3& camPos, const glm::vec3& ray, const glm::vec3& A, const glm::vec3& B, const glm::vec3& C, const glm::vec3& normal);

	std::vector<std::string> GetFiles(const std::string& folder, const std::string& filter, const std::string& fileType);

	void LevelTheTerrain(const glm::vec2& startIndex, const glm::vec2& endIndex, const glm::vec3& startCoord, const glm::vec3& endCoord, Object* terrain, float width);

	void GenerateTJunction(Object* roadP, Object* endP, Object* junctionP, int snappedRoadIndex, const glm::vec3& startCoord, const glm::vec3& junctionCoord, const std::string& shaderPath, const std::string& texturePath, std::vector<Road*>& roads);

	Ref<Prefab> GetPrefabForTerrain(const std::string& texturePath);

	glm::vec2 RotateAPointAroundAPoint(const glm::vec2& p1, const glm::vec2& p2, float angleInRadians);

	struct sort_with_angle
	{
		inline bool operator() (const Road* road1, const Road* road2)
		{
			float roadR1;
			float roadR2;
			if (
				road1->endJunction != nullptr &&
				road2->endJunction != nullptr &&
				road1->endJunction == road2->endJunction
				)
			{
				roadR1 = road1->rotation.z + glm::radians(180.0f);
				roadR2 = road2->rotation.z + glm::radians(180.0f);
			}
			else if (
				road1->endJunction != nullptr &&
				road2->startJunction != nullptr &&
				road1->endJunction == road2->startJunction
				)
			{
				roadR1 = road1->rotation.z + glm::radians(180.0f);
				roadR2 = road2->rotation.z;
			}
			else if (
				road1->startJunction != nullptr &&
				road2->endJunction != nullptr &&
				road1->startJunction == road2->endJunction
				)
			{
				roadR1 = road1->rotation.z;
				roadR2 = road2->rotation.z + glm::radians(180.0f);
			}
			else if (
				road1->startJunction != nullptr &&
				road2->startJunction != nullptr &&
				road1->startJunction == road2->startJunction
				)
			{
				roadR1 = road1->rotation.z;
				roadR2 = road2->rotation.z;
			}
			else
			{
				roadR1 = 0.001f;
				roadR2 = 0.002f;
			}

			return (roadR1 < roadR2);
		}
	};
}