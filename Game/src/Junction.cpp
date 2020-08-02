#include "canpch.h"
#include "Junction.h"

#include "Road.h"
#include "Helper.h"

namespace Can
{
	Junction::Junction(const glm::vec3& position)
		: position(position)
	{
		ConstructObject();
	}
	Junction::Junction(const std::vector<Object*>& junctionPieces, const glm::vec3& position)
		: position(position)
		, junctionPieces(junctionPieces)
	{
	}

	void Junction::ConstructObject()
	{
		glm::vec2 posIn2D = {
			position.x ,
			position.z
		};

		std::sort(connectedRoads.begin(), connectedRoads.end(), Helper::sort_with_angle());

		std::vector<glm::vec3> Intersections;

		size_t roadCount = connectedRoads.size();
		for (size_t i = 0; i < roadCount; i++)
		{
			size_t iNext = (i + 1) % roadCount;
			Road* r1 = connectedRoads[i];
			Road* r2 = connectedRoads[iNext];

			glm::vec3 r1Direction = this == r1->startJunction ? r1->direction : -r1->direction;
			glm::vec3 r2Direction = this == r2->startJunction ? r2->direction : -r2->direction;
			glm::vec3 R1_S = this == r1->startJunction ? r1->startPosition : r1->endPosition;
			glm::vec3 R2_S = this == r2->startJunction ? r2->startPosition : r2->endPosition;
			float roadAngle1 = this == r1->startJunction ? r1->rotation.z : r1->rotation.z + glm::radians(180.0f);
			float roadAngle2 = this == r2->startJunction ? r2->rotation.z : r2->rotation.z + glm::radians(180.0f);
			float r1width = r1->object->prefab->boundingBoxM.x - r1->object->prefab->boundingBoxL.x;
			float r2width = r2->object->prefab->boundingBoxM.x - r2->object->prefab->boundingBoxL.x;


			r1Direction.y = 0.0f;
			r2Direction.y = 0.0f;

			glm::vec3 shiftR1Amount = glm::normalize(glm::rotate(r1Direction, glm::radians(-90.0f), glm::vec3{ 0.0f, 0.0f, 1.0f })) * r1width / 2.0f;
			glm::vec3 shiftR2Amount = glm::normalize(glm::rotate(r2Direction, glm::radians(+90.0f), glm::vec3{ 0.0f, 0.0f, 1.0f })) * r2width / 2.0f;

			R1_S += shiftR1Amount;
			R2_S += shiftR2Amount;

			r2Direction = glm::rotate(r2Direction, glm::radians(90.0f), { 0.0f, 0.0f, 1.0f });
			glm::vec3 I = Helper::RayPlaneIntersection(R1_S, r1Direction, R2_S, r2Direction);

			Intersections.push_back(I);
		}



		glm::vec2 center = { 0.0f, 0.0f };
		for (size_t i = 0; i < roadCount; i++)
		{
			Road* r = connectedRoads[i];
			Ref<Prefab>& prefab = r->type[1];
			size_t indexCount = prefab->indexCount;
			float roadWidth = prefab->boundingBoxM.x - prefab->boundingBoxL.x;
			float roadLength = prefab->boundingBoxM.y - prefab->boundingBoxL.y;

			float* pieceVertices = new float[indexCount * (3 + 2 + 3)];
			float* prefabVerticies = prefab->vertices;

			glm::vec3 intersection1 = Intersections[i];
			glm::vec3 intersection2 = Intersections[(roadCount + i - 1) % roadCount];

			glm::vec3 R1 = this == r->startJunction ? r->endPosition : r->startPosition;

			glm::vec3 JR1 = glm::normalize(R1 - position);
			JR1.y = 0.0f;
			glm::vec3 shiftAmount = glm::rotate(JR1, glm::radians(90.0f), glm::vec3{ 0.0f, 0.0f, 1.0f }) * roadWidth / 2.0f;

			glm::vec3 Jp = position + shiftAmount;
			glm::vec3 Jn = position - shiftAmount;

			float lengthJP = glm::length(Jp - intersection2);
			float lengthJN = glm::length(Jn - intersection1);

			float l = 0.0f;
			if (lengthJN < lengthJP)
			{
				float offsetLength = lengthJP + roadLength;
				glm::vec3 temp = position + JR1 * offsetLength;
				l = lengthJP;
				if (this == r->startJunction)
					r->startPosition = temp;
				else
					r->endPosition = temp;
			}
			else
			{
				float offsetLength = lengthJN + roadLength;
				glm::vec3 temp = position + JR1 * offsetLength;
				l = lengthJN;
				if (this == r->startJunction)
					r->startPosition = temp;
				else
					r->endPosition = temp;
			}

			r->ReconstructObject(r->type[0]);

			float angle = r->rotation.z + glm::radians(r->startJunction == this ? 0.0f : 180.0f);

			for (size_t j = 0; j < indexCount; j++)
			{
				size_t index = j * 8;
				glm::vec2 point = {
					prefabVerticies[index + 0],
					prefabVerticies[index + 2]
				};
				if (prefabVerticies[index + 2] < 0.001f && prefabVerticies[index + 2] > -0.001f)
				{
					float percent = std::abs(point.x / (roadWidth / 2.0f));
					if (prefabVerticies[index + 0] < 0.0f)
						point.y += percent * lengthJP;
					else if (prefabVerticies[index + 0] > 0.0f)
						point.y += percent * lengthJN;
				}
				else
				{
					point.y += l;
				}

				glm::vec2 rotatedPoint = Helper::RotateAPointAroundAPoint(point, center, -angle);

				pieceVertices[index + 0] = rotatedPoint.x;
				pieceVertices[index + 1] = prefabVerticies[index + 1];
				pieceVertices[index + 2] = rotatedPoint.y;
				pieceVertices[index + 3] = prefabVerticies[index + 3];
				pieceVertices[index + 4] = prefabVerticies[index + 4];
				pieceVertices[index + 5] = prefabVerticies[index + 5];
				pieceVertices[index + 6] = prefabVerticies[index + 6];
				pieceVertices[index + 7] = prefabVerticies[index + 7];
			}

			Ref<Prefab> newPrefab = CreateRef<Prefab>(prefab->objectPath, prefab->shaderPath, prefab->texturePath, pieceVertices, indexCount);
			junctionPieces.push_back(new Object(newPrefab, prefab, position));
		}

	}
	void Junction::ReconstructObject()
	{
		for (Object* obj : junctionPieces)
			delete obj;
		junctionPieces.clear();

		ConstructObject();
	}
}