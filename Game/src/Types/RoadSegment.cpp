#include "canpch.h"
#include "RoadSegment.h"

#include "RoadNode.h"

#include "Can/Renderer/Object.h"
#include "Can/Math.h"
#include "Helper.h"

namespace Can
{
	RoadSegment::RoadSegment(const RoadType& type, const std::array<glm::vec3, 4>& curvePoints)
		: CurvePoints(curvePoints)
	{
		CalcRotsAndDirs();
		SetType(type);
	}
	RoadSegment::RoadSegment(RoadSegment&& other)
		: type(other.type)
		, Buildings(other.Buildings)
		, Cars(other.Cars)
		, StartNode(other.StartNode)
		, EndNode(other.EndNode)
		, curve_samples(other.curve_samples)
		, curve_t_samples(other.curve_t_samples)
		, object(other.object)
		, CurvePoints(other.CurvePoints)
		, bounding_box(other.bounding_box)
		, Directions(other.Directions)
		, Rotations(other.Rotations)
	{
		other.object = nullptr;
		other.curve_samples.clear();
		other.curve_t_samples.clear();
		other.Buildings.clear();
		other.Cars.clear();
	}
	RoadSegment::~RoadSegment()
	{
		if (object)
			std::cout << "";
		delete object;
	}

	RoadSegment& RoadSegment::operator=(RoadSegment&& other)
	{
		if (object) delete object;
		type = other.type;
		Buildings = other.Buildings;
		Cars = other.Cars;
		StartNode = other.StartNode;
		EndNode = other.EndNode;
		curve_samples = other.curve_samples;
		curve_t_samples = other.curve_t_samples;
		object = other.object;
		CurvePoints = other.CurvePoints;
		bounding_box = other.bounding_box;
		Directions = other.Directions;
		Rotations = other.Rotations;

		other.object = nullptr;
		other.curve_samples.clear();
		other.curve_t_samples.clear();
		other.Buildings.clear();
		other.Cars.clear();

		return *this;
	}

	void RoadSegment::Construct()
	{
		bounding_box = Math::GetBoundingBoxOfBezierCurve(CurvePoints, type.road_width * 0.5f);
		curve_t_samples.clear();
		curve_t_samples = Math::GetCubicCurveSampleTs(CurvePoints, type.road_length);
		curve_samples.clear();
		size_t count = curve_t_samples.size();

		size_t prefabIndexCount = type.road->indexCount;
		size_t indexCount = prefabIndexCount * (count - 1);
		size_t vertexCount = indexCount * (3 + 2 + 3);
		TexturedObjectVertex* TOVertices = new TexturedObjectVertex[indexCount];

		glm::vec3 p1 = CurvePoints[0];
		curve_samples.push_back(p1);
		for (int c = 1; c < count; c++)
		{
			glm::vec3 p2 = Math::CubicCurve<float>(CurvePoints, curve_t_samples[c]);
			glm::vec3 vec1 = p2 - p1;
			float length = glm::length(vec1);
			float scale = length / type.road_length;
			glm::vec3 dir1 = vec1 / length;
			glm::vec3 dir2 = (c < count - 1) ? glm::normalize((glm::vec3)Math::CubicCurve<float>(CurvePoints, curve_t_samples[c + 1]) - p2) : -Directions[1];

			float rot1 = (dir1.z < 0.0f) ? glm::acos(-dir1.x) + glm::radians(180.f) : glm::acos(dir1.x);
			float rot2 = (dir2.z < 0.0f) ? glm::acos(-dir2.x) + glm::radians(180.f) : glm::acos(dir2.x);

			float diffRot = glm::acos(glm::dot(dir1, dir2));
			float diff = rot2 - rot1;
			diff += (diff < -3.0f ? glm::radians(360.0f) : diff > 3.0f ? glm::radians(-360.0f) : 0.0f);
			for (int i = 0; i < prefabIndexCount; i++)
			{
				size_t offset = (c - 1) * prefabIndexCount + i;
				size_t index = (size_t)i * 9;

				glm::vec2 point{
					type.road->vertices[index + 0] * scale,
					type.road->vertices[index + 2]
				};

				glm::vec2 point2{
					type.road->vertices[index + 0] * scale,
					0.0f
				};

				float t = point.x / (type.road_length * scale);
				t = t < 0.01f ? 0.0f : (t > 0.99f ? 1.0f : t);

				point = Helper::RotateAPointAroundAPoint(point, rot1);
				point2 = Helper::RotateAPointAroundAPoint(point2, rot1);
				point = Helper::RotateAPointAroundAPoint(point, Math::Lerp(0.0f, diff, t), point2);

				TOVertices[offset].Position.x = point.x + p1.x - CurvePoints[0].x;
				TOVertices[offset].Position.y = type.road->vertices[index + 1] + p1.y - CurvePoints[0].y;
				TOVertices[offset].Position.z = point.y + p1.z - CurvePoints[0].z;
				TOVertices[offset].UV = *(glm::vec2*) & type.road->vertices[index + 3];
				TOVertices[offset].Normal = *(glm::vec3*) & type.road->vertices[index + 5];
				TOVertices[offset].TextureIndex = type.road->vertices[index + 8];
			}
			p1 = p2;
			curve_samples.push_back(p1);
		}
		Prefab* newPrefab = new Prefab(type.road->objectPath, type.road->shaderPath, type.road->texturePath, (float*)TOVertices, indexCount);

		for (int c = 0; c < count-1; c++)
		{
			glm::vec3 p2 = Math::CubicCurve<float>(CurvePoints, (c + 1.0f) / count);
			glm::vec3 vec = p2 - p1;
			float length = glm::length(vec);
			glm::vec3 dir = vec / length;

			float scale = length / type.road_length;

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

		object = new Object(newPrefab, CurvePoints[0]);
		object->owns_prefab = true;
	}

	void RoadSegment::CalcRotsAndDirs()
	{
		Directions = {
			(glm::vec3)glm::normalize(CurvePoints[1] - CurvePoints[0]),
				(glm::vec3)glm::normalize(CurvePoints[2] - CurvePoints[3])
		};
		glm::vec2 dir = glm::normalize(glm::vec2{ Directions[0].x, Directions[0].z });
		Rotations[0].y = glm::acos(dir.x) * ((float)(dir.y < 0.0f) * 2.0f - 1.0f);
		glm::vec3 dirR = glm::rotate(Directions[0], -Rotations[0].y, glm::vec3{ 0.0f, 1.0f, 0.0f });
		dir = glm::normalize(glm::vec2{ dirR.x, dirR.y });
		Rotations[0].x = glm::acos(dir.x) * ((float)(dir.y < 0.0f) * 2.0f - 1.0f);

		dir = glm::normalize(glm::vec2{ Directions[1].x, Directions[1].z });
		Rotations[1].y = glm::acos(dir.x) * ((float)(dir.y < 0.0f) * 2.0f - 1.0f);
		dirR = glm::rotate(Directions[1], -Rotations[1].y, glm::vec3{ 0.0f, 1.0f, 0.0f });
		dir = glm::normalize(glm::vec2{ dirR.x, dirR.y });
		Rotations[1].x = glm::acos(dir.x) * ((float)(dir.y < 0.0f) * 2.0f - 1.0f);
	}

	void RoadSegment::ReConstruct()
	{
		if (object)
			delete object;

		Construct();

		/* Update Later
		for (size_t i = 0; i < connectedBuildings.size(); i++)
		{
			Building* building = connectedBuildings[i];
			glm::vec3 B = building->position - startPosition;
			float bLength = glm::length(B);

			float angle = glm::acos(glm::dot(direction, B) / bLength);

			float c = bLength * glm::cos(angle);
			if (c <= 0.0f || c >= length)
			{
				auto man = &(GameScene::ActiveGameScene->m_BuildingManager);
				auto it = std::find(
					man->GetBuildings().begin(),
					man->GetBuildings().end(),
					building
				);
				connectedBuildings.erase(connectedBuildings.begin() + i);
				building->connectedRoad = nullptr;
				if (man->snapOptions[0])
					man->GetBuildings().erase(it);
				delete building;
				i--;
			}
		}
		*/
	}

	void RoadSegment::SetStartPosition(const glm::vec3& position)
	{
		CurvePoints[0] = position;
		CalcRotsAndDirs();
		ReConstruct();
	}
	void RoadSegment::SetEndPosition(const glm::vec3& position)
	{
		CurvePoints[3] = position;
		CalcRotsAndDirs();
		ReConstruct();
	}
	void RoadSegment::SetCurvePoints(const std::array<glm::vec3, 4>& curvePoints)
	{
		CurvePoints = curvePoints;
		CalcRotsAndDirs();
		ReConstruct();
	}
	void RoadSegment::SetCurvePoint(size_t index, const glm::vec3& curvePoint)
	{
		CurvePoints[index] = curvePoint;
		CalcRotsAndDirs();
		ReConstruct();
	}
}