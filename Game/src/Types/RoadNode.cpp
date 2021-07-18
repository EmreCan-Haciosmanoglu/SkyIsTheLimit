#include "canpch.h"
#include "RoadNode.h"

#include "Types/RoadSegment.h"

#include "Helper.h"
#include "Can/Math.h"

#include "GameApp.h"

namespace Can
{
	RoadNode::RoadNode(const std::vector<u64>& roadSegments, const v3& position, u8 elevation_type)
		: roadSegments(roadSegments)
		, position(position)
		, elevation_type(elevation_type)
	{
		Reconstruct();
	}
	RoadNode::RoadNode(RoadNode&& other)
		: object(other.object)
		, roadSegments(other.roadSegments)
		, position(other.position)
		, index(other.index)
		, elevation_type(other.elevation_type)
	{
		other.roadSegments.clear();
		other.object = nullptr;
	}
	RoadNode::~RoadNode()
	{
		if (object)
			std::cout << "";
		delete object;
	}

	RoadNode& RoadNode::operator=(RoadNode&& other)
	{
		if (object) delete object;
		object = other.object;
		roadSegments = other.roadSegments;
		position = other.position;
		index = other.index;
		elevation_type = other.elevation_type;

		other.roadSegments.clear();
		other.object = nullptr;
		return *this;
	}

	void RoadNode::AddRoadSegment(std::vector<u64> arr)
	{
		for (u64 i = 0; i < arr.size(); i++)
			roadSegments.push_back(arr[i]);
		Reconstruct();
	}
	void RoadNode::RemoveRoadSegment(u64 roadSegment)
	{
		std::vector<u64>::iterator it = std::find(roadSegments.begin(), roadSegments.end(), roadSegment);
		if (it != roadSegments.end())
		{
			roadSegments.erase(it);
			if (roadSegments.size() > 0)
				Reconstruct();
		}
	}

	void RoadNode::Reconstruct()
	{
		if (roadSegments.size() == 0)
			return;

		if (object) delete object;
		auto& segments = GameScene::ActiveGameScene->m_RoadManager.m_Segments;
		auto& nodes = GameScene::ActiveGameScene->m_RoadManager.m_Nodes;

		RoadSegment& rs = segments[roadSegments[0]];
		if (roadSegments.size() == 1)
		{
			v3 rotation = v3(0.0f);
			bool isStartNode = index == rs.StartNode;
			if (isStartNode)
			{
				rs.SetStartPosition(position);
				rotation = { 0.0f,
					rs.GetStartRotation().y + glm::radians(180.0f),
					-rs.GetStartRotation().x
				};
			}
			else
			{
				rs.SetEndPosition(position);
				rotation = { 0.0f,
					rs.GetEndRotation().y + glm::radians(180.0f),
					-rs.GetEndRotation().x
				};
			}

			object = new Object(
				rs.elevation_type == -1 ? (rs.type.asymmetric && isStartNode ? rs.type.tunnel_end_mirror : rs.type.tunnel_end) :
				rs.elevation_type == 0 ? (rs.type.asymmetric && isStartNode ? rs.type.road_end_mirror : rs.type.road_end) : nullptr,
				position,
				rotation
			);

			object->owns_prefab = false;
			return;
		}

		std::sort(roadSegments.begin(), roadSegments.end(), Helper::sort_by_angle());

		u64 count = roadSegments.size();
		std::vector<v3> Intersections(count, { 0.0f, 0.0f, 0.0f });

		for (u64 i = 0; i < count; i++)
		{
			u64 next_i = (i + 1) % count;

			RoadSegment& rs1 = segments[roadSegments[i]];
			RoadSegment& rs2 = segments[roadSegments[next_i]];

			v3 r1Dir = index == rs1.StartNode ? rs1.GetStartDirection() : rs1.GetEndDirection();
			v3 r2Dir = index == rs2.StartNode ? rs2.GetStartDirection() : rs2.GetEndDirection();
			r1Dir.y = 0.0f;
			r2Dir.y = 0.0f;

			v3 shiftR1Dir = glm::normalize(v3{ +r1Dir.z, 0.0f, -r1Dir.x });
			v3 shiftR2Dir = glm::normalize(v3{ -r2Dir.z, 0.0f, +r2Dir.x });

			v3 shiftR1Amount = shiftR1Dir * (rs1.type.road_width * 0.5f);
			v3 shiftR2Amount = shiftR2Dir * (rs2.type.road_width * 0.5f);

			f32 dot_product = glm::dot(shiftR1Dir, shiftR2Dir);
			f32 len_product = glm::length(shiftR1Dir) * glm::length(shiftR2Dir);
			f32 cos_value = dot_product / len_product;
			f32 minned_maxxed = std::max(-1.0f, std::min(cos_value, 1.0f));
			f32 angleDiff = glm::acos(minned_maxxed);
			if (angleDiff < glm::radians(2.5f) || angleDiff > glm::radians(177.5f))
			{
				Intersections[i] = position + shiftR1Amount;
			}
			else
			{
				Intersections[i] = Helper::RayPlaneIntersection(
					position + shiftR1Amount,
					r1Dir,
					position + shiftR2Amount,
					shiftR2Dir);
			}
			if (isnan(Intersections[i].x) || isinf(Intersections[i].x))
				std::cout << "";
		}

		bool isTunnel = elevation_type == -1;
		u64 indexCount = 0;
		for (u64 i = 0; i < count; i++)
		{
			const RoadSegment& rs = segments[roadSegments[i]];
			bool asym = rs.type.asymmetric;
			bool isStartNode = index == rs.StartNode;
			bool isFromGround = rs.elevation_type == 0;
			indexCount += (isTunnel ?
				(asym && !isStartNode ? rs.type.tunnel_junction_mirror : rs.type.tunnel_junction) :
				(asym && !isStartNode ? rs.type.road_junction_mirror : rs.type.road_junction))->indexCount;
			indexCount += isTunnel && isFromGround ? rs.type.tunnel_entrance->indexCount : 0U;
		}

		TexturedObjectVertex* TOVertices = new TexturedObjectVertex[indexCount];
		std::array<Ref<Texture2D>, MAX_TEXTURE_SLOTS> textures;
		uint8_t textureSlotIndex = 0;
		u64 oneVertexSize = sizeof(TexturedObjectVertex) / sizeof(f32);

		u64 offset = 0;
		for (u64 i = 0; i < count; i++)
		{
			RoadSegment& rs = segments[roadSegments[i]];
			bool asym = rs.type.asymmetric;
			bool isStartNode = index == rs.StartNode;
			bool isFromGround = rs.elevation_type == 0;
			Prefab* prefab = nullptr;
			f32 halfWidth = 0.0f;
			f32 junction_length = 0.0f;
			if (isTunnel)
			{
				halfWidth = rs.type.tunnel_width * 0.5f;
				junction_length = rs.type.tunnel_junction_length;
				prefab = asym && !isStartNode ? rs.type.tunnel_junction_mirror : rs.type.tunnel_junction;
			}
			else
			{
				halfWidth = rs.type.road_width * 0.5f;
				junction_length = rs.type.road_junction_length;
				prefab = asym && !isStartNode ? rs.type.road_junction_mirror : rs.type.road_junction;
			}
			f32* prefabVertices = prefab->vertices;
			u64 prefabIndexCount = prefab->indexCount;


			f32 textureIndex = -1.0f;
			for (uint8_t i = 0; i < textureSlotIndex; i++)
			{
				if (*(textures[i].get()) == *(prefab->textures[0].get()))
				{
					textureIndex = i;
					break;
				}
			}
			if (textureIndex == -1.0f)
			{
				textureIndex = (f32)textureSlotIndex;
				textures[textureSlotIndex] = prefab->textures[0];
				textureSlotIndex++;
			}

			v3 intersection1 = Intersections[i];
			v3 intersection2 = Intersections[((i - 1) + count) % count];

			v3 RoadPos = index == rs.StartNode ? rs.GetCurvePoint(1) : rs.GetCurvePoint(2);
			v3 RoadDir = index == rs.StartNode ? rs.GetStartDirection() : rs.GetEndDirection();
			RoadDir.y = 0.0f;
			RoadDir = glm::normalize(RoadDir);

			v3 shiftAmount{ +RoadDir.z * halfWidth, 0.0f, -RoadDir.x * halfWidth };

			v3 rp = RoadPos + shiftAmount;
			v3 rn = RoadPos - shiftAmount;

			v3 jp = position + shiftAmount;
			v3 jn = position - shiftAmount;

			f32 ljp = glm::length(rp - jp) - glm::length(rp - intersection1);
			f32 ljn = glm::length(rn - jn) - glm::length(rn - intersection2);

			f32 l = ljn > ljp ? ljn : ljp;

			f32 offsetl = l + junction_length;
			v3 temp = position + RoadDir * offsetl;
			rs.SetCurvePoint(3 * (u64)(index != rs.StartNode), temp);
			f32 lcp = glm::length((index == rs.StartNode) ? rs.GetCurvePoint(1) - rs.GetCurvePoint(0) : rs.GetCurvePoint(2) - rs.GetCurvePoint(3));
			if (offsetl > lcp)
				rs.SetCurvePoint(2 - (u64)(index == rs.StartNode), temp + RoadDir * 0.1f);

			f32 angle = index == rs.StartNode ? rs.GetStartRotation().y : rs.GetEndRotation().y;
			for (u64 j = 0; j < prefabIndexCount; j++)
			{
				u64 index = j * oneVertexSize;
				v2 point{
					prefabVertices[index + 0],
					prefabVertices[index + 2]
				};
				if (point.x < 0.001f)
				{
					f32 percent = std::abs(point.y / halfWidth);
					if (point.y < 0.001f)
						point.x += percent * ljp;
					else if (point.y > 0.001f)
						point.x += percent * ljn;
				}
				else
				{
					point.x += l;
				}

				v2 rotatedPoint = Math::RotatePoint(point, -angle);

				TOVertices[offset + j].Position.x = rotatedPoint.x;
				TOVertices[offset + j].Position.y = prefabVertices[index + 1];
				TOVertices[offset + j].Position.z = rotatedPoint.y;
				TOVertices[offset + j].UV.x = prefabVertices[index + 3];
				TOVertices[offset + j].UV.y = prefabVertices[index + 4];
				TOVertices[offset + j].Normal.x = prefabVertices[index + 5];
				TOVertices[offset + j].Normal.y = prefabVertices[index + 6];
				TOVertices[offset + j].Normal.z = prefabVertices[index + 7];
				TOVertices[offset + j].TextureIndex = textureIndex;
			}
			offset += prefabIndexCount;
			if (isTunnel && isFromGround)
			{
				Prefab* tunnel_entrance = asym && !isStartNode ? rs.type.tunnel_entrance_mirror : rs.type.tunnel_entrance;
				TexturedObjectVertex* verts = (TexturedObjectVertex*)(tunnel_entrance->vertices);
				textureIndex = -1.0f;
				for (uint8_t i = 0; i < textureSlotIndex; i++)
				{
					if (*(textures[i].get()) == *(tunnel_entrance->textures[0].get()))
					{
						textureIndex = i;
						break;
					}
				}
				if (textureIndex == -1.0f)
				{
					textureIndex = (f32)textureSlotIndex;
					textures[textureSlotIndex] = tunnel_entrance->textures[0];
					textureSlotIndex++;
				}
				for (u64 j = 0; j < tunnel_entrance->indexCount; j++)
				{
					v3 point = verts[j].Position;
					point.x += junction_length;
					point = glm::rotateY(point, angle);

					TOVertices[offset + j].Position = point;
					TOVertices[offset + j].UV = verts[j].UV;
					TOVertices[offset + j].Normal = verts[j].Normal;
					TOVertices[offset + j].TextureIndex = textureIndex;
				}
				offset += tunnel_entrance->indexCount;
			}
		}
		
		Prefab* newPrefab = new Prefab(
			"",
			segments[roadSegments[0]].type.road_junction->shaderPath, // we may have different shaders in the future
			textures,
			textureSlotIndex,
			(f32*)TOVertices,
			indexCount,
			indexCount * oneVertexSize
		);
		object = new Object(newPrefab, position);
		object->owns_prefab = true;
	}
}