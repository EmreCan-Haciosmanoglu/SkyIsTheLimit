#include "canpch.h"
#include "RoadSegment.h"

#include "RoadNode.h"

#include "Can/Renderer/Object.h"
#include "Can/Math.h"
#include "Helper.h"

#include "Scenes/GameScene.h"
#include "GameApp.h"

#include "Types/Road_Type.h"

namespace Can
{
	RoadSegment::RoadSegment(RoadSegment&& other)
		: type(other.type)
		, buildings(other.buildings)
		, people(other.people)
		, StartNode(other.StartNode)
		, EndNode(other.EndNode)
		, curve_samples(other.curve_samples)
		, curve_t_samples(other.curve_t_samples)
		, object(other.object)
		, CurvePoints(other.CurvePoints)
		, bounding_rect(other.bounding_rect)
		, bounding_polygon(other.bounding_polygon)
		, elevation_type(other.elevation_type)
		, Directions(other.Directions)
		, Rotations(other.Rotations)
	{
		assert(false);
		other.object = nullptr;
		other.curve_samples.clear();
		other.curve_t_samples.clear();
		other.bounding_polygon.clear();
		other.buildings.clear();
		other.people.clear();
	}
	RoadSegment::~RoadSegment()
	{
		if (object)
			std::cout << "";
		delete object;
	}

	RoadSegment& RoadSegment::operator=(RoadSegment&& other)
	{
		assert(false);
		if (object) delete object;
		type = other.type;
		buildings = other.buildings;
		people = other.people;
		StartNode = other.StartNode;
		EndNode = other.EndNode;
		curve_samples = other.curve_samples;
		curve_t_samples = other.curve_t_samples;
		object = other.object;
		CurvePoints = other.CurvePoints;
		bounding_rect = other.bounding_rect;
		bounding_polygon = other.bounding_polygon;
		Directions = other.Directions;
		Rotations = other.Rotations;

		other.object = nullptr;
		other.curve_samples.clear();
		other.curve_t_samples.clear();
		other.bounding_polygon.clear();
		other.buildings.clear();
		other.people.clear();

		return *this;
	}

	void RoadSegment::Construct()
	{
		GameApp* app = GameScene::ActiveGameScene->MainApplication;
		Road_Type& rtype = app->road_types[type];
		f32 w = elevation_type == -1 ? rtype.tunnel_width : (elevation_type == 0 ? rtype.road_width : 1.0f);
		f32 l = elevation_type == -1 ? rtype.tunnel_length : (elevation_type == 0 ? rtype.road_length : 1.0f);

		bounding_rect = Math::GetBoundingBoxOfBezierCurve(CurvePoints, w * 0.5f);
		curve_t_samples.clear();
		curve_t_samples = Math::GetCubicCurveSampleTs(CurvePoints, l);
		curve_samples.clear();
		bounding_polygon.clear();
		u64 count = curve_t_samples.size();

		u64 prefabIndexCount = elevation_type == -1 ? rtype.tunnel->indexCount : elevation_type == 0 ? rtype.road->indexCount : 0;
		u64 indexCount = prefabIndexCount * (count - 1);
		u64 vertexCount = indexCount * (3 + 2 + 3);
		TexturedObjectVertex* TOVertices = new TexturedObjectVertex[indexCount];

		v3 p1 = CurvePoints[0];
		curve_samples.push_back(p1);

		v3 dirTemp = glm::normalize(CurvePoints[1] - p1);

		v2 dirrTemp = glm::normalize((v2)dirTemp);
		f32 yawTemp = glm::acos(dirrTemp.x) * ((float)(dirrTemp.y > 0.0f) * 2.0f - 1.0f);
		v3 dirRTemp = glm::rotateZ(dirTemp, -yawTemp);
		dirrTemp = glm::normalize(v2{ dirRTemp.x, dirRTemp.z });
		f32 pitchTemp = glm::acos(dirrTemp.x) * ((float)(dirrTemp.y > 0.0f) * 2.0f - 1.0f);

		TexturedObjectVertex* PrefabTOVertices =
			elevation_type == -1 ? (TexturedObjectVertex*)(rtype.tunnel->vertices) :
			elevation_type == 0 ? (TexturedObjectVertex*)(rtype.road->vertices) : nullptr;

		for (int c = 1; c < count; c++)
		{
			v3 p2 = Math::CubicCurve<f32>(CurvePoints, curve_t_samples[c]);
			v3 vec1 = p2 - p1;
			f32 length = glm::length(vec1);
			f32 scale = length / l;
			v3 dir1 = vec1 / length;
			v3 dir2 = (c < count - 1) ? glm::normalize((v3)Math::CubicCurve<f32>(CurvePoints, curve_t_samples[c + 1]) - p2) : -Directions[1];
			v3 shift1 = glm::normalize(v3{ -dir1.y, dir1.x, 0.0f }) * w * 0.5f;
			v3 shift2 = glm::normalize(v3{ -dir2.y, dir2.x, 0.0f }) * w * 0.5f;


			v2 dirr1 = glm::normalize((v2)dir1);
			v2 dirr2 = glm::normalize((v2)dir2);
			f32 yaw1 = glm::acos(dirr1.x) * ((f32)(dirr1.y > 0.0f) * 2.0f - 1.0f);
			f32 yaw2 = glm::acos(dirr2.x) * ((f32)(dirr2.y > 0.0f) * 2.0f - 1.0f);
			v3 dirR1 = glm::rotateZ(dir1, -yaw1);
			v3 dirR2 = glm::rotateZ(dir2, -yaw2);
			dirr1 = glm::normalize(v2{ dirR1.x, dirR1.z });
			dirr2 = glm::normalize(v2{ dirR2.x, dirR2.z });
			f32 pitch1 = glm::acos(glm::abs(dirr1.x)) * ((f32)(dirr1.y < 0.0f) * 2.0f - 1.0f);
			f32 pitch2 = glm::acos(glm::abs(dirr2.x)) * ((f32)(dirr2.y < 0.0f) * 2.0f - 1.0f);


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

				point = glm::rotateY(point, Math::Lerp(0.0f, pitchDiff, t));
				point = glm::rotateZ(point, Math::Lerp(0.0f, yawDiff, t));
				point.x += xoffset;

				if (c == 1 && PrefabTOVertices[i].Position.x < 0.01f)
				{
					point = glm::rotateZ(point, yawTemp);
				}
				else
				{
					if (c < count - 1 || PrefabTOVertices[i].Position.x < l + 0.01f)
						point = glm::rotateY(point, pitch1);
					point = glm::rotateZ(point, yaw1);
				}

				point += p1;
				point -= CurvePoints[0];
				TOVertices[offset].Position = point;
				TOVertices[offset].UV = PrefabTOVertices[i].UV;
				TOVertices[offset].Normal = PrefabTOVertices[i].Normal;
				TOVertices[offset].TextureIndex = PrefabTOVertices[i].TextureIndex;
			}
			bounding_polygon.push_back(std::array<v3, 3>{
				p1 + shift1,
					p1 - shift1,
					p2 + shift2
			});
			bounding_polygon.push_back(std::array<v3, 3>{
				p1 - shift1,
					p2 - shift2,
					p2 + shift2
			});

			p1 = p2;
			curve_samples.push_back(p1);
		}

		Prefab* newPrefab = nullptr;
		if (elevation_type == -1)
			newPrefab = new Prefab(rtype.tunnel->objectPath, rtype.tunnel->shaderPath, rtype.tunnel->texturePath, (f32*)TOVertices, indexCount);
		else if (elevation_type == 0)
			newPrefab = new Prefab(rtype.road->objectPath, rtype.road->shaderPath, rtype.road->texturePath, (f32*)TOVertices, indexCount);

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

		if (elevation_type == 0)
			Helper::UpdateTheTerrain(this, false);
	}

	void RoadSegment::CalcRotsAndDirs()
	{
		Directions = {
			(v3)glm::normalize(CurvePoints[1] - CurvePoints[0]),
				(v3)glm::normalize(CurvePoints[2] - CurvePoints[3])
		};
		v2 dir = glm::normalize((v2)Directions[0]);
		Rotations[0].y = glm::acos(dir.x) * ((f32)(dir.y > 0.0f) * 2.0f - 1.0f);
		v3 dirR = glm::rotateZ(Directions[0], -Rotations[0].y);
		dir = glm::normalize(v2{ dirR.x, dirR.z });
		Rotations[0].x = glm::acos(glm::abs(dir.x)) * ((f32)(dir.y < 0.0f) * 2.0f - 1.0f);

		dir = glm::normalize((v2)Directions[1]);
		Rotations[1].y = glm::acos(dir.x) * ((f32)(dir.y > 0.0f) * 2.0f - 1.0f);
		dirR = glm::rotateZ(Directions[1], -Rotations[1].y);
		dir = glm::normalize(v2{ dirR.x, dirR.z });
		Rotations[1].x = glm::acos(glm::abs(dir.x)) * ((f32)(dir.y < 0.0f) * 2.0f - 1.0f);
	}

	void RoadSegment::construct(RoadSegment* dest, u64 type, const std::array<v3, 4>& curvePoints, s8 elevation_type)
	{
		dest->CurvePoints = curvePoints;
		dest->elevation_type = elevation_type;
		dest->CalcRotsAndDirs();
		dest->SetType(type);
	}

	void RoadSegment::move(RoadSegment* dest, RoadSegment* src)
	{
		dest->type = src->type;
		dest->buildings = src->buildings;
		dest->people = src->people;
		dest->StartNode = src->StartNode;
		dest->EndNode = src->EndNode;
		dest->curve_samples = src->curve_samples;
		dest->curve_t_samples = src->curve_t_samples;
		dest->object = src->object;
		dest->CurvePoints = src->CurvePoints;
		dest->bounding_rect = src->bounding_rect;
		dest->bounding_polygon = src->bounding_polygon;
		dest->elevation_type = src->elevation_type;
		dest->Directions = src->Directions;
		dest->Rotations = src->Rotations;

		RoadSegment::reset_to_default(src);
	}

	void RoadSegment::reset_to_default(RoadSegment* dest)
	{
		dest->type = 0;
		dest->buildings = {};
		dest->people = {};
		dest->StartNode = (u64)-1;
		dest->EndNode = (u64)-1;
		dest->curve_samples.clear();
		dest->curve_t_samples.clear();
		dest->object = nullptr;
		dest->CurvePoints={};
		dest->bounding_rect = {};
		dest->bounding_polygon.clear();
		dest->elevation_type = 0;
		dest->Directions = {};
		dest->Rotations = {};
	}

	void RoadSegment::remove(RoadSegment* obj)
	{
		delete obj->object;
		RoadSegment::reset_to_default(obj);
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
	void RoadSegment::SetCurvePoints(const std::array<v3, 4>& curvePoints)
	{
		if (elevation_type == 0)
			Helper::UpdateTheTerrain(this, true);
		CurvePoints = curvePoints;
		CalcRotsAndDirs();
		ReConstruct();
	}
	void RoadSegment::SetCurvePoint(u64 index, const v3& curvePoint)
	{
		std::array<v3, 4> cps = CurvePoints;
		cps[index] = curvePoint;
		SetCurvePoints(cps);
	}
	bool remove_person_from(RoadSegment& segment, Person* person)
	{
		auto it = std::find(segment.people.begin(), segment.people.end(), person);
		if (it != segment.people.end())
		{
			segment.people.erase(it);
			return true;
		}
		return false;
	}
	bool remove_car_from(RoadSegment& segment, Car* car)
	{
		auto it = std::find(segment.vehicles.begin(), segment.vehicles.end(), car);
		if (it != segment.vehicles.end())
		{
			segment.vehicles.erase(it);
			return true;
		}
		return false;
	}
}