#include "canpch.h"
#include "RoadSegment.h"

#include "RoadNode.h"

#include "Can/Renderer/Object.h"
#include "Can/Math.h"
#include "Helper.h"

#include "Scenes/GameScene.h"

namespace Can
{
	RoadSegment::RoadSegment(const RoadType& type, const std::array<v3, 4>& curvePoints, s8 elevation_type)
		: CurvePoints(curvePoints)
		, elevation_type(elevation_type)
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
		, elevation_type(other.elevation_type)
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
		f32 w = elevation_type == -1 ? type.tunnel_width : (elevation_type == 0 ? type.road_width : 1.0f);
		f32 l = elevation_type == -1 ? type.tunnel_length : (elevation_type == 0 ? type.road_length : 1.0f);

		bounding_box = Math::GetBoundingBoxOfBezierCurve(CurvePoints, w * 0.5f);
		curve_t_samples.clear();
		curve_t_samples = Math::GetCubicCurveSampleTs(CurvePoints, l);
		curve_samples.clear();
		u64 count = curve_t_samples.size();

		u64 prefabIndexCount = elevation_type == -1 ? type.tunnel->indexCount : elevation_type == 0 ? type.road->indexCount : 0;
		u64 indexCount = prefabIndexCount * (count - 1);
		u64 vertexCount = indexCount * (3 + 2 + 3);
		TexturedObjectVertex* TOVertices = new TexturedObjectVertex[indexCount];

		v3 p1 = CurvePoints[0];
		curve_samples.push_back(p1);

		v3 dirTemp = glm::normalize(CurvePoints[1] - p1);

		v2 dirrTemp = glm::normalize(v2{ dirTemp.x, dirTemp.z });
		f32 yawTemp = glm::acos(dirrTemp.x) * ((float)(dirrTemp.y < 0.0f) * 2.0f - 1.0f);
		v3 dirRTemp = glm::rotateY(dirTemp, -yawTemp);
		dirrTemp = glm::normalize(v2{ dirRTemp.x, dirRTemp.y });
		f32 pitchTemp = glm::acos(dirrTemp.x) * ((float)(dirrTemp.y > 0.0f) * 2.0f - 1.0f);

		TexturedObjectVertex* PrefabTOVertices =
			elevation_type == -1 ? (TexturedObjectVertex*)(type.tunnel->vertices) :
			elevation_type == 0 ? (TexturedObjectVertex*)(type.road->vertices) : nullptr;

		for (int c = 1; c < count; c++)
		{
			v3 p2 = Math::CubicCurve<f32>(CurvePoints, curve_t_samples[c]);
			v3 vec1 = p2 - p1;
			f32 length = glm::length(vec1);
			f32 scale = length / l;
			v3 dir1 = vec1 / length;
			v3 dir2 = (c < count - 1) ? glm::normalize((v3)Math::CubicCurve<f32>(CurvePoints, curve_t_samples[c + 1]) - p2) : -Directions[1];


			v2 dirr1 = glm::normalize(v2{ dir1.x, dir1.z });
			v2 dirr2 = glm::normalize(v2{ dir2.x, dir2.z });
			f32 yaw1 = glm::acos(dirr1.x) * ((f32)(dirr1.y < 0.0f) * 2.0f - 1.0f);
			f32 yaw2 = glm::acos(dirr2.x) * ((f32)(dirr2.y < 0.0f) * 2.0f - 1.0f);
			v3 dirR1 = glm::rotateY(dir1, -yaw1);
			v3 dirR2 = glm::rotateY(dir2, -yaw2);
			dirr1 = glm::normalize(v2{ dirR1.x, dirR1.y });
			dirr2 = glm::normalize(v2{ dirR2.x, dirR2.y });
			f32 pitch1 = glm::acos(dirr1.x) * ((f32)(dirr1.y > 0.0f) * 2.0f - 1.0f);
			f32 pitch2 = glm::acos(dirr2.x) * ((f32)(dirr2.y > 0.0f) * 2.0f - 1.0f);


			f32 yawDiff = yaw2 - yaw1;
			f32 pitchDiff = pitch2 - pitch1;

			yawDiff += (yawDiff < -3.0f ? glm::radians(360.0f) : yawDiff > 3.0f ? glm::radians(-360.0f) : 0.0f);
			pitchDiff += (pitchDiff < -glm::radians(80.0f) ? glm::radians(180.0f) : pitchDiff > glm::radians(80.0f) ? glm::radians(-180.0f) : 0.0f);

			for (u64 i = 0; i < prefabIndexCount; i++)
			{
				u64 offset = (c - 1) * prefabIndexCount + i;

				f32 xoffset = PrefabTOVertices[i].Position.x * scale;

				v3 point{
					0.0f,
					PrefabTOVertices[i].Position.y,
					PrefabTOVertices[i].Position.z
				};

				f32 t = xoffset / (l * scale);
				t = t < 0.01f ? 0.0f : (t > 0.99f ? 1.0f : t);

				point = glm::rotateZ(point, Math::Lerp(0.0f, pitchDiff, t));
				point = glm::rotateY(point, Math::Lerp(0.0f, yawDiff, t));
				point.x += xoffset;

				if (c == 1 && PrefabTOVertices[i].Position.x < 0.01f)
				{
					point = glm::rotateY(point, yawTemp);
				}
				else
				{
					if (c < count - 1 || PrefabTOVertices[i].Position.x < l + 0.01f)
						point = glm::rotateZ(point, pitch1);
					point = glm::rotateY(point, yaw1);
				}

				point += p1;
				point -= CurvePoints[0];
				TOVertices[offset].Position = point;
				TOVertices[offset].UV = PrefabTOVertices[i].UV;
				TOVertices[offset].Normal = PrefabTOVertices[i].Normal;
				TOVertices[offset].TextureIndex = PrefabTOVertices[i].TextureIndex;
			}
			p1 = p2;
			curve_samples.push_back(p1);
		}

		Prefab* newPrefab = nullptr;
		if (elevation_type == -1)
			newPrefab = new Prefab(type.tunnel->objectPath, type.tunnel->shaderPath, type.tunnel->texturePath, (f32*)TOVertices, indexCount);
		else if (elevation_type == 0)
			newPrefab = new Prefab(type.road->objectPath, type.road->shaderPath, type.road->texturePath, (f32*)TOVertices, indexCount);

		for (int c = 0; c < count - 1; c++)
		{
			for (int i = 0; i < prefabIndexCount; i++)
			{
				u64 offset = c * prefabIndexCount + i;
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

		GameApp* app = GameScene::ActiveGameScene->MainApplication;
		if (elevation_type == 0)
			Helper::UpdateTheTerrain(app, this, false);
	}

	void RoadSegment::CalcRotsAndDirs()
	{
		Directions = {
			(v3)glm::normalize(CurvePoints[1] - CurvePoints[0]),
				(v3)glm::normalize(CurvePoints[2] - CurvePoints[3])
		};
		v2 dir = glm::normalize(v2{ Directions[0].x, Directions[0].z });
		Rotations[0].y = glm::acos(dir.x) * ((f32)(dir.y < 0.0f) * 2.0f - 1.0f);
		v3 dirR = glm::rotate(Directions[0], -Rotations[0].y, v3{ 0.0f, 1.0f, 0.0f });
		dir = glm::normalize(v2{ dirR.x, dirR.y });
		Rotations[0].x = glm::acos(dir.x) * ((f32)(dir.y > 0.0f) * 2.0f - 1.0f);

		dir = glm::normalize(v2{ Directions[1].x, Directions[1].z });
		Rotations[1].y = glm::acos(dir.x) * ((f32)(dir.y < 0.0f) * 2.0f - 1.0f);
		dirR = glm::rotate(Directions[1], -Rotations[1].y, v3{ 0.0f, 1.0f, 0.0f });
		dir = glm::normalize(v2{ dirR.x, dirR.y });
		Rotations[1].x = glm::acos(dir.x) * ((f32)(dir.y > 0.0f) * 2.0f - 1.0f);
	}

	void RoadSegment::ReConstruct()
	{
		if (object)
			delete object;

		Construct();

		/* Update Later
		for (u64 i = 0; i < connectedBuildings.size(); i++)
		{
			Building* building = connectedBuildings[i];
			v3 B = building->position - startPosition;
			f32 bLength = glm::length(B);

			f32 angle = glm::acos(glm::dot(direction, B) / bLength);

			f32 c = bLength * glm::cos(angle);
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

	void RoadSegment::SetStartPosition(const v3& position)
	{
		CurvePoints[0] = position;
		CalcRotsAndDirs();
		ReConstruct();
	}
	void RoadSegment::SetEndPosition(const v3& position)
	{
		CurvePoints[3] = position;
		CalcRotsAndDirs();
		ReConstruct();
	}
	void RoadSegment::SetCurvePoints(const std::array<v3, 4>& curvePoints)
	{
		CurvePoints = curvePoints;
		CalcRotsAndDirs();
		ReConstruct();
	}
	void RoadSegment::SetCurvePoint(u64 index, const v3& curvePoint)
	{
		CurvePoints[index] = curvePoint;
		CalcRotsAndDirs();
		ReConstruct();
	}
}