#include "canpch.h"
#include "Road.h"

#include "Junction.h"
#include "End.h"

#include "Helper.h"

namespace Can
{
	Road::Road(const Ref<Prefab>& prefab, const glm::vec3& startPos, const glm::vec3& endPos)
		: startPosition(startPos)
		, endPosition(endPos)
		, direction(glm::normalize(endPos - startPos))
		, rotation({ 0.0f, glm::atan(direction.y / direction.x), glm::atan(direction.z / direction.x) + (direction.x <= 0 ? glm::radians(180.0f) : 0.0f) })
		, length(glm::length(endPos - startPos))
		, object(nullptr)
	{
		ConstructObject(prefab, startPos, endPos);
	}
	Road::Road(Object* object, const glm::vec3& startPos, const glm::vec3& endPos)
		: startPosition(startPos)
		, endPosition(endPos)
		, direction(glm::normalize(endPos - startPos))
		, length(glm::length(endPos - startPos))
		, object(object)
	{
	}
	Road::~Road()
	{
		delete object;
	}
	void Road::ConstructObject(const Ref<Prefab>& prefab)
	{
		float lengthRoad = prefab->boundingBoxM.x - prefab->boundingBoxL.x;
		int count = (int)(length / lengthRoad);
		float scale = (length / lengthRoad) / count;

		size_t indexCount = prefab->indexCount * count;
		size_t vertexCount = indexCount * (3 + 2 + 3);

		float* vertices = new float[vertexCount];
		for (int c = 0; c < count; c++)
		{
			for (int i = 0; i < prefab->indexCount; i++)
			{
				size_t index = i * 8;
				vertices[c * prefab->indexCount * 8 + index + 0] = scale * (prefab->vertices[index + 0] + (c * lengthRoad));
				vertices[c * prefab->indexCount * 8 + index + 1] = prefab->vertices[index + 1];
				vertices[c * prefab->indexCount * 8 + index + 2] = prefab->vertices[index + 2];
				vertices[c * prefab->indexCount * 8 + index + 3] = prefab->vertices[index + 3];
				vertices[c * prefab->indexCount * 8 + index + 4] = prefab->vertices[index + 4];
				vertices[c * prefab->indexCount * 8 + index + 5] = prefab->vertices[index + 5];
				vertices[c * prefab->indexCount * 8 + index + 6] = prefab->vertices[index + 6];
				vertices[c * prefab->indexCount * 8 + index + 7] = prefab->vertices[index + 7];
			}
		}
		Ref<Prefab> newPrefab = CreateRef<Prefab>(prefab->objectPath, prefab->shaderPath, prefab->texturePath, vertices, indexCount);
		newPrefab->boundingBoxL = prefab->boundingBoxL;
		newPrefab->boundingBoxM = prefab->boundingBoxM;
		newPrefab->boundingBoxM.x *= count * scale;


		object = new Object(newPrefab, startPosition, glm::vec3{ 1.0f, 1.0f, 1.0f }, rotation);
	}
	void Road::ReconstructObject(const Ref<Prefab>& prefab)
	{
		if (object)
			delete object;

		ConstructObject(prefab);
	}
}