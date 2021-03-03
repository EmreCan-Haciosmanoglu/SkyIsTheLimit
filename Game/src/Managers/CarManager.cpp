#include "canpch.h"
#include "CarManager.h"
#include "Scenes/GameScene.h"
#include "Types/RoadSegment.h"
#include "GameApp.h"

#include "Can/Math.h"

namespace Can
{
	CarManager::CarManager(GameScene* scene)
		: m_Scene(scene)
	{
		m_Guideline = new Object(m_Scene->MainApplication->cars[m_Type], m_Scene->MainApplication->cars[m_Type], glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), false);
	}
	CarManager::~CarManager()
	{
	}

	void CarManager::OnUpdate(glm::vec3& prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection)
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
	void CarManager::OnUpdate_Adding(glm::vec3& prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection)
	{

		m_Guideline->SetTransform(prevLocation);
		Prefab* selectedCar = m_Guideline->type;

		for (RoadSegment* roadSegment : m_Scene->m_RoadManager.GetRoadSegments())
		{
			float roadWidth = roadSegment->Type[0]->boundingBoxM.z - roadSegment->Type[0]->boundingBoxL.z;
			float roadLength = roadSegment->Type[0]->boundingBoxM.x - roadSegment->Type[0]->boundingBoxL.x;
			float snapDistance = roadWidth * 0.5f;

			const std::array<glm::vec3, 4>& vs = roadSegment->GetCurvePoints();
			std::array<std::array<glm::vec2, 3>, 2> roadPolygon = Math::GetBoundingBoxOfBezierCurve(vs, snapDistance);

			if (Math::CheckPolygonPointCollision(roadPolygon, glm::vec2{ prevLocation.x, prevLocation.z }))
			{
				std::vector<float> ts{ 0.0f };
				std::vector<glm::vec3> ps = Math::GetCubicCurveSamples(vs, roadLength, ts);
				size_t size = ps.size();
				glm::vec3 p0 = ps[0];
				for (size_t i = 1; i < size; i++)
				{
					glm::vec3 p1 = ps[i];
					glm::vec3 dirToP1 = p1 - p0;
					dirToP1.y = 0.0f;
					dirToP1 = glm::normalize(dirToP1);

					glm::vec3 dirToPrev = prevLocation - p0;
					float l1 = glm::length(dirToPrev);

					float angle = glm::acos(glm::dot(dirToP1, dirToPrev) / l1);
					float dist = l1 * glm::sin(angle);

					if (dist < snapDistance)
					{
						float c = l1 * glm::cos(angle);
						if (c >= -0.5f * roadLength && c <= 1.5f * roadLength) // needs lil' bit more length to each directions
						{
							prevLocation = roadSegment->GetStartPosition();
							m_SnappedRoadSegment = roadSegment;
							glm::vec3 r{
								0.0f,
								roadSegment->GetStartRotation().y ,
								roadSegment->GetStartRotation().x
							};
							m_Guideline->SetTransform(prevLocation, glm::vec3(1.0f), r);
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
	void CarManager::OnUpdate_Removing(glm::vec3& prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection)
	{
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
		if (m_SnappedRoadSegment != nullptr)
		{
			Object* car = new Object(
				m_Scene->MainApplication->cars[m_Type],
				m_Scene->MainApplication->cars[m_Type],
				m_Guideline->position,
				glm::vec3{ 1.0f, 1.0f, 1.0f },
				m_Guideline->rotation
			);
			m_Cars.push_back(car);
			ResetStates();
			m_Guideline->enabled = true;
		}
		return false;
	}
	bool CarManager::OnMousePressed_Removing()
	{
		return false;
	}

	void CarManager::SetType(size_t type)
	{
		m_Type = type;
		delete m_Guideline;
		m_Guideline = new Object(m_Scene->MainApplication->cars[m_Type], m_Scene->MainApplication->cars[m_Type], glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f));

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
	}
}