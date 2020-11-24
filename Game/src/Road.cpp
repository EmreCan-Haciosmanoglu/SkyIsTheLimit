#include "canpch.h"
#include "Road.h"

#include "Junction.h"
#include "End.h"

#include "Building.h"

#include "Helper.h"

#include "TestScene.h"

namespace Can
{
	Road::Road(Road* road, const glm::vec3& startPos, const glm::vec3& endPos)
		: startPosition(startPos)
		, endPosition(endPos)
		, direction(glm::normalize(endPos - startPos))
		, length(glm::length(endPos - startPos))
		, type(road->type)
		, typeIndex(road->typeIndex)
	{
		float rotationY = 0.0f;
		float rotationZ = 0.0f;
		if (direction.x < 0.0001f && direction.x > -0.0001f)
		{
			rotationY = ((float)(direction.z > 0.0f) * 2.0f - 1.0f) * glm::radians(90.0f);
			rotationZ = glm::atan(direction.y / direction.z);
		}
		else
		{
			rotationY = glm::atan(direction.z / direction.x) + (float)(direction.x < 0.0f) * glm::radians(180.0f);
			float dot = glm::dot(direction, glm::vec3{ direction.x, 0.0f, direction.z });
			float len = glm::length(glm::vec3{ direction.x, 0.0f, direction.z });
			rotationZ = glm::acos(glm::max(-1.0f, glm::min(dot / len, 1.0f)));
		}
		rotation = { 0.0f, -rotationY, rotationZ };

		ConstructObject(road->type[0]);
	}
	Road::Road(Prefab* prefab, const glm::vec3& startPos, const glm::vec3& endPos, const std::array<Prefab*, 3>& type, size_t typeIndex)
		: startPosition(startPos)
		, endPosition(endPos)
		, direction(glm::normalize(endPos - startPos))
		, length(glm::length(endPos - startPos))
		, type(type)
		, typeIndex(typeIndex)
	{
		float rotationY = 0.0f;
		float rotationZ = 0.0f;
		if (direction.x < 0.0001f && direction.x > -0.0001f)
		{
			rotationY = ((float)(direction.z > 0.0f) * 2.0f - 1.0f) * glm::radians(90.0f);
			rotationZ = glm::atan(direction.y / direction.z);
		}
		else
		{
			rotationY = glm::atan(direction.z / direction.x) + (float)(direction.x < 0.0f) * glm::radians(180.0f);
			float dot = glm::dot(direction, glm::vec3{ direction.x, 0.0f, direction.z });
			float len = glm::length(glm::vec3{ direction.x, 0.0f, direction.z });
			rotationZ = glm::acos(glm::max(-1.0f, glm::min(dot / len, 1.0f)));
		}
		rotation = { 0.0f, -rotationY, rotationZ };

		ConstructObject(prefab);
	}
	Road::Road(Object* object, const glm::vec3& startPos, const glm::vec3& endPos, const std::array<Prefab*, 3>& type, size_t typeIndex)
		: startPosition(startPos)
		, endPosition(endPos)
		, direction(glm::normalize(endPos - startPos))
		, length(glm::length(endPos - startPos))
		, object(object)
		, type(type)
		, typeIndex(typeIndex)
	{
		float rotationY = 0.0f;
		float rotationZ = 0.0f;
		if (direction.x < 0.0001f && direction.x > -0.0001f)
		{
			rotationY = ((float)(direction.z > 0.0f) * 2.0f - 1.0f) * glm::radians(90.0f);
			rotationZ = glm::atan(direction.y / direction.z);
		}
		else
		{
			rotationY = glm::atan(direction.z / direction.x) + (float)(direction.x < 0.0f) * glm::radians(180.0f);
			float dot = glm::dot(direction, glm::vec3{ direction.x, 0.0f, direction.z });
			float len = glm::length(glm::vec3{ direction.x, 0.0f, direction.z });
			rotationZ = glm::acos(glm::max(-1.0f, glm::min(dot / len, 1.0f)));
		}
		rotation = { 0.0f, -rotationY, rotationZ };

	}
	Road::~Road()
	{
		delete object;
	}
	void Road::ConstructObject(Prefab* prefab)
	{
		float lengthRoad = prefab->boundingBoxM.x - prefab->boundingBoxL.x;
		int count = (int)(length / lengthRoad);
		float scale = (length / lengthRoad) / count;

		size_t indexCount = prefab->indexCount * count;
		size_t vertexCount = indexCount * (3 + 2 + 3);

		TexturedObjectVertex* TOVertices = new TexturedObjectVertex[indexCount];
		for (int c = 0; c < count; c++)
		{
			for (int i = 0; i < prefab->indexCount; i++)
			{
				size_t offset = c * prefab->indexCount + i;
				size_t index = i * 9;
				TOVertices[offset].Position.x = scale * (prefab->vertices[index + 0] + (c * lengthRoad));
				TOVertices[offset].Position.y = prefab->vertices[index + 1];
				TOVertices[offset].Position.z = prefab->vertices[index + 2];
				TOVertices[offset].UV.x = prefab->vertices[index + 3];
				TOVertices[offset].UV.y = prefab->vertices[index + 4];
				TOVertices[offset].Normal.x = prefab->vertices[index + 5];
				TOVertices[offset].Normal.y = prefab->vertices[index + 6];
				TOVertices[offset].Normal.z = prefab->vertices[index + 7];
				TOVertices[offset].TextureIndex = prefab->vertices[index + 8];
			}
		}
		Prefab* newPrefab = new Prefab(prefab->objectPath, prefab->shaderPath, prefab->texturePath, (float*)TOVertices, indexCount);
		newPrefab->boundingBoxL = prefab->boundingBoxL;
		newPrefab->boundingBoxM = prefab->boundingBoxM;
		newPrefab->boundingBoxM.x *= count * scale;


		object = new Object(newPrefab, prefab, startPosition, glm::vec3(1.0f), rotation);
	}
	void Road::ReconstructObject(Prefab* prefab)
	{
		if (object)
			delete object;

		ConstructObject(prefab);

		for (size_t i = 0; i < connectedBuildings.size(); i++)
		{
			Building* building = connectedBuildings[i];
			glm::vec3 B = building->position - startPosition;
			float bLength = glm::length(B);

			float angle = glm::acos(glm::dot(direction, B) / bLength);

			float c = bLength * glm::cos(angle);
			if (c <= 0.0f || c >= length)
			{
				auto it = std::find(
					TestScene::m_Buildings.begin(),
					TestScene::m_Buildings.end(),
					building
				);
				connectedBuildings.erase(connectedBuildings.begin() + i);
				building->connectedRoad = nullptr;
				if(TestScene::buildingSnapOptions[0])
					TestScene::m_Buildings.erase(it);
				delete building;
				i--;
			}
		}
	}

	void Road::SetStartPosition(const glm::vec3& pos)
	{
		startPosition = pos;
		direction = glm::normalize(endPosition - startPosition);
		length = glm::length(endPosition - startPosition);
		rotation = { 0.0f, -(glm::atan(direction.z / direction.x) + (direction.x <= 0 ? glm::radians(180.0f) : 0.0f)), glm::atan(direction.y / direction.x) };

		Prefab* type = object->type;
		ReconstructObject(type);
	}
	void Road::SetEndPosition(const glm::vec3& pos)
	{
		endPosition = pos;
		direction = glm::normalize(endPosition - startPosition);
		length = glm::length(endPosition - startPosition);
		rotation = { 0.0f, -(glm::atan(direction.z / direction.x) + (direction.x <= 0 ? glm::radians(180.0f) : 0.0f)), glm::atan(direction.y / direction.x) };

		Prefab* type = object->type;
		ReconstructObject(type);
	}
}