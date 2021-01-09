#include "canpch.h"
#include "Junction.h"

#include "Types/RoadSegment.h"
#include "Helper.h"

namespace Can
{
	Junction::Junction(const glm::vec3& position)
		: position(position)
		, object(nullptr)
	{
	}
	Junction::Junction(const std::vector<RoadSegment*>& connectedRoadSegments, const glm::vec3& position)
		: position(position)
		, object(nullptr)
		, connectedRoadSegments(connectedRoadSegments)
	{
	}
	Junction::Junction(Object* object, const glm::vec3& position)
		: position(position)
		, object(object)
	{
	}

	Junction::~Junction()
	{
		delete object;
	}

	void Junction::ConstructObject()
	{
		glm::vec2 posIn2D = {
			position.x ,
			position.z
		};

		std::sort(connectedRoadSegments.begin(), connectedRoadSegments.end(), Helper::sort_with_angle());

		std::vector<glm::vec3> Intersections;

		size_t roadSegmentCount = connectedRoadSegments.size();
		for (size_t i = 0; i < roadSegmentCount; i++)
		{
			size_t iNext = (i + 1) % roadSegmentCount;
			RoadSegment* rs1 = connectedRoadSegments[i];
			RoadSegment* rs2 = connectedRoadSegments[iNext];

			glm::vec3 R1_S = this == rs1->ConnectedObjectAtStart.junction ? rs1->GetStartPosition() : rs1->GetEndPosition();
			glm::vec3 R2_S = this == rs2->ConnectedObjectAtStart.junction ? rs2->GetStartPosition() : rs2->GetEndPosition();

			glm::vec3 r1Direction = this == rs1->ConnectedObjectAtStart.junction ? rs1->GetStartDirection() : rs1->GetEndDirection();
			glm::vec3 r2Direction = this == rs2->ConnectedObjectAtStart.junction ? rs2->GetStartDirection() : rs2->GetEndDirection();

			float r1width = rs1->Type[0]->boundingBoxM.z - rs1->Type[0]->boundingBoxL.z;
			float r2width = rs2->Type[0]->boundingBoxM.z - rs2->Type[0]->boundingBoxL.z;


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

		size_t indexCount = 0;
		for (size_t i = 0; i < roadSegmentCount; i++)
			indexCount += connectedRoadSegments[i]->Type[1]->indexCount;
		TexturedObjectVertex* TOVertices = new TexturedObjectVertex[indexCount];

		std::array<Ref<Texture2D>, MAX_TEXTURE_SLOTS> textures;
		uint8_t textureSlotIndex = 0;

		size_t offset = 0;
		for (size_t i = 0; i < roadSegmentCount; i++)
		{
			RoadSegment* rs = connectedRoadSegments[i];
			Prefab* prefab = rs->Type[1];
			float* prefabVerticies = prefab->vertices;
			size_t prefabIndexCount = prefab->indexCount;
			float roadWidth = prefab->boundingBoxM.z - prefab->boundingBoxL.z;
			float roadLength = prefab->boundingBoxM.x - prefab->boundingBoxL.x;

			float textureIndex = -1.0f;
			{
				for (uint8_t i = 0; i < textureSlotIndex; i++)
				{
					if (*(textures[i].get()) == *(prefab->textures[0].get()))
					{
						textureIndex = (float)i;
						break;
					}
				}
				if (textureIndex == -1.0f)
				{
					textureIndex = (float)textureSlotIndex;
					textures[textureSlotIndex] = prefab->textures[0];
					textureSlotIndex++;
				}
			}

			glm::vec3 intersection1 = Intersections[i];
			glm::vec3 intersection2 = Intersections[((roadSegmentCount + i) - 1) % roadSegmentCount];

			glm::vec3 R1 = this == rs->ConnectedObjectAtStart.junction ? rs->GetStartPosition() : rs->GetEndPosition();

			glm::vec3 JR1 = this == rs->ConnectedObjectAtStart.junction ? rs->GetStartDirection() : rs->GetEndDirection();
			JR1.y = 0.0f;
			JR1 = glm::normalize(JR1);
			glm::vec3 shiftAmount = glm::rotate(JR1, glm::radians(90.0f), glm::vec3{ 0.0f, 1.0f, 0.0f }) * roadWidth / 2.0f;

			glm::vec3 Jp = position + shiftAmount;
			glm::vec3 Jn = position - shiftAmount;
			glm::vec3 R1p = R1 + shiftAmount;
			glm::vec3 R1n = R1 - shiftAmount;

			float lengthJP = glm::length(R1p - intersection2) - glm::length(R1p - Jp);
			float lengthJN = glm::length(R1n - intersection1) - glm::length(R1n - Jn);

			float l = 0.0f;
			if (lengthJN < lengthJP)
			{
				float offsetLength = lengthJP + roadLength;
				glm::vec3 temp = position + JR1 * offsetLength;
				l = lengthJP;
				if (this == rs->ConnectedObjectAtStart.junction)
					rs->SetCurvePoint(0, temp);
				else
					rs->SetCurvePoint(3, temp);
			}
			else
			{
				float offsetLength = lengthJN + roadLength;
				glm::vec3 temp = position + JR1 * offsetLength;
				l = lengthJN;
				if (this == rs->ConnectedObjectAtStart.junction)
					rs->SetCurvePoint(0, temp);
				else
					rs->SetCurvePoint(3, temp);
			}


			float angle = rs->ConnectedObjectAtStart.junction == this ? rs->GetStartRotation().y : rs->GetEndRotation().y;

			size_t OneVertexSize = (int)(sizeof(TexturedObjectVertex) / sizeof(float));
			for (size_t j = 0; j < prefabIndexCount; j++)
			{
				size_t index = j * OneVertexSize;
				glm::vec2 point = {
					prefabVerticies[index + 0],
					prefabVerticies[index + 2]
				};
				if (point.x < 0.001f)
				{
					float percent = std::abs(point.y / (roadWidth / 2.0f));
					if (point.y < 0.001f)
						point.x += percent * lengthJP;
					else if (point.y > 0.001f)
						point.x += percent * lengthJN;
				}
				else
				{
					point.x += l;
				}

				glm::vec2 rotatedPoint = Helper::RotateAPointAroundAPoint(point, -angle);

				TOVertices[offset + j].Position.x = rotatedPoint.x;
				TOVertices[offset + j].Position.y = prefabVerticies[index + 1];
				TOVertices[offset + j].Position.z = rotatedPoint.y;
				TOVertices[offset + j].UV.x = prefabVerticies[index + 3];
				TOVertices[offset + j].UV.y = prefabVerticies[index + 4];
				TOVertices[offset + j].Normal.x = prefabVerticies[index + 5];
				TOVertices[offset + j].Normal.x = prefabVerticies[index + 6];
				TOVertices[offset + j].Normal.x = prefabVerticies[index + 7];
				TOVertices[offset + j].TextureIndex = textureIndex;
			}
			offset += prefabIndexCount;
		}
		Prefab* newPrefab = new Prefab(
			"", // No need
			connectedRoadSegments[0]->Type[1]->shaderPath, // If single shader for roads it okay!
			textures,
			textureSlotIndex,
			(float*)TOVertices,
			indexCount,
			indexCount * (int)(sizeof(TexturedObjectVertex) / sizeof(float))
		);
		object = new Object(newPrefab, newPrefab, position);
	}
	void Junction::ReconstructObject()
	{
		delete object;
		ConstructObject();
	}
}