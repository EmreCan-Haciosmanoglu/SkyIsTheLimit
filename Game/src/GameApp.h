#pragma once
#include "Can.h"

#include "TestScene.h"

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

private:
	Can::Object* ConstructObject(
		const std::string& shaderPath,
		const std::string& texturePath,
		std::vector < glm::vec3 >& vertices,
		std::vector < glm::vec2 >& uvs,
		std::vector < glm::vec3 >& normals
	);

	void UploadObject(
		Can::Object* target,
		const char* objectPath,
		const std::string& shaderPath,
		const std::string& texturePath
	);

	void UploadTerrain(
		Can::Object* target,
		const std::string& texturePath
	);

private:

	TestScene* testScene;

	Can::Object* m_TestObject;
	Can::Object* m_Terrain;
};
