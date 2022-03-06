#include "canpch.h"
#include "BuildingManager.h"

#include "Types/RoadSegment.h"
#include "Types/Tree.h"
#include "Building.h"

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
		GameApp* app = m_Scene->MainApplication;
		Prefab* selectedBuilding = m_Guideline->prefab;
		f32 buildingWidth = selectedBuilding->boundingBoxM.y - selectedBuilding->boundingBoxL.y;
		f32 buildingDepthFromCenter = -selectedBuilding->boundingBoxL.x;

		bool snappedToRoad = false;
		if (snapOptions[0])
		{
			auto& segments = m_Scene->m_RoadManager.m_Segments;
			u64 count = segments.size();
			for (u64 rsIndex = 0; rsIndex < count; rsIndex++)
			{
				RoadSegment& rs = segments[rsIndex];
				RoadType& type = app->road_types[rs.type];
				if (type.zoneable == false)
					continue;
				f32 roadWidth = type.road_width;
				f32 roadLength = type.road_length;
				f32 snapDistance = buildingDepthFromCenter + (roadWidth * 0.5f);

				const std::array<v3, 4>& vs = rs.GetCurvePoints();
				std::array<std::array<v2, 3>, 2> roadPolygon = Math::GetBoundingBoxOfBezierCurve(vs, snapDistance);

				if (Math::CheckPolygonPointCollision(roadPolygon, (v2)prevLocation))
				{
					std::vector<f32> ts{ 0.0f };
					std::vector<v3> ps = Math::GetCubicCurveSamples(vs, roadLength, ts);
					size_t size = ps.size();
					v3 p0 = ps[0];
					for (size_t i = 1; i < size; i++)
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
								bool r = glm::cross(dirToP1, dirToPrev).z > 0.0f;
								v3 shiftDir{ -dirToP1.y, dirToP1.x, 0.0f };
								v3 shiftAmount = ((f32)r * 2.0f - 1.0f) * shiftDir * snapDistance;
								prevLocation = p0 + (dirToP1 * c) + shiftAmount;
								m_SnappedRoadSegment = rsIndex;
								m_GuidelinePosition = prevLocation;
								f32 rotationOffset = (f32)(dirToP1.x < 0.0f) * glm::radians(180.0f);
								f32 rotation = glm::atan(dirToP1.y / dirToP1.x) + rotationOffset;
								m_GuidelineRotation = v3{
									0.0f,
									0.0f,
									(f32)r * glm::radians(180.0f) + glm::radians(-90.0f) + rotation
								};
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
			if (m_SnappedRoadSegment != -1)
			{
				v2 boundingL = (v2)m_Guideline->prefab->boundingBoxL;
				v2 boundingM = (v2)m_Guideline->prefab->boundingBoxM;
				v2 boundingP = (v2)prevLocation;
				auto& segments = m_Scene->m_RoadManager.m_Segments;

				for (Building* building : segments[m_SnappedRoadSegment].Buildings)
				{
					v2 bL = (v2)building->object->prefab->boundingBoxL;
					v2 bM = (v2)building->object->prefab->boundingBoxM;
					v2 bP = (v2)building->position;

					v2 mtv = Helper::CheckRotatedRectangleCollision(
						bL,
						bM,
						building->object->rotation.z,
						bP,
						boundingL,
						boundingM,
						m_GuidelineRotation.z,
						boundingP
					);
					if (glm::length(mtv) > 0.0f)
					{
						prevLocation += v3(mtv, 0.0f);
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
			Prefab* prefab = m_Guideline->prefab;
			f32 building_height = prefab->boundingBoxM.z - prefab->boundingBoxL.z;

			v3 A = v3{ prefab->boundingBoxL.x, prefab->boundingBoxL.y, prefab->boundingBoxL.z };
			v3 B = v3{ prefab->boundingBoxL.x, prefab->boundingBoxM.y, prefab->boundingBoxL.z };
			v3 C = v3{ prefab->boundingBoxM.x, prefab->boundingBoxL.y, prefab->boundingBoxL.z };
			v3 D = v3{ prefab->boundingBoxM.x, prefab->boundingBoxM.y, prefab->boundingBoxL.z };


			f32 rot = m_Guideline->rotation.z;
			A = glm::rotateZ(A, rot) + m_Guideline->position;
			B = glm::rotateZ(B, rot) + m_Guideline->position;
			C = glm::rotateZ(C, rot) + m_Guideline->position;
			D = glm::rotateZ(D, rot) + m_Guideline->position;

			std::vector<std::array<v3, 3>> building_bounding_polygon = {
				std::array<v3, 3>{A, B, D},
				std::array<v3, 3>{A, C, D}
			};

			std::array<v3, 2> building_bounding_box{
				v3{std::min({A.x, B.x, C.x, D.x}), std::min({A.y, B.y, C.y, D.y}), A.z},
				v3{std::max({A.x, B.x, C.x, D.x}), std::max({A.y, B.y, C.y, D.y}), A.z + building_height}
			};

			auto& segments = m_Scene->m_RoadManager.m_Segments;
			u64 count = segments.size();
			for (u64 rsIndex = 0; rsIndex < count; rsIndex++)
			{
				RoadSegment& rs = segments[rsIndex];
				RoadType& type = app->road_types[rs.type];
				if (rsIndex == m_SnappedRoadSegment)
					continue;

				f32 road_height = rs.elevation_type == -1 ? type.tunnel_height : type.road_height;
				std::array<v3, 2> road_bounding_box{
					rs.object->prefab->boundingBoxL + rs.CurvePoints[0],
					rs.object->prefab->boundingBoxM + rs.CurvePoints[0]
				};

				if (Math::check_bounding_box_bounding_box_collision(road_bounding_box, building_bounding_box))
				{
					if (Math::check_bounding_polygon_bounding_polygon_collision_with_z(rs.bounding_polygon, road_height, building_bounding_polygon, building_height))
					{
						collidedWithRoad = true;
						break;
					}
				}
			}
		}

		if (restrictions[0] && m_Scene->m_TreeManager.restrictions[0])
		{
			v2 buildingL = (v2)selectedBuilding->boundingBoxL;
			v2 buildingM = (v2)selectedBuilding->boundingBoxM;
			v2 buildingP = (v2)m_GuidelinePosition;
			for (Tree* tree : m_Scene->m_TreeManager.GetTrees())
			{
				v2 treeL = (v2)tree->object->prefab->boundingBoxL;
				v2 treeM = (v2)tree->object->prefab->boundingBoxM;
				v2 treeP = (v2)tree->object->position;
				v2 mtv = Helper::CheckRotatedRectangleCollision(
					treeL,
					treeM,
					tree->object->rotation.z,
					treeP,
					buildingL,
					buildingM,
					m_GuidelineRotation.z,
					buildingP
				);
				tree->object->tintColor = v4(1.0f);
				if (glm::length(mtv) > 0.0f)
					tree->object->tintColor = v4{ 1.0f, 0.3f, 0.2f, 1.0f };
			}
		}

		bool collidedWithOtherBuildings = false;
		if (restrictions[0])
		{
			v2 buildingL = (v2)selectedBuilding->boundingBoxL;
			v2 buildingM = (v2)selectedBuilding->boundingBoxM;
			v2 buildingP = (v2)m_GuidelinePosition;

			for (Building* building : m_Buildings)
			{
				v2 bL = (v2)building->object->prefab->boundingBoxL;
				v2 bM = (v2)building->object->prefab->boundingBoxM;
				v2 bP = (v2)building->position;
				v2 mtv = Helper::CheckRotatedRectangleCollision(
					bL,
					bM,
					building->object->rotation.z,
					bP,
					buildingL,
					buildingM,
					m_GuidelineRotation.z,
					buildingP
				);
				if (glm::length(mtv) > 0.0f)
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
			Building* newBuilding = new Building(
				m_Guideline->prefab,
				m_SnappedRoadSegment,
				m_SnappedT,
				m_GuidelinePosition,
				m_GuidelineRotation
			);
			newBuilding->type = m_Type;
			if (m_SnappedRoadSegment != (u64)-1)
				segments[m_SnappedRoadSegment].Buildings.push_back(newBuilding);
			m_Buildings.push_back(newBuilding);
			ResetStates();
			m_Guideline->enabled = true;

			if (restrictions[0] && m_Scene->m_TreeManager.restrictions[0])
			{
				v2 buildingL = (v2)newBuilding->object->prefab->boundingBoxL;
				v2 buildingM = (v2)newBuilding->object->prefab->boundingBoxM;
				v2 buildingP = (v2)newBuilding->object->position;

				auto& trees = m_Scene->m_TreeManager.GetTrees();
				for (size_t i = 0; i < trees.size(); i++)
				{
					Object* tree = trees[i]->object;
					v2 mtv = Helper::CheckRotatedRectangleCollision(
						buildingL,
						buildingM,
						newBuilding->object->rotation.z,
						buildingP,
						(v2)tree->prefab->boundingBoxL,
						(v2)tree->prefab->boundingBoxM,
						tree->rotation.z,
						(v2)tree->position
					);

					if (glm::length(mtv) > 0.0f)
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