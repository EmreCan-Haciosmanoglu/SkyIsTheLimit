#include "canpch.h"
#include "BuildingManager.h"

#include "Types/RoadSegment.h"
#include "Junction.h"
#include "Building.h"
#include "End.h"

#include "GameApp.h"
#include "Scenes/GameScene.h"
#include "RoadManager.h"
#include "TreeManager.h"
#include "Helper.h"

#include "Can/Math.h"

namespace Can
{
	BuildingManager::BuildingManager(GameScene* scene)
		: m_Scene(scene)
	{
		m_Guideline = new Object(m_Scene->MainApplication->buildings[m_Type], m_Scene->MainApplication->buildings[m_Type], glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), false);
	}
	BuildingManager::~BuildingManager()
	{
	}

	void BuildingManager::OnUpdate(glm::vec3 prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection)
	{
		switch (m_ConstructionMode)
		{
		case BuildingConstructionMode::None:
			break;
		case BuildingConstructionMode::Construct:
			OnUpdate_Construction(prevLocation, cameraPosition, cameraDirection);
			break;
		case BuildingConstructionMode::Upgrade:
			break;
		case BuildingConstructionMode::Destruct:
			OnUpdate_Destruction(prevLocation, cameraPosition, cameraDirection);
			break;
		default:
			break;
		}
	}
	void BuildingManager::OnUpdate_Construction(glm::vec3 prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection)
	{
		b_ConstructionRestricted = true;
		m_SnappedRoadSegment = nullptr;
		m_Guideline->SetTransform(prevLocation);
		m_GuidelinePosition = prevLocation;

		Prefab* selectedBuilding = m_Guideline->type;
		float buildingWidth = selectedBuilding->boundingBoxM.z - selectedBuilding->boundingBoxL.z;
		float buildingDepthFromCenter = -selectedBuilding->boundingBoxL.x;

		bool snappedToRoad = false;
		/* Update Later
		if (snapOptions[0])
		{
			for (Road* road : m_Scene->m_RoadManager.GetRoads())
			{
				float roadWidth = road->object->prefab->boundingBoxM.z - road->object->prefab->boundingBoxL.z;
				glm::vec3& roadDir = road->direction;
				roadDir.y = 0.0f;
				float snapDistance = buildingDepthFromCenter + (roadWidth / 2.0f);

				glm::vec3 Intersection = Helper::RayPlaneIntersection(
					cameraPosition,
					cameraDirection,
					road->GetStartPosition(),
					{ 0.0f, 1.0f, 0.0f, }
				);

				glm::vec3 B = Intersection - road->GetStartPosition();
				float bLength = glm::length(B);

				float angle = glm::acos(glm::dot(roadDir, B) / bLength);
				float distance = bLength * glm::sin(angle);

				if (distance < snapDistance)
				{
					bool right = (glm::cross(B, roadDir)).y > 0.0f;
					glm::vec3 shiftDir = glm::normalize(glm::rotate(roadDir, glm::radians(-90.0f), glm::vec3{ 0.0f, 1.0f, 0.0f }));
					glm::vec3 shiftAmount = (right * 2.0f - 1.0f) * shiftDir * snapDistance;

					float c = bLength * glm::cos(angle);
					if (c <= 0 || c >= road->length)
						continue;
					prevLocation = road->GetStartPosition() + road->direction * c + shiftAmount;
					m_SnappedRoad = road;
					m_GuidelinePosition = prevLocation;
					m_GuidelineRotation = { 0.0f, right * glm::radians(180.0f) + glm::radians(90.0f) + road->rotation.y ,0.0f };
					m_Guideline->SetTransform(m_GuidelinePosition, glm::vec3(1.0f), m_GuidelineRotation);
					snappedToRoad = true;
					break;
				}
			}
		}
		*/

		if (snapOptions[1])
		{
			if (m_SnappedRoadSegment)
			{
				glm::vec2 boundingSquareL = { m_Guideline->prefab->boundingBoxL.x, m_Guideline->prefab->boundingBoxL.z };
				glm::vec2 boundingSquareM = { m_Guideline->prefab->boundingBoxM.x, m_Guideline->prefab->boundingBoxM.z };

				for (Building* building : m_SnappedRoadSegment->Buildings)
				{
					Object* object = building->object;
					glm::vec2 mtv = Helper::CheckRotatedRectangleCollision(
						{ object->prefab->boundingBoxL.x, object->prefab->boundingBoxL.z },
						{ object->prefab->boundingBoxM.x, object->prefab->boundingBoxM.z },
						object->rotation.y,
						{ object->position.x, object->position.z },
						boundingSquareL,
						boundingSquareM,
						m_GuidelineRotation.y,
						{ m_GuidelinePosition.x, m_GuidelinePosition.z }
					);
					if (mtv.x != 0.0f || mtv.y != 0.0f)
					{
						prevLocation += glm::vec3{ mtv.x, 0.0f, mtv.y };
						m_GuidelinePosition = prevLocation;
						m_Guideline->SetTransform(m_GuidelinePosition, glm::vec3(1.0f), m_GuidelineRotation);
						break;
					}
				}
			}
		}

		bool collidedWithRoad = false;
		if (restrictions[0] && m_Scene->m_RoadManager.restrictions[2])
		{
			glm::vec2 pos{ m_Guideline->position.x, m_Guideline->position.z };
			glm::vec2 A{ m_Guideline->prefab->boundingBoxL.x, m_Guideline->prefab->boundingBoxL.z };
			glm::vec2 D{ m_Guideline->prefab->boundingBoxM.x, m_Guideline->prefab->boundingBoxM.z };
			glm::vec2 B{ A.x, D.y }; // this is faster right???
			glm::vec2 C{ D.x, A.y }; // this is faster right???

			float rot = m_Guideline->rotation.y;
			A = Math::RotatePoint(A, rot) + pos;
			B = Math::RotatePoint(B, rot) + pos;
			C = Math::RotatePoint(C, rot) + pos;
			D = Math::RotatePoint(D, rot) + pos;

			std::array<std::array<glm::vec2, 3>, 2> polygonBuilding = {
				std::array<glm::vec2,3>{A, B, D},
				std::array<glm::vec2,3>{A, C, D}
			};

			for (RoadSegment* roadSegment : m_Scene->m_RoadManager.GetRoadSegments())
			{
				if (roadSegment == m_SnappedRoadSegment)
					continue;
				float roadPrefabWidth = roadSegment->Type[0]->boundingBoxM.z - roadSegment->Type[0]->boundingBoxL.z;
				const std::array<glm::vec3, 4>& cps = roadSegment->GetCurvePoints();
				std::array<std::array<glm::vec2, 3>, 2> newRoadBoundingBox = Math::GetBoundingBoxOfBezierCurve(cps, roadPrefabWidth * 0.5f);

				if (Math::CheckPolygonCollision(newRoadBoundingBox, polygonBuilding))
				{
					std::array<std::array<glm::vec2, 3>, (10 - 1) * 2> newRoadBoundingPolygon = Math::GetBoundingPolygonOfBezierCurve<10, 10>(cps, roadPrefabWidth * 0.5f);
					if (Math::CheckPolygonCollision(newRoadBoundingPolygon, polygonBuilding))
					{
						collidedWithRoad = true;
						break;
					}
				}
			}
		}

		if (restrictions[0] && m_Scene->m_TreeManager.restrictions[0])
		{
			glm::vec2 buildingL = { selectedBuilding->boundingBoxL.x, selectedBuilding->boundingBoxL.z };
			glm::vec2 buildingM = { selectedBuilding->boundingBoxM.x, selectedBuilding->boundingBoxM.z };
			glm::vec2 buildingP = { m_GuidelinePosition.x, m_GuidelinePosition.z };
			for (Object* tree : m_Scene->m_TreeManager.GetTrees())
			{
				glm::vec2 treeL = { tree->prefab->boundingBoxL.x, tree->prefab->boundingBoxL.z };
				glm::vec2 treeM = { tree->prefab->boundingBoxM.x, tree->prefab->boundingBoxM.z };
				glm::vec2 treeP = { tree->position.x, tree->position.z };
				glm::vec2 mtv = Helper::CheckRotatedRectangleCollision(
					treeL,
					treeM,
					tree->rotation.y,
					treeP,
					buildingL,
					buildingM,
					m_GuidelineRotation.y,
					buildingP
				);
				tree->tintColor = glm::vec4(1.0f);
				if (mtv.x != 0.0f || mtv.y != 0.0f)
					tree->tintColor = glm::vec4{ 1.0f, 0.3f, 0.2f, 1.0f };
			}
		}

		bool collidedWithOtherBuildings = false;
		if (restrictions[0])
		{
			glm::vec2 buildingL = { selectedBuilding->boundingBoxL.x, selectedBuilding->boundingBoxL.z };
			glm::vec2 buildingM = { selectedBuilding->boundingBoxM.x, selectedBuilding->boundingBoxM.z };
			for (Building* building : m_Buildings)
			{
				glm::vec2 bL = { building->object->prefab->boundingBoxL.x, building->object->prefab->boundingBoxL.z };
				glm::vec2 bM = { building->object->prefab->boundingBoxM.x, building->object->prefab->boundingBoxM.z };

				glm::vec2 mtv = Helper::CheckRotatedRectangleCollision(
					bL,
					bM,
					building->object->rotation.y,
					glm::vec2{ building->position.x, building->position.z },
					buildingL,
					buildingM,
					m_GuidelineRotation.y,
					glm::vec2{ m_GuidelinePosition.x, m_GuidelinePosition.z }
				);
				if (mtv.x != 0.0f || mtv.y != 0.0f)
				{
					collidedWithOtherBuildings = true;
					break;
				}
			}
		}

		b_ConstructionRestricted = (restrictions[1] && !snappedToRoad) || collidedWithRoad || collidedWithOtherBuildings;
		m_Guideline->tintColor = b_ConstructionRestricted ? glm::vec4{ 1.0f, 0.3f, 0.2f, 1.0f } : glm::vec4(1.0f);
	}
	void BuildingManager::OnUpdate_Destruction(glm::vec3 prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection)
	{
		m_SelectedBuildingToDestruct = m_Buildings.end();
		for (auto& it = m_Buildings.begin(); it != m_Buildings.end(); ++it)
		{
			Building* building = *it;
			building->object->tintColor = glm::vec4(1.0f);

			if (Helper::CheckBoundingBoxHit(
				cameraPosition,
				cameraDirection,
				building->object->prefab->boundingBoxL + building->position,
				building->object->prefab->boundingBoxM + building->position
			))
			{
				m_SelectedBuildingToDestruct = it;
				building->object->tintColor = glm::vec4{ 1.0f, 0.3f, 0.2f, 1.0f };
				break;
			}
		}
	}

	bool BuildingManager::OnMousePressed(MouseCode button)
	{
		switch (m_ConstructionMode)
		{
		case BuildingConstructionMode::None:
			break;
		case BuildingConstructionMode::Construct:
			if (button != MouseCode::Button0)
				return false;
			OnMousePressed_Construction();
			break;
		case BuildingConstructionMode::Upgrade:
			break;
		case BuildingConstructionMode::Destruct:
			if (button != MouseCode::Button0)
				return false;
			OnMousePressed_Destruction();
			break;
		default:
			break;
		}
		return false;
	}
	bool BuildingManager::OnMousePressed_Construction()
	{
		if (!b_ConstructionRestricted)
		{
			Building* newBuilding = new Building(m_Guideline->type, m_SnappedRoadSegment, m_GuidelinePosition, m_GuidelineRotation);
			if (m_SnappedRoadSegment)
				m_SnappedRoadSegment->Buildings.push_back(newBuilding);
			m_Buildings.push_back(newBuilding);
			ResetStates();
			m_Guideline->enabled = true;

			if (restrictions[0] && m_Scene->m_TreeManager.restrictions[0])
			{
				glm::vec2 buildingL = { newBuilding->object->prefab->boundingBoxL.x, newBuilding->object->prefab->boundingBoxL.z };
				glm::vec2 buildingM = { newBuilding->object->prefab->boundingBoxM.x, newBuilding->object->prefab->boundingBoxM.z };
				glm::vec2 buildingP = { newBuilding->object->position.x, newBuilding->object->position.z };

				auto& trees = m_Scene->m_TreeManager.GetTrees();
				for (size_t i = 0; i < trees.size(); i++)
				{
					Object* tree = trees[i];
					glm::vec2 mtv = Helper::CheckRotatedRectangleCollision(
						buildingL,
						buildingM,
						newBuilding->object->rotation.y,
						buildingP,
						glm::vec2{ tree->prefab->boundingBoxL.x ,tree->prefab->boundingBoxL.z },
						glm::vec2{ tree->prefab->boundingBoxM.x ,tree->prefab->boundingBoxM.z },
						tree->rotation.y,
						glm::vec2{ tree->position.x, tree->position.z }
					);

					if (mtv.x != 0.0f || mtv.y != 0.0f)
					{
						trees.erase(trees.begin() + i);
						delete tree;
						i--;
					}
				}
			}
		}
		return false;
	}
	bool BuildingManager::OnMousePressed_Destruction()
	{
		if (m_SelectedBuildingToDestruct != m_Buildings.end())
		{
			Building* building = *m_SelectedBuildingToDestruct;
			if (building->connectedRoadSegment)
			{
				std::vector<Building*>& connectedBuildings = building->connectedRoadSegment->Buildings;
				auto it = std::find(connectedBuildings.begin(), connectedBuildings.end(), building);
				connectedBuildings.erase(it);
			}
			m_Buildings.erase(m_SelectedBuildingToDestruct);
			delete building;
		}
		return false;
	}

	void BuildingManager::SetType(size_t type)
	{
		m_Type = type;
		delete m_Guideline;
		m_Guideline = new Object(m_Scene->MainApplication->buildings[m_Type], m_Scene->MainApplication->buildings[m_Type], glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f));
	}
	void BuildingManager::SetConstructionMode(BuildingConstructionMode mode)
	{
		ResetStates();
		m_ConstructionMode = mode;

		switch (m_ConstructionMode)
		{
		case Can::BuildingConstructionMode::None:
			break;
		case Can::BuildingConstructionMode::Construct:
			m_Guideline->enabled = true;
			break;
		case Can::BuildingConstructionMode::Upgrade:
			break;
		case Can::BuildingConstructionMode::Destruct:
			break;
		default:
			break;
		}
	}

	void BuildingManager::ResetStates()
	{
		m_GuidelinePosition = glm::vec3(-1.0f);
		m_GuidelineRotation = glm::vec3(0.0f);

		m_SnappedRoadSegment = nullptr;

		m_SelectedBuildingToDestruct = m_Buildings.end();

		for (Building* building : m_Buildings)
			building->object->tintColor = glm::vec4(1.0f);

		m_Guideline->enabled = false;
		m_Guideline->tintColor = glm::vec4(1.0f);
	}
}