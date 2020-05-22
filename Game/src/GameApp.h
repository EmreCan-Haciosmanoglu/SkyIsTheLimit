#pragma once
#include "Can.h"

#include "TestScene.h"

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

		bool RayTriangleIntersection(
			const glm::vec3& camPos,
			const glm::vec3& ray,
			const glm::vec3& A,
			const glm::vec3& B,
			const glm::vec3& C,
			const glm::vec3& normal
		);

		Can::Object* UploadObject(
			const char* objectPath,
			const std::string& shaderPath,
			const std::string& texturePath
		);

		Can::Object* GenerateStraightRoad(
			const char* objectPath,
			const std::string& shaderPath,
			const std::string& texturePath,
			const glm::vec3& startCoord,
			const glm::vec3& endCoord
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

		Can::Object* m_TestObject;
	};
}