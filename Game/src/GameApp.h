#pragma once
#include "Can.h"

#include "TestScene.h"
#include "UIScene.h"

namespace Can
{
	class GameApp : public Can::Application
	{
	public:
		GameApp();
		~GameApp();

		bool loadOBJ(
			const char* path,
			std::vector < glm::vec3 >& out_vertices,
			std::vector < glm::vec2 >& out_uvs,
			std::vector < glm::vec3 >& out_normals
		);

		void SetTransform(Can::Object* object, glm::vec3 pos, glm::vec3 scale);

		void SetTransform(Can::Object* object, glm::vec3 pos, glm::vec3 scale, glm::vec3 rotation);

		void GameApp::LevelTheTerrain(const glm::vec2& startIndex, const glm::vec2& endIndex, const glm::vec3& startCoord, const glm::vec3& endCoord, Can::Object* terrain, float width);

		bool RayTriangleIntersection(
			const glm::vec3& camPos,
			const glm::vec3& ray,
			const glm::vec3& A,
			const glm::vec3& B,
			const glm::vec3& C,
			const glm::vec3& normal
		);
		static glm::vec3 GameApp::RayPlaneIntersection(
			const glm::vec3& X,
			const glm::vec3& v,
			const glm::vec3& C,
			const glm::vec3& n
		);

		inline glm::vec2 RotateAPointAroundAPoint(const glm::vec2& p1, const glm::vec2& p2, float angleInRadians )
		{
			return glm::vec2{
				glm::cos(angleInRadians) * (p1.x - p2.x) - glm::sin(angleInRadians) * (p1.y - p2.y) + p2.x,
				glm::sin(angleInRadians) * (p1.x - p2.x) + glm::cos(angleInRadians) * (p1.y - p2.y) + p2.y
			};
		}

		glm::vec2 LineSLineSIntersection(glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3);

		Can::Object* UploadObject(
			const char* objectPath,
			const std::string& shaderPath,
			const std::string& texturePath
		);

		void GenerateTJunction(
			Can::Object* roadP,
			Can::Object* endP,
			Can::Object* junctionP,
			int snappedRoadIndex,
			const glm::vec3& startCoord,
			const glm::vec3& junctionCoord,
			const std::string& shaderPath,
			const std::string& texturePath,
			std::vector<Road*>& roads
		);

		void UpdateTheJunction(
			Junction* junction, 
			Can::Object* prefab,
			const std::string& shaderPath,
			const std::string& texturePath
			);

		void ReconstructRoad(
			Road* road, 
			Can::Object* prefab,
			const std::string& shaderPath,
			const std::string& texturePath
		);

	private:
		Can::Object* ConstructObject(
			const std::string& shaderPath,
			const std::string& texturePath,
			std::vector < glm::vec3 >& vertices,
			std::vector < glm::vec2 >& uvs,
			std::vector < glm::vec3 >& normals
		);

		Can::Object* UploadTerrain(
			const std::string& texturePath
		);

	public:
		Can::Object* m_Terrain;
	private:

		TestScene* testScene;
		UIScene* uiScene;
	};
}