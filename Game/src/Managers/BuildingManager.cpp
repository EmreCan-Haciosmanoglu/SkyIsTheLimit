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

	void BuildingManager::OnUpdate(glm::vec3& prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection)
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
	void BuildingManager::OnUpdate_Construction(glm::vec3& prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection)
	{
		b_ConstructionRestricted = true;
		m_SnappedRoadSegment = nullptr;
		m_Guideline->SetTransform(prevLocation);
		m_GuidelinePosition = prevLocation;

		Prefab* selectedBuilding = m_Guideline->type;
		float buildingWidth = selectedBuilding->boundingBoxM.z - selectedBuilding->boundingBoxL.z;
		float buildingDepthFromCenter = -selectedBuilding->boundingBoxL.x;

		bool snappedToRoad = false;
		if (snapOptions[0])
		{
			for (RoadSegment* roadSegment : m_Scene->m_RoadManager.GetRoadSegments())
			{
				float roadWidth = roadSegment->Type[0]->boundingBoxM.z - roadSegment->Type[0]->boundingBoxL.z;
				float roadLength = roadSegment->Type[0]->boundingBoxM.x - roadSegment->Type[0]->boundingBoxL.x;
				float snapDistance = buildingDepthFromCenter + (roadWidth * 0.5f);

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
								bool r = glm::cross(dirToP1, dirToPrev).y < 0.0f;
								glm::vec3 shiftDir{ -dirToP1.z, 0.0f, dirToP1.x };
								glm::vec3 shiftAmount = ((float)r * 2.0f - 1.0f) * shiftDir * snapDistance;
								prevLocation = p0 + (dirToP1 * c) + shiftAmount;
								m_SnappedRoadSegment = roadSegment;
								m_GuidelinePosition = prevLocation;
								float rotationOffset = (float)(dirToP1.x < 0.0f) * glm::radians(180.0f);
								float rotation = glm::atan(-dirToP1.z / dirToP1.x) + rotationOffset;
								m_GuidelineRotation = glm::vec3{ 0.0f, (float)r * glm::radians(180.0f) + glm::radians(90.0f) + rotation, 0.0f };
								m_Guideline->SetTransform(m_GuidelinePosition, glm::vec3(1.0f), m_GuidelineRotation);
								snappedToRoad = true;
								m_SnappedT = ts[i];
								goto snapped;
							}
						}
						p0 = p1;
					}
				}
			}
		}
	snapped:

		if (snapOptions[1])
		{
			if (m_SnappedRoadSegment)
			{
				glm::vec2 boundingL{ m_Guideline->prefab->boundingBoxL.x, m_Guideline->prefab->boundingBoxL.z };
				glm::vec2 boundingM{ m_Guideline->prefab->boundingBoxM.x, m_Guideline->prefab->boundingBoxM.z };
				glm::vec2 boundingP{ prevLocation.x, prevLocation.z };

				for (Building* building : m_SnappedRoadSegment->Buildings)
				{
					glm::vec2 bL{ building->object->prefab->boundingBoxL.x, building->object->prefab->boundingBoxL.z };
					glm::vec2 bM{ building->object->prefab->boundingBoxM.x, building->object->prefab->boundingBoxM.z };
					glm::vec2 bP{ building->position.x, building->position.z };

					glm::vec2 mtv = Helper::CheckRotatedRectangleCollision(
						bL,
						bM,
						building->object->rotation.y,
						bP,
						boundingL,
						boundingM,
						m_GuidelineRotation.y,
						boundingP
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
			glm::vec2 buildingL{ selectedBuilding->boundingBoxL.x, selectedBuilding->boundingBoxL.z };
			glm::vec2 buildingM{ selectedBuilding->boundingBoxM.x, selectedBuilding->boundingBoxM.z };
			glm::vec2 buildingP{ m_GuidelinePosition.x, m_GuidelinePosition.z };
			for (Object* tree : m_Scene->m_TreeManager.GetTrees())
			{
				glm::vec2 treeL{ tree->prefab->boundingBoxL.x, tree->prefab->boundingBoxL.z };
				glm::vec2 treeM{ tree->prefab->boundingBoxM.x, tree->prefab->boundingBoxM.z };
				glm::vec2 treeP{ tree->position.x, tree->position.z };
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
			glm::vec2 buildingL{ selectedBuilding->boundingBoxL.x, selectedBuilding->boundingBoxL.z };
			glm::vec2 buildingM{ selectedBuilding->boundingBoxM.x, selectedBuilding->boundingBoxM.z };
			glm::vec2 buildingP{ m_GuidelinePosition.x, m_GuidelinePosition.z };

			for (Building* building : m_Buildings)
			{
				glm::vec2 bL{ building->object->prefab->boundingBoxL.x, building->object->prefab->boundingBoxL.z };
				glm::vec2 bM{ building->object->prefab->boundingBoxM.x, building->object->prefab->boundingBoxM.z };
				glm::vec2 bP{ building->position.x, building->position.z };
				glm::vec2 mtv = Helper::CheckRotatedRectangleCollision(
					bL,
					bM,
					building->object->rotation.y,
					bP,
					buildingL,
					buildingM,
					m_GuidelineRotation.y,
					buildingP
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
	void BuildingManager::OnUpdate_Destruction(glm::vec3& prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection)
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
			Building* newBuilding = new Building(m_Guideline->type, m_SnappedRoadSegment, m_SnappedT, m_GuidelinePosition, m_GuidelineRotation);
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
		ResetStates();
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
		m_SnappedT = -1.0f;

		m_SelectedBuildingToDestruct = m_Buildings.end();

		for (Building* building : m_Buildings)
			building->object->tintColor = glm::vec4(1.0f);

		m_Guideline->enabled = false;
		m_Guideline->tintColor = glm::vec4(1.0f);
	}
}