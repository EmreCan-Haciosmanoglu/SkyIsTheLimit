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
		m_Guideline = new Object(m_Scene->MainApplication->buildings[m_Type]);
		m_Guideline->enabled = false;
	}
	BuildingManager::~BuildingManager()
	{
	}

	void BuildingManager::OnUpdate(v3& prevLocation, const v3& cameraPosition, const v3& cameraDirection)
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
	void BuildingManager::OnUpdate_Construction(v3& prevLocation, const v3& cameraPosition, const v3& cameraDirection)
	{
		b_ConstructionRestricted = true;
		m_SnappedRoadSegment = (u64)-1;
		m_Guideline->SetTransform(prevLocation);
		m_GuidelinePosition = prevLocation;

		Prefab* selectedBuilding = m_Guideline->prefab;
		float buildingWidth = selectedBuilding->boundingBoxM.z - selectedBuilding->boundingBoxL.z;
		float buildingDepthFromCenter = -selectedBuilding->boundingBoxL.x;

		bool snappedToRoad = false;
		if (snapOptions[0])
		{
			auto& segments = m_Scene->m_RoadManager.m_Segments;
			u64 count = segments.size();
			for(u64 rsIndex = 0; rsIndex<count; rsIndex++)
			{
				RoadSegment& rs = segments[rsIndex];
				float roadWidth = rs.road_type.width;
				float roadLength = rs.road_type.length;
				float snapDistance = buildingDepthFromCenter + (roadWidth * 0.5f);

				const std::array<v3, 4>& vs = rs.GetCurvePoints();
				std::array<std::array<v2, 3>, 2> roadPolygon = Math::GetBoundingBoxOfBezierCurve(vs, snapDistance);

				if (Math::CheckPolygonPointCollision(roadPolygon, v2{ prevLocation.x, prevLocation.z }))
				{
					std::vector<float> ts{ 0.0f };
					std::vector<v3> ps = Math::GetCubicCurveSamples(vs, roadLength, ts);
					size_t size = ps.size();
					v3 p0 = ps[0];
					for (size_t i = 1; i < size; i++)
					{
						v3 p1 = ps[i];
						v3 dirToP1 = p1 - p0;
						dirToP1.y = 0.0f;
						dirToP1 = glm::normalize(dirToP1);

						v3 dirToPrev = prevLocation - p0;
						float l1 = glm::length(dirToPrev);

						float angle = glm::acos(glm::dot(dirToP1, dirToPrev) / l1);
						float dist = l1 * glm::sin(angle);

						if (dist < snapDistance)
						{
							float c = l1 * glm::cos(angle);
							if (c >= -0.5f * roadLength && c <= 1.5f * roadLength) // needs lil' bit more length to each directions
							{
								bool r = glm::cross(dirToP1, dirToPrev).y < 0.0f;
								v3 shiftDir{ -dirToP1.z, 0.0f, dirToP1.x };
								v3 shiftAmount = ((float)r * 2.0f - 1.0f) * shiftDir * snapDistance;
								prevLocation = p0 + (dirToP1 * c) + shiftAmount;
								m_SnappedRoadSegment = rsIndex;
								m_GuidelinePosition = prevLocation;
								float rotationOffset = (float)(dirToP1.x < 0.0f) * glm::radians(180.0f);
								float rotation = glm::atan(-dirToP1.z / dirToP1.x) + rotationOffset;
								m_GuidelineRotation = v3{ 0.0f, (float)r * glm::radians(180.0f) + glm::radians(90.0f) + rotation, 0.0f };
								m_Guideline->SetTransform(m_GuidelinePosition, m_GuidelineRotation);
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
				v2 boundingL{ m_Guideline->prefab->boundingBoxL.x, m_Guideline->prefab->boundingBoxL.z };
				v2 boundingM{ m_Guideline->prefab->boundingBoxM.x, m_Guideline->prefab->boundingBoxM.z };
				v2 boundingP{ prevLocation.x, prevLocation.z };
				auto& segments = m_Scene->m_RoadManager.m_Segments;
				
				for (Building* building : segments[m_SnappedRoadSegment].Buildings)
				{
					v2 bL{ building->object->prefab->boundingBoxL.x, building->object->prefab->boundingBoxL.z };
					v2 bM{ building->object->prefab->boundingBoxM.x, building->object->prefab->boundingBoxM.z };
					v2 bP{ building->position.x, building->position.z };

					v2 mtv = Helper::CheckRotatedRectangleCollision(
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
						prevLocation += v3{ mtv.x, 0.0f, mtv.y };
						m_GuidelinePosition = prevLocation;
						m_Guideline->SetTransform(m_GuidelinePosition, m_GuidelineRotation);
						break;
					}
				}
			}
		}

		bool collidedWithRoad = false;
		if (restrictions[0] && (m_Scene->m_RoadManager.restrictionFlags & 0x4/*change with #define*/))
		{
			v2 pos{ m_Guideline->position.x, m_Guideline->position.z };
			v2 A{ m_Guideline->prefab->boundingBoxL.x, m_Guideline->prefab->boundingBoxL.z };
			v2 D{ m_Guideline->prefab->boundingBoxM.x, m_Guideline->prefab->boundingBoxM.z };
			v2 B{ A.x, D.y }; // this is faster right???
			v2 C{ D.x, A.y }; // this is faster right???

			float rot = m_Guideline->rotation.y;
			A = Math::RotatePoint(A, rot) + pos;
			B = Math::RotatePoint(B, rot) + pos;
			C = Math::RotatePoint(C, rot) + pos;
			D = Math::RotatePoint(D, rot) + pos;

			std::array<std::array<v2, 3>, 2> polygonBuilding = {
				std::array<v2,3>{A, B, D},
				std::array<v2,3>{A, C, D}
			};

			auto& segments = m_Scene->m_RoadManager.m_Segments;
			u64 count = segments.size();
			for(u64 rsIndex = 0; rsIndex<count;rsIndex++)
			{
				RoadSegment& rs = segments[rsIndex];
				if (rsIndex == m_SnappedRoadSegment)
					continue;
				float roadPrefabWidth = rs.road_type.width;
				const std::array<v3, 4>& cps = rs.GetCurvePoints();
				std::array<std::array<v2, 3>, 2> newRoadBoundingBox = Math::GetBoundingBoxOfBezierCurve(cps, roadPrefabWidth * 0.5f);

				if (Math::CheckPolygonCollision(newRoadBoundingBox, polygonBuilding))
				{
					std::array<std::array<v2, 3>, (10 - 1) * 2> newRoadBoundingPolygon = Math::GetBoundingPolygonOfBezierCurve<10, 10>(cps, roadPrefabWidth * 0.5f);
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
			v2 buildingL{ selectedBuilding->boundingBoxL.x, selectedBuilding->boundingBoxL.z };
			v2 buildingM{ selectedBuilding->boundingBoxM.x, selectedBuilding->boundingBoxM.z };
			v2 buildingP{ m_GuidelinePosition.x, m_GuidelinePosition.z };
			for (Object* tree : m_Scene->m_TreeManager.GetTrees())
			{
				v2 treeL{ tree->prefab->boundingBoxL.x, tree->prefab->boundingBoxL.z };
				v2 treeM{ tree->prefab->boundingBoxM.x, tree->prefab->boundingBoxM.z };
				v2 treeP{ tree->position.x, tree->position.z };
				v2 mtv = Helper::CheckRotatedRectangleCollision(
					treeL,
					treeM,
					tree->rotation.y,
					treeP,
					buildingL,
					buildingM,
					m_GuidelineRotation.y,
					buildingP
				);
				tree->tintColor = v4(1.0f);
				if (mtv.x != 0.0f || mtv.y != 0.0f)
					tree->tintColor = v4{ 1.0f, 0.3f, 0.2f, 1.0f };
			}
		}

		bool collidedWithOtherBuildings = false;
		if (restrictions[0])
		{
			v2 buildingL{ selectedBuilding->boundingBoxL.x, selectedBuilding->boundingBoxL.z };
			v2 buildingM{ selectedBuilding->boundingBoxM.x, selectedBuilding->boundingBoxM.z };
			v2 buildingP{ m_GuidelinePosition.x, m_GuidelinePosition.z };

			for (Building* building : m_Buildings)
			{
				v2 bL{ building->object->prefab->boundingBoxL.x, building->object->prefab->boundingBoxL.z };
				v2 bM{ building->object->prefab->boundingBoxM.x, building->object->prefab->boundingBoxM.z };
				v2 bP{ building->position.x, building->position.z };
				v2 mtv = Helper::CheckRotatedRectangleCollision(
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
		m_Guideline->tintColor = b_ConstructionRestricted ? v4{ 1.0f, 0.3f, 0.2f, 1.0f } : v4(1.0f);
	}
	void BuildingManager::OnUpdate_Destruction(v3& prevLocation, const v3& cameraPosition, const v3& cameraDirection)
	{
		m_SelectedBuildingToDestruct = m_Buildings.end();
		for (auto& it = m_Buildings.begin(); it != m_Buildings.end(); ++it)
		{
			Building* building = *it;
			building->object->tintColor = v4(1.0f);

			if (Helper::CheckBoundingBoxHit(
				cameraPosition,
				cameraDirection,
				building->object->prefab->boundingBoxL + building->position,
				building->object->prefab->boundingBoxM + building->position
			))
			{
				m_SelectedBuildingToDestruct = it;
				building->object->tintColor = v4{ 1.0f, 0.3f, 0.2f, 1.0f };
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
			auto& segments = m_Scene->m_RoadManager.m_Segments;
			Building* newBuilding = new Building(m_Guideline->prefab, m_SnappedRoadSegment, m_SnappedT, m_GuidelinePosition, m_GuidelineRotation);
			if (m_SnappedRoadSegment != (u64)-1)
				segments[m_SnappedRoadSegment].Buildings.push_back(newBuilding);
			m_Buildings.push_back(newBuilding);
			ResetStates();
			m_Guideline->enabled = true;

			if (restrictions[0] && m_Scene->m_TreeManager.restrictions[0])
			{
				v2 buildingL = { newBuilding->object->prefab->boundingBoxL.x, newBuilding->object->prefab->boundingBoxL.z };
				v2 buildingM = { newBuilding->object->prefab->boundingBoxM.x, newBuilding->object->prefab->boundingBoxM.z };
				v2 buildingP = { newBuilding->object->position.x, newBuilding->object->position.z };

				auto& trees = m_Scene->m_TreeManager.GetTrees();
				for (size_t i = 0; i < trees.size(); i++)
				{
					Object* tree = trees[i];
					v2 mtv = Helper::CheckRotatedRectangleCollision(
						buildingL,
						buildingM,
						newBuilding->object->rotation.y,
						buildingP,
						v2{ tree->prefab->boundingBoxL.x ,tree->prefab->boundingBoxL.z },
						v2{ tree->prefab->boundingBoxM.x ,tree->prefab->boundingBoxM.z },
						tree->rotation.y,
						v2{ tree->position.x, tree->position.z }
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
				auto& segments = m_Scene->m_RoadManager.m_Segments;
				std::vector<Building*>& connectedBuildings = segments[building->connectedRoadSegment].Buildings;
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
		m_Guideline = new Object(m_Scene->MainApplication->buildings[m_Type]);
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
		m_GuidelinePosition = v3(-1.0f);
		m_GuidelineRotation = v3(0.0f);

		m_SnappedRoadSegment = (u64)-1;
		m_SnappedT = -1.0f;

		m_SelectedBuildingToDestruct = m_Buildings.end();

		for (Building* building : m_Buildings)
			building->object->tintColor = v4(1.0f);

		m_Guideline->enabled = false;
		m_Guideline->tintColor = v4(1.0f);
	}
}