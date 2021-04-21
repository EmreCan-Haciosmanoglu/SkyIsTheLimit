#include "canpch.h"
#include "RoadNode.h"

#include "Types/RoadSegment.h"

#include "Helper.h"
#include "Can/Math.h"

namespace Can
{
	RoadNode::RoadNode(const std::vector<RoadSegment*>& roadSegments, const glm::vec3& position)
		: roadSegments(roadSegments)
		, position(position)
	{
		Reconstruct();
	}

	void RoadNode::AddRoadSegment(RoadSegment* roadSegment)
	{
		roadSegments.push_back(roadSegment);
		Reconstruct();
	}
	void RoadNode::RemoveRoadSegment(RoadSegment* roadSegment)
	{
		std::vector<RoadSegment*>::iterator it = std::find(roadSegments.begin(), roadSegments.end(), roadSegment);
		if (it != roadSegments.end())
		{
			roadSegments.erase(it);
			if (roadSegments.size() > 0)
				Reconstruct();
		}
	}

	void RoadNode::Reconstruct()
	{
		if (object)delete object;

		RoadSegment* rs = roadSegments[0];
		if (roadSegments.size() == 1)
		{
			glm::vec3 rotation = glm::vec3(0.0f);
			if (this == rs->StartNode)
			{
				rotation = { 0.0f,
					rs->GetStartRotation().y + glm::radians(180.0f),
					rs->GetStartRotation().x
				};
			}
			else
			{
				rotation = { 0.0f,
					rs->GetEndRotation().y + glm::radians(180.0f),
					rs->GetEndRotation().x
				};
			}
			object = new Object(rs->road_type.end, rs->road_type.end, position, glm::vec3(1.0f), rotation);
			return;
		}

		std::sort(roadSegments.begin(), roadSegments.end(), sort_by_angle());

		size_t count = roadSegments.size();
		std::vector<glm::vec3> Intersections(count, { 0.0f, 0.0f, 0.0f });

		for (size_t i = 0; i < count; i++)
		{
			size_t next_i = (i + 1) % count;

			RoadSegment* rs1 = roadSegments[i];
			RoadSegment* rs2 = roadSegments[next_i];

			glm::vec3 r1Dir = this == rs1->StartNode ? rs1->GetStartDirection() : rs1->GetEndDirection();
			glm::vec3 r2Dir = this == rs2->StartNode ? rs2->GetStartDirection() : rs2->GetEndDirection();
			r1Dir.y = 0.0f;
			r2Dir.y = 0.0f;

			glm::vec3 shiftR1Dir = glm::normalize(glm::vec3{ +r1Dir.z, 0.0f, -r1Dir.x });
			glm::vec3 shiftR2Dir = glm::normalize(glm::vec3{ -r2Dir.z, 0.0f, +r2Dir.x });

			glm::vec3 shiftR1Amount = shiftR1Dir * (rs1->road_type.width * 0.5f);
			glm::vec3 shiftR2Amount = shiftR2Dir * (rs2->road_type.width * 0.5f);

			float angleDiff = glm::acos(std::min(glm::dot(shiftR1Dir, shiftR2Dir), 1.0f));
			if (angleDiff < glm::radians(2.5f) && angleDiff > glm::radians(177.5f))
			{
				Intersections[i].x = position.x + shiftR1Amount.x;
				Intersections[i].z = position.z + shiftR1Amount.z;
			}
			else
			{
				glm::vec3 intersection = Helper::RayPlaneIntersection(
					position + shiftR1Amount,
					r1Dir,
					position + shiftR2Amount,
					r2Dir);

				Intersections[i].x = intersection.x;
				Intersections[i].z = intersection.z;
			}
		}

		size_t indexCount = 0;
		for (size_t i = 0; i < count; i++)
			indexCount += roadSegments[i]->road_type.junction->indexCount;

		TexturedObjectVertex* TOVertices = new TexturedObjectVertex[indexCount];
		std::array<Ref<Texture2D>, MAX_TEXTURE_SLOTS> textures;
		uint8_t textureSlotIndex = 0;
		size_t oneVertexSize = sizeof(TexturedObjectVertex) / sizeof(float);

		size_t offset = 0;
		for (size_t i = 0; i < count; i++)
		{
			RoadSegment* rs = roadSegments[i];
			Prefab* prefab = rs->road_type.junction;
			float* prefabVertices = prefab->vertices;
			size_t prefabIndexCount = prefab->indexCount;
			float halfWidth = rs->road_type.width * 0.5f;
			float junction_length = rs->road_type.junction_length;


			uint32_t textureIndex = 0 - 1;
			for (uint8_t i = 0; i < textureSlotIndex; i++)
			{
				if (*(textures[i].get()) == *(prefab->textures[0].get()))
				{
					textureIndex = i;
					break;
				}
			}
			if (textureIndex == (0 - 1))
			{
				textureIndex = (float)textureSlotIndex;
				textures[textureSlotIndex] = prefab->textures[0];
				textureSlotIndex++;
			}

			glm::vec3 intersection1 = Intersections[i];
			glm::vec3 intersection2 = Intersections[((i - 1) + count) % count];

			glm::vec3 RoadPos = this == rs->StartNode ? rs->GetCurvePoint(1) : rs->GetCurvePoint(2);
			glm::vec3 RoadDir = this == rs->StartNode ? rs->GetStartDirection() : rs->GetEndDirection();
			RoadDir.y = 0.0f;
			RoadDir = glm::normalize(RoadDir);

			glm::vec3 shiftAmount{ +RoadDir.z * halfWidth, 0.0f, -RoadDir.x * halfWidth };

			glm::vec3 rp = RoadPos + shiftAmount;
			glm::vec3 rn = RoadPos - shiftAmount;

			float ljp = -glm::length(rp - intersection1);
			float ljn = -glm::length(rn - intersection2);
			//float ljp = glm::length(rp - intersection1);
			//float ljn = glm::length(rn - intersection2);

			float l = ljn > ljp ? ljn : ljp;
			//float l = std::max(ljn, ljp);
			//float l = ljn < ljp ? ljn : ljp;
			//float l = std::min(ljn, ljp);

			float offsetl = l + junction_length;
			glm::vec3 temp = position + RoadDir * offsetl;
			rs->SetCurvePoint(3 * (size_t)(this != rs->StartNode), temp);
			float lcp = glm::length((this == rs->StartNode) ? rs->GetCurvePoint(1) - rs->GetCurvePoint(0) : rs->GetCurvePoint(2) - rs->GetCurvePoint(3));
			if (offsetl > lcp)
				rs->SetCurvePoint(2 - (size_t)(this == rs->StartNode), temp + RoadDir * 0.1f);

			float angle = this == rs->StartNode ? rs->GetStartRotation().y : rs->GetEndRotation().y;
			for (size_t j = 0; j < prefabIndexCount; j++)
			{
				size_t index = j * oneVertexSize;
				glm::vec2 point{
					prefabVertices[index + 0],
					prefabVertices[index + 2]
				};
				if (point.x < 0.001f)
				{
					float percent = std::abs(point.y / halfWidth);
					if (point.y < 0.001f)
						point.x += percent * ljp;
					else if (point.y > 0.001f)
						point.x += percent * ljn;
				}
				else
				{
					point.x += l;
				}

				glm::vec2 rotatedPoint = Math::RotatePoint(point, -angle);

				TOVertices[offset + j].Position.x = rotatedPoint.x;
				TOVertices[offset + j].Position.y = prefabVertices[index + 1];
				TOVertices[offset + j].Position.z = rotatedPoint.y;
				TOVertices[offset + j].UV.x = prefabVertices[index + 3];
				TOVertices[offset + j].UV.y = prefabVertices[index + 4];
				TOVertices[offset + j].Normal.x = prefabVertices[index + 5];
				TOVertices[offset + j].Normal.x = prefabVertices[index + 6];
				TOVertices[offset + j].Normal.x = prefabVertices[index + 7];
				TOVertices[offset + j].TextureIndex = textureIndex;
			}
			offset += prefabIndexCount;
		}
		Prefab* newPrefab = new Prefab(
			"",
			roadSegments[0]->road_type.junction->shaderPath,
			textures,
			textureSlotIndex,
			(float*)TOVertices,
			indexCount,
			indexCount * oneVertexSize
		);
		object = new Object(newPrefab, newPrefab, position);
	}
}