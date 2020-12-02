#include "canpch.h"
#include "RoadSegment.h"

#include "Can/Renderer/Object.h"
#include "Can/Math.h"
#include "Helper.h"

namespace Can
{
	RoadSegment::RoadSegment(const std::array<Prefab*, 3>& type, const std::array<glm::vec3, 4>& curvePoints)
		: Type(type)
		, CurvePoints(curvePoints)
		, Directions({
			glm::normalize(curvePoints[1] - curvePoints[0]),
			glm::normalize(curvePoints[2] - curvePoints[3])
			})
	{
		glm::vec2 dir = glm::normalize(glm::vec2{ Directions[0].x, Directions[0].z });
		Rotations[0].y = glm::acos(dir.x) * ((float)(dir.y > 0.0f) * 2.0f - 1.0f);
		glm::vec3 dirR = glm::rotate(Directions[0], -Rotations[0].y, glm::vec3{ 0.0f, 1.0f, 0.0f });
		dir = glm::normalize(glm::vec2{ dirR.x, dirR.y });
		Rotations[0].x = glm::acos(dir.x) * ((float)(dir.y < 0.0f) * 2.0f - 1.0f);

		dir = glm::normalize(glm::vec2{ Directions[1].x, Directions[1].z });
		Rotations[1].y = glm::acos(dir.x) * ((float)(dir.y > 0.0f) * 2.0f - 1.0f);
		dirR = glm::rotate(Directions[1], -Rotations[1].y, glm::vec3{ 0.0f, 1.0f, 0.0f });
		dir = glm::normalize(glm::vec2{ dirR.x, dirR.y });
		Rotations[1].x = glm::acos(dir.x) * ((float)(dir.y < 0.0f) * 2.0f - 1.0f);

		Construct();
	}
	RoadSegment::~RoadSegment()
	{
		delete object;
	}

	void RoadSegment::Construct()
	{
		float lengthRoad = Type[0]->boundingBoxM.x - Type[0]->boundingBoxL.x;

		float l = glm::length(CurvePoints[3] - CurvePoints[0]);
		size_t count = 1;
		while (l > lengthRoad)
		{
			count *= 2;
			glm::vec3 p = Math::CubicCurve<float>(CurvePoints, 1.0f / count);
			l = glm::length(p - CurvePoints[0]);
		}
		if (count > 1) count /= 2;

		while (l > lengthRoad)
		{
			count++;
			glm::vec3 p = Math::CubicCurve<float>(CurvePoints, 1.0f / count);
			l = glm::length(p - CurvePoints[0]);
		}
		if (count > 1) count--;


		size_t prefabIndexCount = Type[0]->indexCount;
		size_t indexCount = prefabIndexCount * count;
		size_t vertexCount = indexCount * (3 + 2 + 3);
		glm::vec3 p1 = CurvePoints[0];
		TexturedObjectVertex* TOVertices = new TexturedObjectVertex[indexCount];

		for (int c = 0; c < count; c++)
		{
			glm::vec3 p2 = Math::CubicCurve<float>(CurvePoints, (c + 1.0f) / count);
			glm::vec3 vec = p2 - p1;
			float length = glm::length(vec);
			glm::vec3 dir = vec / length;

			float scale = length / lengthRoad;
			float rot = glm::acos(dir.x) * ((float)(dir.z < 0.0f) * 2.0f - 1.0f);

			for (int i = 0; i < prefabIndexCount; i++)
			{
				size_t offset = c * prefabIndexCount + i;
				size_t index = i * 9;

				glm::vec2 point{
					Type[0]->vertices[index + 0] * scale,
					Type[0]->vertices[index + 2]
				};

				point = Helper::RotateAPointAroundAPoint(point, -rot);

				TOVertices[offset].Position.x = point.x + p1.x - CurvePoints[0].x;
				TOVertices[offset].Position.y = Type[0]->vertices[index + 1] + p1.y - CurvePoints[0].y;
				TOVertices[offset].Position.z = point.y + p1.z - CurvePoints[0].z;
				TOVertices[offset].UV.x = Type[0]->vertices[index + 3];
				TOVertices[offset].UV.y = Type[0]->vertices[index + 4];
				TOVertices[offset].Normal.x = Type[0]->vertices[index + 5];
				TOVertices[offset].Normal.y = Type[0]->vertices[index + 6];
				TOVertices[offset].Normal.z = Type[0]->vertices[index + 7];
				TOVertices[offset].TextureIndex = Type[0]->vertices[index + 8];
			}
			p1 = p2;
		}
		Prefab* newPrefab = new Prefab(Type[0]->objectPath, Type[0]->shaderPath, Type[0]->texturePath, (float*)TOVertices, indexCount);

		for (int c = 0; c < count; c++)
		{
			glm::vec3 p2 = Math::CubicCurve<float>(CurvePoints, (c + 1.0f) / count);
			glm::vec3 vec = p2 - p1;
			float length = glm::length(vec);
			glm::vec3 dir = vec / length;

			float scale = length / lengthRoad;

			for (int i = 0; i < prefabIndexCount; i++)
			{
				size_t offset = c * prefabIndexCount + i;
				newPrefab->boundingBoxL.x = std::min(newPrefab->boundingBoxL.x, TOVertices[offset].Position.x);
				newPrefab->boundingBoxL.y = std::min(newPrefab->boundingBoxL.y, TOVertices[offset].Position.y);
				newPrefab->boundingBoxL.z = std::min(newPrefab->boundingBoxL.z, TOVertices[offset].Position.z);
				newPrefab->boundingBoxM.x = std::max(newPrefab->boundingBoxM.x, TOVertices[offset].Position.x);
				newPrefab->boundingBoxM.y = std::max(newPrefab->boundingBoxM.y, TOVertices[offset].Position.y);
				newPrefab->boundingBoxM.z = std::max(newPrefab->boundingBoxM.z, TOVertices[offset].Position.z);
			}
		}

		object = new Object(newPrefab, Type[0], CurvePoints[0], glm::vec3(1.0f), glm::vec3(0.0f));
	}
	void RoadSegment::ReConstruct()
	{
	}
	void RoadSegment::ChangeType(Prefab* type)
	{
	}

	void RoadSegment::SetStartPosition(const glm::vec3& position)
	{
	}
	void RoadSegment::SetEndPosition(const glm::vec3& position)
	{
	}
}