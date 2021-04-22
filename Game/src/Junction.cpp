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

			glm::vec3 R1Position = this == rs1->ConnectedObjectAtStart.junction ? rs1->ConnectedObjectAtStart.junction->position : rs1->ConnectedObjectAtEnd.junction->position;
			glm::vec3 R2Position = this == rs2->ConnectedObjectAtStart.junction ? rs2->ConnectedObjectAtStart.junction->position : rs2->ConnectedObjectAtEnd.junction->position;

			glm::vec3 r1Direction = this == rs1->ConnectedObjectAtStart.junction ? rs1->GetStartDirection() : rs1->GetEndDirection();
			glm::vec3 r2Direction = this == rs2->ConnectedObjectAtStart.junction ? rs2->GetStartDirection() : rs2->GetEndDirection();
			r1Direction.y = 0.0f;
			r2Direction.y = 0.0f;

			float r1width = rs1->Type[0]->boundingBoxM.z - rs1->Type[0]->boundingBoxL.z;
			float r2width = rs2->Type[0]->boundingBoxM.z - rs2->Type[0]->boundingBoxL.z;


			// glm::vec3 shiftRDir = glm::normalize(glm::rotateY(rDirection, glm::radians(-90.0f)));
			glm::vec3 shiftR1Dir = glm::normalize(glm::vec3{ +r1Direction.z, r1Direction.y, -r1Direction.x });
			glm::vec3 shiftR2Dir = glm::normalize(glm::vec3{ -r2Direction.z, r2Direction.y, +r2Direction.x });

			glm::vec3 shiftR1Amount = shiftR1Dir * (r1width  * 0.5f);
			glm::vec3 shiftR2Amount = shiftR2Dir * (r2width  * 0.5f);

			float angleDiff = glm::degrees(glm::acos(std::min(glm::dot(shiftR1Dir, shiftR2Dir),1.0f)));
			if (angleDiff < 2.5f || angleDiff > 177.5f)
			{
				Intersections.push_back({ position.x + shiftR1Amount.x, 0.0f, position.z + shiftR1Amount.z });
				continue;
			}
			else
			{
				R1Position += shiftR1Amount;
				R2Position += shiftR2Amount;

				glm::vec3 I = Helper::RayPlaneIntersection(R1Position, r1Direction, R2Position, shiftR2Dir);
				I.y = 0.0f;
				Intersections.push_back(I);
			}
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
			glm::vec3 intersection2 = Intersections[((i - 1) + roadSegmentCount) % roadSegmentCount];


			//glm::vec3 RPos = this == rs->ConnectedObjectAtStart.junction ? rs->GetCurvePoint(1) : rs->GetCurvePoint(2);
			glm::vec3 RPos = rs->GetCurvePoint(2 - (size_t)(this == rs->ConnectedObjectAtStart.junction));
			glm::vec3 RDir = this == rs->ConnectedObjectAtStart.junction ? rs->GetStartDirection() : rs->GetEndDirection();
			RDir.y = 0.0f;
			RDir = glm::normalize(RDir);

			glm::vec3 shiftAmount = glm::vec3{ RDir.z, RDir.y, -RDir.x } *(roadWidth * 0.5f);

			glm::vec3 Jp = position + shiftAmount;
			glm::vec3 R1p = RPos + shiftAmount;

			glm::vec3 Jn = position - shiftAmount;
			glm::vec3 R1n = RPos - shiftAmount;

			float lcp = glm::length((this == rs->ConnectedObjectAtStart.junction ? rs->GetCurvePoint(1) - rs->GetCurvePoint(0) : rs->GetCurvePoint(2) - rs->GetCurvePoint(3)));
			float lengthJP = glm::length(R1p - Jp) - glm::length(R1p - intersection1);
			float lengthJN = glm::length(R1n - Jn) - glm::length(R1n - intersection2);
			float l = lengthJN > lengthJP ? lengthJN : lengthJP;

			float offsetLength = l + roadLength;
			glm::vec3 temp = position + RDir * offsetLength;
			rs->SetCurvePoint(3 * (size_t)(this != rs->ConnectedObjectAtStart.junction), temp);
			if (offsetLength >= lcp)
				rs->SetCurvePoint(2 - (size_t)(this == rs->ConnectedObjectAtStart.junction), temp + RDir * 0.1f);

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
			connectedRoadSegments[0]->Type[1]->shaderPath,
			textures,
			textureSlotIndex,
			(float*)TOVertices,
			indexCount,
			indexCount * (int)(sizeof(TexturedObjectVertex) / sizeof(float))
		);
		object = new Object(newPrefab, position);
		object->owns_prefab = true;
	}
	void Junction::ReconstructObject()
	{
		delete object;
		ConstructObject();
	}
}