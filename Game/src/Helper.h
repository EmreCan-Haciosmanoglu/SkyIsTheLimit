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

	 Object* ConstructObject(const std::string& shaderPath, const std::string& texturePath, std::vector < glm::vec3 >& vertices, std::vector < glm::vec2 >& uvs, std::vector < glm::vec3 >& normals);

	std::vector<std::string> GetFiles(const std::string& folder, const std::string& filter, const std::string& fileType);
	
	void LevelTheTerrain(const glm::vec2& startIndex, const glm::vec2& endIndex, const glm::vec3& startCoord, const glm::vec3& endCoord,  Object* terrain, float width);

	void GenerateTJunction( Object* roadP,  Object* endP,  Object* junctionP, int snappedRoadIndex, const glm::vec3& startCoord, const glm::vec3& junctionCoord, const std::string& shaderPath, const std::string& texturePath, std::vector<Road*>& roads);

	void UpdateTheJunction(Junction* junction,  Object* prefab, const std::string& shaderPath, const std::string& texturePath);

	void ReconstructRoad(Road* road,  Object* prefab, const std::string& shaderPath, const std::string& texturePath);
	Ref<Prefab> GetPrefabForTerrain(const std::string& texturePath);

	glm::vec2 RotateAPointAroundAPoint(const glm::vec2& p1, const glm::vec2& p2, float angleInRadians);

	struct sort_with_angle
	{
		inline bool operator() (const Road* road1, const Road* road2)
		{
			glm::vec3 R0R1_1;
			glm::vec3 R0R1_2;
			if (
				road1->endJunction != nullptr &&
				road2->endJunction != nullptr &&
				road1->endJunction == road2->endJunction
				)
			{
				R0R1_1 = road1->startPos - road1->endPos;
				R0R1_2 = road2->startPos - road2->endPos;
			}
			else if (
				road1->endJunction != nullptr &&
				road2->startJunction != nullptr &&
				road1->endJunction == road2->startJunction
				)
			{
				R0R1_1 = road1->startPos - road1->endPos;
				R0R1_2 = road2->endPos - road2->startPos;
			}
			else if (
				road1->startJunction != nullptr &&
				road2->endJunction != nullptr &&
				road1->startJunction == road2->endJunction
				)
			{
				R0R1_1 = road1->endPos - road1->startPos;
				R0R1_2 = road2->startPos - road2->endPos;
			}
			else if (
				road1->startJunction != nullptr &&
				road2->startJunction != nullptr &&
				road1->startJunction == road2->startJunction
				)
			{
				R0R1_1 = road1->endPos - road1->startPos;
				R0R1_2 = road2->endPos - road2->startPos;
			}
			else
			{
				R0R1_1 = { 0.01f, 1.0f, 0.01f };
				R0R1_2 = { 0.01f, 1.0f, 0.01f };
			}

			float ed1 = R0R1_1.x <= 0.0f ? 180.0f : 0.0f;
			float ed2 = R0R1_2.x <= 0.0f ? 180.0f : 0.0f;

			float angleR0R1_1 = std::fmod(glm::degrees(glm::atan(-R0R1_1.z / R0R1_1.x)) + ed1 + 360.0f, 360.0f);
			float angleR0R1_2 = std::fmod(glm::degrees(glm::atan(-R0R1_2.z / R0R1_2.x)) + ed2 + 360.0f, 360.0f);

			return (angleR0R1_1 < angleR0R1_2);
		}
	};
}