#include "canpch.h"
#include "Junction.h"

#include "Road.h"
#include "Helper.h"

namespace Can
{
	Junction::Junction(const glm::vec3& position)
		: position(position)
	{
	}
	Junction::Junction(const std::vector<Road*>& connectedRoads, const glm::vec3& position)
		: position(position)
		, connectedRoads(connectedRoads)
	{
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

			glm::vec3 R1_S = this == r1->startJunction ? r1->GetStartPosition() : r1->GetEndPosition();
			glm::vec3 R2_S = this == r2->startJunction ? r2->GetStartPosition() : r2->GetEndPosition();

			glm::vec3 r1Direction = this == r1->startJunction ? r1->direction : -r1->direction;
			glm::vec3 r2Direction = this == r2->startJunction ? r2->direction : -r2->direction;

			float r1width = r1->object->prefab->boundingBoxM.z - r1->object->prefab->boundingBoxL.z;
			float r2width = r2->object->prefab->boundingBoxM.z - r2->object->prefab->boundingBoxL.z;


			r1Direction.y = 0.0f;
			r2Direction.y = 0.0f;

			glm::vec3 shiftR1Dir = glm::normalize(glm::rotate(r1Direction, glm::radians(-90.0f), glm::vec3{ 0.0f, 1.0f, 0.0f }));
			glm::vec3 shiftR2Dir = glm::normalize(glm::rotate(r2Direction, glm::radians(+90.0f), glm::vec3{ 0.0f, 1.0f, 0.0f }));

			glm::vec3 shiftR1Amount = shiftR1Dir * r1width / 2.0f;
			glm::vec3 shiftR2Amount = shiftR2Dir * r2width / 2.0f;

			float angleDiff = (r1Direction.x / r1Direction.z) - (r2Direction.x / r2Direction.z);
			if (angleDiff < 0.0001f && angleDiff > -0.0001f)
			{
				Intersections.push_back({ position.x + shiftR1Amount.x, 0.0f, position.z + shiftR1Amount.z });
				continue;
			}

			R1_S += shiftR1Amount;
			R2_S += shiftR2Amount;

			r2Direction = glm::rotate(r2Direction, glm::radians(90.0f), glm::vec3{ 0.0f, 1.0f, 0.0f });
			glm::vec3 I = Helper::RayPlaneIntersection(R1_S, r1Direction, R2_S, r2Direction);

			Intersections.push_back(I);
		}



		glm::vec2 center = { 0.0f, 0.0f };
		for (size_t i = 0; i < roadCount; i++)
		{
			Road* r = connectedRoads[i];
			Prefab* prefab = r->type[1];
			size_t indexCount = prefab->indexCount;
			float roadWidth = prefab->boundingBoxM.z - prefab->boundingBoxL.z;
			float roadLength = prefab->boundingBoxM.x - prefab->boundingBoxL.x;

			//float* pieceVertices = new float[indexCount * (3 + 2 + 3)];
			TexturedObjectVertex* TOVertices = new TexturedObjectVertex[indexCount];
			float* prefabVerticies = prefab->vertices;

			glm::vec3 intersection1 = Intersections[i];
			glm::vec3 intersection2 = Intersections[((roadCount + i) - 1) % roadCount];

			glm::vec3 R1 = this == r->startJunction ? r->GetEndPosition() : r->GetStartPosition();

			glm::vec3 JR1 = glm::normalize(R1 - position);
			JR1.y = 0.0f;
			JR1 = glm::normalize(JR1);
			glm::vec3 shiftAmount = glm::rotate(JR1, glm::radians(90.0f), glm::vec3{ 0.0f, 1.0f, 0.0f }) * roadWidth / 2.0f;

			glm::vec3 Jp = position + shiftAmount;
			glm::vec3 Jn = position - shiftAmount;
			glm::vec3 R1p = R1 + shiftAmount;
			glm::vec3 R1n = R1 - shiftAmount;

			float lengthJP = glm::length(R1p - Jp) - glm::length(R1p - intersection2);
			float lengthJN = glm::length(R1n - Jn) - glm::length(R1n - intersection1);

			float l = 0.0f;
			if (lengthJN < lengthJP)
			{
				float offsetLength = lengthJP + roadLength;
				glm::vec3 temp = position + JR1 * offsetLength;
				l = lengthJP;
				if (this == r->startJunction)
					r->SetStartPosition(temp);
				else
					r->SetEndPosition(temp);
			}
			else
			{
				float offsetLength = lengthJN + roadLength;
				glm::vec3 temp = position + JR1 * offsetLength;
				l = lengthJN;
				if (this == r->startJunction)
					r->SetStartPosition(temp);
				else
					r->SetEndPosition(temp);
			}


			float angle = r->rotation.y + glm::radians(r->startJunction == this ? 0.0f : 180.0f);

			for (size_t j = 0; j < indexCount; j++)
			{
				size_t index = j * (int)(sizeof(TexturedObjectVertex) / sizeof(float));
				glm::vec2 point = {
					prefabVerticies[index + 0],
					prefabVerticies[index + 2]
				};
				if (point.x < 0.001f)
				{
					float percent = std::abs(point.y / (roadWidth / 2.0f));
					if (point.y < 0.0f)
						point.x += percent * lengthJP;
					else if (point.y > 0.0f)
						point.x += percent * lengthJN;
				}
				else
				{
					point.x += l;
				}

				glm::vec2 rotatedPoint = Helper::RotateAPointAroundAPoint(point, center, -angle);

				TOVertices[j].Position.x = rotatedPoint.x;
				TOVertices[j].Position.y = prefabVerticies[index + 1];
				TOVertices[j].Position.z = rotatedPoint.y;
				TOVertices[j].UV.x = prefabVerticies[index + 3];
				TOVertices[j].UV.y = prefabVerticies[index + 4];
				TOVertices[j].Normal.x = prefabVerticies[index + 5];
				TOVertices[j].Normal.x = prefabVerticies[index + 6];
				TOVertices[j].Normal.x = prefabVerticies[index + 7];
				TOVertices[j].TextureIndex = 0.0f;
			}

			Prefab* newPrefab = new Prefab(prefab->objectPath, prefab->shaderPath, prefab->texturePath, (float*)TOVertices, indexCount);
			junctionPieces.push_back(new Object(newPrefab, prefab, position));
		}
	}
	void Junction::ReconstructObject()
	{
		for (Object* obj : junctionPieces)
		{
			delete obj;
			obj = nullptr;
		}
		junctionPieces.clear();

		ConstructObject();
	}
}