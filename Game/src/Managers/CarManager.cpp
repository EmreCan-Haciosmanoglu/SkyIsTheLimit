#include "canpch.h"
#include "CarManager.h"
#include "Scenes/GameScene.h"
#include "Types/RoadSegment.h"
#include "GameApp.h"
#include "Helper.h"

#include "Can/Math.h"

namespace Can
{
	CarManager::CarManager(GameScene* scene)
		: m_Scene(scene)
	{
		m_Guideline = new Object(m_Scene->MainApplication->cars[m_Type]);
		m_Guideline->enabled = false;
	}
	CarManager::~CarManager()
	{
	}

	void CarManager::OnUpdate(v3& prevLocation, const v3& cameraPosition, const v3& cameraDirection)
	{
		switch (m_ConstructionMode)
		{
		case CarConstructionMode::None:
			break;
		case CarConstructionMode::Adding:
			OnUpdate_Adding(prevLocation, cameraPosition, cameraDirection);
			break;
		case CarConstructionMode::Removing:
			OnUpdate_Removing(prevLocation, cameraPosition, cameraDirection);
			break;
		default:
			break;
		}
	}
	void CarManager::OnUpdate_Adding(v3& prevLocation, const v3& cameraPosition, const v3& cameraDirection)
	{
		m_SnappedRoadSegment = -1;
		m_Guideline->SetTransform(prevLocation);
		Prefab* selectedCar = m_Guideline->prefab;
		GameApp* app = m_Scene->MainApplication;

		u64 count = m_Scene->m_RoadManager.m_Segments.size();
		for (u64 rsIndex = 0; rsIndex < count; rsIndex++)
		{
			RoadSegment& rs = m_Scene->m_RoadManager.m_Segments[rsIndex];
			RoadType& type = app->road_types[rs.type];
			f32 roadWidth = type.road_width;
			f32 roadLength = type.road_length;
			f32 snapDistance = roadWidth * 0.5f;

			const std::array<v3, 4>& vs = rs.GetCurvePoints();
			std::array<std::array<v2, 3>, 2> roadPolygon = Math::GetBoundingBoxOfBezierCurve(vs, snapDistance);

			if (Math::CheckPolygonPointCollision(roadPolygon, (v2)prevLocation))
			{
				std::vector<f32> ts{ 0.0f };
				std::vector<v3> ps = Math::GetCubicCurveSamples(vs, roadLength, ts);
				u64 size = ps.size();
				v3 p0 = ps[0];
				for (u64 i = 1; i < size; i++)
				{
					v3 p1 = ps[i];
					v3 dirToP1 = p1 - p0;
					dirToP1.z = 0.0f;
					dirToP1 = glm::normalize(dirToP1);

					v3 dirToPrev = prevLocation - p0;
					dirToPrev.z = 0.0f;
					f32 l1 = glm::length(dirToPrev);

					f32 angle = glm::acos(glm::dot(dirToP1, dirToPrev) / l1);
					f32 dist = l1 * glm::sin(angle);

					if (dist < snapDistance)
					{
						f32 c = l1 * glm::cos(angle);
						if (c >= -0.5f * roadLength && c <= 1.5f * roadLength) // needs lil' bit more length to each directions
						{
							prevLocation = rs.GetStartPosition();
							m_SnappedRoadSegment = rsIndex;
							v3 r{
								0.0f,
								rs.GetStartRotation().x,
								rs.GetStartRotation().y + glm::radians(180.f)
							};
							m_Guideline->SetTransform(prevLocation, r);
							goto snapped;
						}
					}
					p0 = p1;
				}
			}
		}
	snapped:
		int a = 1;
	}
	void CarManager::OnUpdate_Removing(v3& prevLocation, const v3& cameraPosition, const v3& cameraDirection)
	{
		m_SelectedCarToRemove = m_Cars.end();

		for (auto& it = m_Cars.begin(); it != m_Cars.end(); ++it)
		{
			Car* car = *it;
			Object* obj = car->object;
			obj->tintColor = v4(1.0f);

			if (Helper::CheckBoundingBoxHit(
				cameraPosition,
				cameraDirection,
				obj->prefab->boundingBoxL + obj->position,
				obj->prefab->boundingBoxM + obj->position
			))
			{
				m_SelectedCarToRemove = it;
				obj->tintColor = v4{ 1.0f, 0.3f, 0.2f, 1.0f };
				break;
			}
		}
	}

	bool CarManager::OnMousePressed(MouseCode button)
	{
		switch (m_ConstructionMode)
		{
		case CarConstructionMode::None:
			break;
		case CarConstructionMode::Adding:
			OnMousePressed_Adding();
			break;
		case CarConstructionMode::Removing:
			OnMousePressed_Removing();
			break;
		default:
			break;
		}
		return false;
	}
	bool CarManager::OnMousePressed_Adding()
	{
		if (m_SnappedRoadSegment != -1)
		{
			auto& segments = m_Scene->m_RoadManager.m_Segments;
			std::vector<f32> ts{ 0 };
			f32 lengthRoad = m_Scene->MainApplication->cars[m_Type]->boundingBoxM.x - m_Scene->MainApplication->cars[m_Type]->boundingBoxL.x;
			std::vector<v3> samples = Math::GetCubicCurveSamples(segments[m_SnappedRoadSegment].GetCurvePoints(), lengthRoad, ts);
			v3 targeT = samples[1];
			Car* car = new Car(
				m_Scene->MainApplication->cars[m_Type],
				m_SnappedRoadSegment,
				0.0f,
				1.5,
				m_Guideline->position,
				targeT,
				m_Guideline->rotation
			);
			car->type = m_Type;
			m_Cars.push_back(car);
			segments[m_SnappedRoadSegment].Cars.push_back(car);
			ResetStates();
			m_Guideline->enabled = true;
		}
		return false;
	}
	bool CarManager::OnMousePressed_Removing()
	{
		if (m_SelectedCarToRemove != m_Cars.end())
		{
			Car* r_car = (*m_SelectedCarToRemove);
			RoadSegment& rs = m_Scene->m_RoadManager.m_Segments[r_car->roadSegment];
			rs.Cars.erase(std::find(rs.Cars.begin(), rs.Cars.end(), r_car));
			Object* car = r_car->object;
			m_Cars.erase(m_SelectedCarToRemove);
			m_SelectedCarToRemove = m_Cars.end();
			delete car;
		}
		return false;
	}

	void CarManager::SetType(u64 type)
	{
		m_Type = type;
		delete m_Guideline;
		m_Guideline = new Object(m_Scene->MainApplication->cars[m_Type]);

	}
	void CarManager::SetConstructionMode(CarConstructionMode mode)
	{
		ResetStates();
		m_ConstructionMode = mode;

		switch (m_ConstructionMode)
		{
		case CarConstructionMode::None:
			break;
		case CarConstructionMode::Adding:
			m_Guideline->enabled = true;
			break;
		case CarConstructionMode::Removing:
			break;
		default:
			break;
		}
	}
	void CarManager::ResetStates()
	{

		m_SelectedCarToRemove = m_Cars.end();

		for (Car* car : m_Cars)
			car->object->tintColor = v4(1.0f);

		m_Guideline->enabled = false;
		m_Guideline->tintColor = v4(1.0f);
		m_SnappedRoadSegment = -1;
	}
}