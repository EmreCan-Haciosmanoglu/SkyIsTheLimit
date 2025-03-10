#include "canpch.h"
#include "BuildingManager.h"

#include "Types/RoadSegment.h"
#include "Types/Road_Type.h"
#include "Types/Vehicle_Type.h"
#include "Types/Building_Type.h"
#include "Types/Tree.h"
#include "Types/Person.h"
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
		m_Guideline = new Object(m_Scene->MainApplication->building_types[building_type_index].prefab);
		m_Guideline->enabled = false;
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
		f32 offset_from_side_road = -selectedBuilding->boundingBoxL.x;

		bool snappedToRoad = false;
		if (snapOptions[0])
		{
			auto& segments = m_Scene->m_RoadManager.road_segments;
			u64 capacity = segments.capacity;
			for (u64 rsIndex = 0; rsIndex < capacity; rsIndex++)
			{
				auto values = segments.values;
				if (values[rsIndex].valid == false)
					continue;
				RoadSegment& rs = values[rsIndex].value;
				Road_Type& type = app->road_types[rs.type];
				if (type.zoneable == false)
					continue;
				f32 roadWidth = type.road_width;
				f32 roadLength = type.road_length;
				f32 snapDistance = offset_from_side_road + (roadWidth * 0.5f);

				const std::array<v3, 4>& vs = rs.GetCurvePoints();
				std::array<std::array<v2, 3>, 2> roadPolygon = Math::GetBoundingBoxOfBezierCurve(vs, snapDistance);

				if (Math::CheckPolygonPointCollision(roadPolygon, (v2)prevLocation))
				{
					auto& ps = rs.curve_samples;
					size_t curve_sample_count = ps.size();
					v3 p_0;
					v3 p_1 = ps[0];
					for (size_t i = 1; i < curve_sample_count; i++)
					{
						p_0 = p_1;
						p_1 = ps[i];
						v3 dir_to_p_1 = p_1 - p_0;
						v3 dir_to_bulding_from_road_center = prevLocation - p_0;
						v3 dir_to_p_2 = (i < curve_sample_count - 1) ? ps[i + 1] - p_1 : rs.GetEndDirection() * -1.0f;

						dir_to_p_1.z = 0.0f;
						dir_to_bulding_from_road_center.z = 0.0f;
						dir_to_p_2.z = 0.0f;


						f32 len_p_0_to_p_1 = glm::length(dir_to_p_1);
						dir_to_p_1 = dir_to_p_1 / len_p_0_to_p_1;
						dir_to_bulding_from_road_center = glm::normalize(dir_to_bulding_from_road_center);
						dir_to_p_2 = glm::normalize(dir_to_p_2);

						v3 rotated_1 = glm::normalize(v3{ dir_to_p_1.y, -dir_to_p_1.x, 0.0f }) * (roadWidth * 0.5f);
						v3 rotated_2 = glm::normalize(v3{ dir_to_p_2.y, -dir_to_p_2.x, 0.0f }) * (roadWidth * 0.5f);

						snapped_from_right = glm::cross(dir_to_p_1, dir_to_bulding_from_road_center).z < 0.0f;
						if (snapped_from_right == false)
						{
							rotated_1 *= -1.0f;
							rotated_2 *= -1.0f;
						}

						v3 road_side_end_point_one = p_0 + rotated_1;
						v3 road_side_end_point_two = p_1 + rotated_2;

						v3 dir_side_road = road_side_end_point_two - road_side_end_point_one;
						v3 dir_to_building_from_side_road = prevLocation - road_side_end_point_one;
						f32 scaler = glm::dot(dir_side_road, dir_to_building_from_side_road) / glm::length2(dir_side_road);
						f32 scaler_max = std::max(1.0f, len_p_0_to_p_1 / glm::length(dir_side_road));
						if (scaler > scaler_max) {

							if (i < curve_sample_count - 1)
								continue;
							scaler = 1.0f;
						}
						else if (scaler > 1.0f)
						{
							scaler = 1.0f;
						}
						else if (scaler < 0.0f)
						{
							if (i == 1)
								break;
							if (scaler < 0.5f)
								break;
							scaler = 0.0f;
						}

						f32 dotted = std::max(-1.0f, std::min(1.0f, glm::dot(dir_to_p_1, dir_to_bulding_from_road_center)));
						f32 angle = glm::acos(dotted);
						f32 lenn = glm::length(prevLocation - p_0);
						f32 dist = lenn * glm::sin(angle);
						if (dist > snapDistance || dist < -snapDistance) continue;

						v3 rotated_3 = glm::normalize(v3{ dir_side_road.y, -dir_side_road.x, 0.0f }) * offset_from_side_road;
						if (snapped_from_right == false)
							rotated_3 *= -1.0f;

						prevLocation = road_side_end_point_one + dir_side_road * scaler + rotated_3;
						f32 rotation_offset = (f32)(dir_side_road.x < 0.0f) * glm::radians(180.0f);
						f32 rotation = glm::atan(dir_side_road.y / dir_side_road.x) + rotation_offset;
						m_SnappedRoadSegment = rsIndex;
						m_GuidelinePosition = prevLocation;
						m_GuidelineRotation = v3{ 0.0f, 0.0f,
							((f32)snapped_from_right - 1.0f) * glm::radians(180.0f) + glm::radians(-90.0f) + rotation
						};
						m_Guideline->SetTransform(m_GuidelinePosition, m_GuidelineRotation);
						snappedToRoad = true;
						snapped_t_index = i - 1;
						snapped_t = scaler;

#if 1
						rsIndex = capacity;
						break;
#elif
						goto snapped;
#endif
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
				auto& segments = m_Scene->m_RoadManager.road_segments;

				for (Building* building : segments[m_SnappedRoadSegment].buildings)
				{
					v2 bL = (v2)building->object->prefab->boundingBoxL;
					v2 bM = (v2)building->object->prefab->boundingBoxM;
					v2 bP = (v2)building->object->position;

					v2 mtv = Helper::check_rotated_rectangle_collision(
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
		if (restrictions[0] && (m_Scene->m_RoadManager.restrictionFlags & (u8)RoadRestrictions::RESTRICT_COLLISIONS))
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

			auto& segments = m_Scene->m_RoadManager.road_segments;
			u64 capacity = segments.capacity;
			for (u64 rsIndex = 0; rsIndex < capacity; rsIndex++)
			{
				auto values = segments.values;
				if (values[rsIndex].valid == false)
					continue;
				RoadSegment& rs = values[rsIndex].value;
				Road_Type& type = app->road_types[rs.type];
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
				v2 mtv = Helper::check_rotated_rectangle_collision(
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
				v2 bP = (v2)building->object->position;
				v2 mtv = Helper::check_rotated_rectangle_collision(
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
		for (auto it = m_Buildings.cbegin(); it != m_Buildings.cend(); ++it)
		{
			Building* building = *it;
			building->object->tintColor = v4(1.0f);

			if (Helper::check_if_ray_intersects_with_bounding_box(
				cameraPosition,
				cameraDirection,
				building->object->prefab->boundingBoxL + building->object->position,
				building->object->prefab->boundingBoxM + building->object->position
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
		auto& building_types = m_Scene->MainApplication->building_types;

		auto& vehicle_types = m_Scene->MainApplication->vehicle_types;
		auto& personal_vehicles = m_Scene->MainApplication->personal_vehicles;
		auto& commercial_vehicles = m_Scene->MainApplication->commercial_vehicles;
		auto& industrial_vehicles = m_Scene->MainApplication->industrial_vehicles;
		auto& ambulances = m_Scene->MainApplication->ambulances;
		auto& police_cars = m_Scene->MainApplication->police_cars;
		auto& garbage_trucks = m_Scene->MainApplication->garbage_trucks;

		auto& person_prefabs = m_Scene->MainApplication->people;

		auto& person_manager = m_Scene->m_PersonManager;
		auto& tree_manager = m_Scene->m_TreeManager;
		auto& car_manager = m_Scene->m_CarManager;

		auto& road_segments = m_Scene->m_RoadManager.road_segments;
		auto& trees = tree_manager.GetTrees();

		if (!b_ConstructionRestricted)
		{
			Building* new_building{ new Building() };
			new_building->object = new Object(
				m_Guideline->prefab,
				m_GuidelinePosition,
				m_GuidelineRotation
			);
			new_building->connected_road_segment = m_SnappedRoadSegment;
			new_building->snapped_t_index = snapped_t_index;
			new_building->snapped_t = snapped_t;
			auto& building_type{ building_types[building_type_index] };
			new_building->type = building_type_index;
			new_building->snapped_to_right = snapped_from_right;
			if (m_SnappedRoadSegment != (u64)-1)
				road_segments[m_SnappedRoadSegment].buildings.push_back(new_building);
			m_Buildings.push_back(new_building);

			switch (building_type.group)
			{
			case Building_Group::House:
			case Building_Group::Residential:
			{
				if (building_type.group == Building_Group::House)
					buildings_houses.push_back(new_building);
				else
					buildings_residential.push_back(new_building);

				u16 domicilled{ random_u16(0, building_type.capacity) };
				for (u64 i{ 0 }; i < domicilled; i++)
				{
					u64 type{ 0 };
					Person* new_person{ new Person(
						person_prefabs[type],
						random_f32(4.0f, 6.0f)
					) };
					new_person->type = type;
					new_person->home = new_building;
					new_person->status = PersonStatus::AtHome;
					new_person->time_left = random_f32(1.0f, 5.0f);
					new_building->people.push_back(new_person);
					bool have_enough_money_to_own_car{ random_f32(1.0f) > 0.4f };
					if (have_enough_money_to_own_car)
					{
						u64 personal_vehicle_index{ random_u64(personal_vehicles.size()) };
						u64 new_vehicle_type_index{ personal_vehicles[personal_vehicle_index] };
						const Vehicle_Type& new_vehicle_type{ vehicle_types[new_vehicle_type_index] };
						Car* new_car{ new Car() };
						new_car->object = new Object(new_vehicle_type.prefab);
						new_car->type = new_vehicle_type_index;
						new_car->speed_in_kmh = random_f32(new_vehicle_type.speed_range_min, new_vehicle_type.speed_range_max);
						assert(building_type.vehicle_parks.size());
						v3 car_pos{ new_building->object->position +
							(v3)(glm::rotate(m4(1.0f), new_building->object->rotation.z, v3{ 0.0f, 0.0f, 1.0f }) *
								glm::rotate(m4(1.0f), new_building->object->rotation.y, v3{ 0.0f, 1.0f, 0.0f }) *
								glm::rotate(m4(1.0f), new_building->object->rotation.x, v3{ 1.0f, 0.0f, 0.0f }) *
								v4(building_type.vehicle_parks[0].offset, 1.0f)) };
						v3 car_rotation{ new_building->object->rotation };
						car_rotation.z += glm::radians(building_type.vehicle_parks[0].rotation_in_degrees);
						new_car->object->SetTransform(car_pos, car_rotation);
						new_car->object->enabled = true;
						new_person->car = new_car;
						car_manager.m_Cars.push_back(new_car);
					}

					person_manager.m_People.push_back(new_person);
					bool will_steal{ random_f32(1.0f) < 0.05f };
					if (will_steal)
					{
						new_person->profession = Profession::Thief;
					}
					else
					{
						Building* work{ getAvailableWorkBuilding() };
						if (work)
						{
							work->people.push_back(new_person);
							new_person->work = work;
							auto& work_building_type{ building_types[work->type] };
							switch (work_building_type.group)
							{
							case Building_Group::House:
							case Building_Group::Residential:
								assert(false, "Wrong Building_Group for work");
								break;
							case Building_Group::Commercial:
								new_person->profession = Profession::General_Commercial_Worker;
								break;
							case Building_Group::Industrial:
								new_person->profession = Profession::General_Industrial_Worker;
								break;
							case Building_Group::Office:
								new_person->profession = Profession::General_Office_Worker;
								break;
							case Building_Group::Hospital:
								new_person->profession = Profession::Doctor;
								break;
							case Building_Group::Police_Station:
								new_person->profession = Profession::Policeman;
								break;
							case Building_Group::Garbage_Collection_Center:
								new_person->profession = Profession::Waste_Management_Worker;
								break;
							default:
								assert(false, "Unimplemented Building_Group!");
								break;
							}
						}
					}
				}
				break;
			}
			case Building_Group::Commercial:
			{
				buildings_commercial.push_back(new_building);
				u16 worker{ random_u16(0, building_type.capacity) };
				for (u64 i{ 0 }; i < worker; ++i)
				{
					Person* p{ person_manager.get_unemployed_person() };
					if (p)
					{
						p->work = new_building;
						p->profession = Profession::General_Commercial_Worker;
						new_building->people.push_back(p);
					}
					else
					{
						break;
					}
				}

				u8 work_vehicle_count{ random_u8(4, 6) };
				for (u64 i{ 0 }; i < work_vehicle_count; ++i)
				{
					u64 commercial_vehicle_index{ random_u64(commercial_vehicles.size()) };
					u64 new_vehicle_type_index{ commercial_vehicles[commercial_vehicle_index] };
					const Vehicle_Type& new_vehicle_type{ vehicle_types[new_vehicle_type_index] };
					Car* new_car{ new Car() };
					new_car->object = new Object(new_vehicle_type.prefab);
					new_car->type = new_vehicle_type_index;
					new_car->speed_in_kmh = random_f32(new_vehicle_type.speed_range_min, new_vehicle_type.speed_range_max);
					assert(building_type.vehicle_parks.size());
					v3 car_pos{ new_building->object->position +
						(v3)(glm::rotate(m4(1.0f), new_building->object->rotation.z, v3{ 0.0f, 0.0f, 1.0f }) *
							glm::rotate(m4(1.0f), new_building->object->rotation.y, v3{ 0.0f, 1.0f, 0.0f }) *
							glm::rotate(m4(1.0f), new_building->object->rotation.x, v3{ 1.0f, 0.0f, 0.0f }) *
							v4(building_type.vehicle_parks[0].offset, 1.0f)) };
					v3 car_rotation{ new_building->object->rotation };
					car_rotation.z += glm::radians(building_type.vehicle_parks[0].rotation_in_degrees);
					new_car->object->SetTransform(car_pos, car_rotation);
					new_car->object->enabled = true;
					new_car->building = new_building;
					new_building->vehicles.push_back(new_car);
					car_manager.m_Cars.push_back(new_car);
				}
				break;
			}
			case Building_Group::Industrial:
			{
				buildings_industrial.push_back(new_building);
				u16 worker{ random_u16(0, building_type.capacity) };
				for (u64 i{ 0 }; i < worker; ++i)
				{
					Person* p{ person_manager.get_unemployed_person() };
					if (p)
					{
						p->work = new_building;
						p->profession = Profession::General_Industrial_Worker;
						new_building->people.push_back(p);
					}
					else
					{
						break;
					}
				}

				u8 work_vehicle_count{ random_u8(4, 6) };
				for (u64 i{ 0 }; i < work_vehicle_count; ++i)
				{
					u64 industrial_vehicle_index{ random_u64(industrial_vehicles.size()) };
					u64 new_vehicle_type_index{ industrial_vehicles[industrial_vehicle_index] };
					const Vehicle_Type& new_vehicle_type{ vehicle_types[new_vehicle_type_index] };
					Car* new_car{ new Car() };
					new_car->object = new Object(new_vehicle_type.prefab);
					new_car->type = new_vehicle_type_index;
					new_car->speed_in_kmh = random_f32(new_vehicle_type.speed_range_min, new_vehicle_type.speed_range_max);
					assert(building_type.vehicle_parks.size());
					v3 car_pos{ new_building->object->position +
						(v3)(glm::rotate(m4(1.0f), new_building->object->rotation.z, v3{ 0.0f, 0.0f, 1.0f }) *
							glm::rotate(m4(1.0f), new_building->object->rotation.y, v3{ 0.0f, 1.0f, 0.0f }) *
							glm::rotate(m4(1.0f), new_building->object->rotation.x, v3{ 1.0f, 0.0f, 0.0f }) *
							v4(building_type.vehicle_parks[0].offset, 1.0f)) };
					v3 car_rotation{ new_building->object->rotation };
					car_rotation.z += glm::radians(building_type.vehicle_parks[0].rotation_in_degrees);
					new_car->object->SetTransform(car_pos, car_rotation);
					new_car->object->enabled = true;
					new_car->building = new_building;
					new_building->vehicles.push_back(new_car);
					car_manager.m_Cars.push_back(new_car);
				}
				break;
			}
			case Building_Group::Office:
			{
				buildings_office.push_back(new_building);
				u16 worker{ random_u16(0, building_type.capacity) };
				for (u64 i{ 0 }; i < worker; ++i)
				{
					Person* p{ person_manager.get_unemployed_person() };
					if (p)
					{
						p->work = new_building;
						p->profession = Profession::General_Office_Worker;
						new_building->people.push_back(p);
					}
					else
					{
						break;
					}
				}
				break;
			}
			case Building_Group::Hospital:
			{
				buildings_specials.push_back(new_building);
				u16 worker{ random_u16(0, building_type.capacity) };
				for (u64 i{ 0 }; i < worker; ++i)
				{
					Person* p{ person_manager.get_unemployed_person() };
					if (p)
					{
						p->work = new_building;
						p->profession = Profession::Doctor;
						new_building->people.push_back(p);
					}
					else
					{
						break;
					}
				}

				u8 work_vehicle_count{ random_u8(4, 6) };
				for (u64 i{ 0 }; i < work_vehicle_count; ++i)
				{
					u64 ambulance_index{ random_u64(ambulances.size()) };
					u64 new_vehicle_type_index{ ambulances[ambulance_index] };
					const Vehicle_Type& new_vehicle_type{ vehicle_types[new_vehicle_type_index] };
					Car* new_car{ new Car() };
					new_car->object = new Object(new_vehicle_type.prefab);
					new_car->type = new_vehicle_type_index;
					new_car->speed_in_kmh = random_f32(new_vehicle_type.speed_range_min, new_vehicle_type.speed_range_max);
					assert(building_type.vehicle_parks.size());
					v3 car_pos{ new_building->object->position +
						(v3)(glm::rotate(m4(1.0f), new_building->object->rotation.z, v3{ 0.0f, 0.0f, 1.0f }) *
							glm::rotate(m4(1.0f), new_building->object->rotation.y, v3{ 0.0f, 1.0f, 0.0f }) *
							glm::rotate(m4(1.0f), new_building->object->rotation.x, v3{ 1.0f, 0.0f, 0.0f }) *
							v4(building_type.vehicle_parks[0].offset, 1.0f)) };
					v3 car_rotation{ new_building->object->rotation };
					car_rotation.z += glm::radians(building_type.vehicle_parks[0].rotation_in_degrees);
					new_car->object->SetTransform(car_pos, car_rotation);
					new_car->object->enabled = true;
					new_car->building = new_building;
					new_building->vehicles.push_back(new_car);
					car_manager.m_Cars.push_back(new_car);
				}
				break;
			}
			case Building_Group::Police_Station:
			{
				buildings_specials.push_back(new_building);
				u16 worker{ random_u16(0, building_type.capacity) };
				for (u64 i{ 0 }; i < worker; ++i)
				{
					Person* p{ person_manager.get_unemployed_person() };
					if (p)
					{
						p->work = new_building;
						p->profession = Profession::Policeman;
						new_building->people.push_back(p);
					}
					else
					{
						break;
					}
				}

				u8 work_vehicle_count{ random_u8(4, 6) };
				for (u64 i{ 0 }; i < work_vehicle_count; ++i)
				{
					u64 police_car_index{ random_u64(police_cars.size()) };
					u64 new_vehicle_type_index{ police_cars[police_car_index] };
					const Vehicle_Type& new_vehicle_type{ vehicle_types[new_vehicle_type_index] };
					Car* new_car{ new Car() };
					new_car->object = new Object(new_vehicle_type.prefab);
					new_car->type = new_vehicle_type_index;
					new_car->speed_in_kmh = random_f32(new_vehicle_type.speed_range_min, new_vehicle_type.speed_range_max);
					assert(building_type.vehicle_parks.size());
					v3 car_pos{ new_building->object->position +
						(v3)(glm::rotate(m4(1.0f), new_building->object->rotation.z, v3{ 0.0f, 0.0f, 1.0f }) *
							glm::rotate(m4(1.0f), new_building->object->rotation.y, v3{ 0.0f, 1.0f, 0.0f }) *
							glm::rotate(m4(1.0f), new_building->object->rotation.x, v3{ 1.0f, 0.0f, 0.0f }) *
							v4(building_type.vehicle_parks[0].offset, 1.0f)) };
					v3 car_rotation{ new_building->object->rotation };
					car_rotation.z += glm::radians(building_type.vehicle_parks[0].rotation_in_degrees);
					new_car->object->SetTransform(car_pos, car_rotation);
					new_car->object->enabled = true;
					new_car->building = new_building;
					new_building->vehicles.push_back(new_car);
					car_manager.m_Cars.push_back(new_car);
				}
				break;
			}
			case Building_Group::Garbage_Collection_Center:
			{
				buildings_specials.push_back(new_building);
				u16 worker{ random_u16(0, building_type.capacity) };
				for (u64 i{ 0 }; i < worker; ++i)
				{
					Person* p{ person_manager.get_unemployed_person() };
					if (p)
					{
						p->work = new_building;
						p->profession = Profession::Waste_Management_Worker;
						new_building->people.push_back(p);
					}
					else
					{
						break;
					}
				}

				u8 work_vehicle_count{ random_u8(4, 6) };
				for (u64 i{ 0 }; i < work_vehicle_count; ++i)
				{
					u64 garbage_truck_index{ random_u64(garbage_trucks.size()) };
					u64 new_vehicle_type_index{ garbage_trucks[garbage_truck_index] };
					const Vehicle_Type& new_vehicle_type{ vehicle_types[new_vehicle_type_index] };
					Car* new_car{ new Car() };
					new_car->object = new Object(new_vehicle_type.prefab);
					new_car->type = new_vehicle_type_index;
					new_car->speed_in_kmh = random_f32(new_vehicle_type.speed_range_min, new_vehicle_type.speed_range_max);
					assert(building_type.vehicle_parks.size());
					v3 car_pos{ new_building->object->position +
						(v3)(glm::rotate(m4(1.0f), new_building->object->rotation.z, v3{ 0.0f, 0.0f, 1.0f }) *
							glm::rotate(m4(1.0f), new_building->object->rotation.y, v3{ 0.0f, 1.0f, 0.0f }) *
							glm::rotate(m4(1.0f), new_building->object->rotation.x, v3{ 1.0f, 0.0f, 0.0f }) *
							v4(building_type.vehicle_parks[0].offset, 1.0f)) };
					v3 car_rotation{ new_building->object->rotation };
					car_rotation.z += glm::radians(building_type.vehicle_parks[0].rotation_in_degrees);
					new_car->object->SetTransform(car_pos, car_rotation);
					new_car->object->enabled = true;
					new_car->building = new_building;
					new_building->vehicles.push_back(new_car);
					car_manager.m_Cars.push_back(new_car);
				}
				break;
			}
			default:
				assert(false, "Unimplemented Building_Group!");
				break;
			}
			ResetStates();
			m_Guideline->enabled = true;


			if (restrictions[0] && tree_manager.restrictions[0])
			{
				v2 buildingL = (v2)new_building->object->prefab->boundingBoxL;
				v2 buildingM = (v2)new_building->object->prefab->boundingBoxM;
				v2 buildingP = (v2)new_building->object->position;

				for (size_t i = 0; i < trees.size(); i++)
				{
					Object* tree = trees[i]->object;
					v2 mtv = Helper::check_rotated_rectangle_collision(
						buildingL,
						buildingM,
						new_building->object->rotation.z,
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
			remove_building(*m_SelectedBuildingToDestruct);
		ResetStates();
		return false;
	}

	void BuildingManager::SetType(size_t type_index)
	{
		building_type_index = type_index;
		delete m_Guideline;
		m_Guideline = new Object(m_Scene->MainApplication->building_types[type_index].prefab);
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
		snapped_t_index = 0;
		snapped_t = -1.0f;

		m_SelectedBuildingToDestruct = m_Buildings.end();

		for (Building* building : m_Buildings)
			building->object->tintColor = v4(1.0f);

		m_Guideline->enabled = false;
		m_Guideline->tintColor = v4(1.0f);
	}
	Building* BuildingManager::getAvailableWorkBuilding()
	{
		auto& building_types{ m_Scene->MainApplication->building_types };
		for (Building* b : buildings_commercial)
		{
			auto& building_type{ building_types[b->type] };
			if (building_type.capacity > b->people.size())
				return b;
		}
		for (Building* b : buildings_specials)
		{
			auto& building_type{ building_types[b->type] };
			if (building_type.capacity > b->people.size())
				return b;
		}
		return nullptr;
	}

	Building* BuildingManager::get_building_to_steal_from(const std::vector<Building*>& ignored_buildings)
	{
		auto& building_types{ m_Scene->MainApplication->building_types };
		for (Building*& building : m_Buildings)
		{
			auto& building_type{ building_types[building->type] };
			if (building_type.group == Building_Group::Police_Station) continue;

			if (building->people.size()) continue;
			if (building->since_last_theft > 0.0f && building->since_last_theft < 60.0f) continue;

			bool ignore = false;
			for (const Building* ignored_building : ignored_buildings)
			{
				if (ignored_building == building)
				{
					ignore = true;
					break;
				}
			}
			if (ignore) continue;
			return building;
		}
		return nullptr;
	}

	void remove_building(Building* b)
	{
		GameScene* game{ GameScene::ActiveGameScene };
		auto& ui{ game->ui_layer };
		auto& road_segments{ game->m_RoadManager.road_segments };
		auto& building_types{ game->MainApplication->building_types };

		auto& buildings{ game->m_BuildingManager.m_Buildings };
		auto& buildings_houses{ game->m_BuildingManager.buildings_houses };
		auto& buildings_residential{ game->m_BuildingManager.buildings_residential };
		auto& buildings_commercial{ game->m_BuildingManager.buildings_commercial };
		auto& buildings_industrial{ game->m_BuildingManager.buildings_industrial };
		auto& buildings_office{ game->m_BuildingManager.buildings_office };
		auto& buildings_specials{ game->m_BuildingManager.buildings_specials };

		const auto& people_on_the_road{ game->m_PersonManager.get_people_on_the_road() };
		const auto& building_type{ building_types[b->type] };

		while (b->people.size() > 0)
			remove_person(b->people[0]);

		for (Person* p : people_on_the_road)
		{
			if (p->path_end_building == b)
			{
				if (p->car_driving)
				{
					reset_car_back_to_building_from(p->car_driving);
				}
				else
				{
					reset_person_back_to_building_from(p);
				}
			}
		}

		if (b->connected_road_segment != -1)
		{
			auto& connected_buildings{ road_segments[b->connected_road_segment].buildings };
			auto it{ std::find(connected_buildings.begin(), connected_buildings.end(), b) };
			assert(it != connected_buildings.end());
			connected_buildings.erase(it);
		}

		auto it{ std::find(buildings.begin(), buildings.end(), b) };
		assert(it != buildings.end());
		buildings.erase(it);

		switch (building_type.group)
		{
		case Building_Group::House:
		{
			auto home_it{ std::find(buildings_houses.begin(), buildings_houses.end(), b) };
			assert(it != buildings_houses.end());
			buildings_houses.erase(home_it);
			break;
		}
		case Building_Group::Residential:
		{
			auto home_it{ std::find(buildings_residential.begin(), buildings_residential.end(), b) };
			assert(it != buildings_residential.end());
			buildings_residential.erase(home_it);
			break;
		}
		case Building_Group::Commercial:
		{
			auto home_it{ std::find(buildings_commercial.begin(), buildings_commercial.end(), b) };
			assert(it != buildings_commercial.end());
			buildings_commercial.erase(home_it);
			break;
		}
		case Building_Group::Industrial:
		{
			auto home_it{ std::find(buildings_industrial.begin(), buildings_industrial.end(), b) };
			assert(it != buildings_industrial.end());
			buildings_industrial.erase(home_it);
			break;
		}
		case Building_Group::Office:
		{
			auto home_it{ std::find(buildings_office.begin(), buildings_office.end(), b) };
			assert(it != buildings_office.end());
			buildings_office.erase(home_it);
			break;
		}
		case Building_Group::Hospital:
		case Building_Group::Police_Station:
		case Building_Group::Garbage_Collection_Center:
		{
			auto home_it{ std::find(buildings_specials.begin(), buildings_specials.end(), b) };
			assert(it != buildings_specials.end());
			buildings_specials.erase(home_it);
			break;
		}
		default:
			assert(false, "Unimplemented Building_Group");
			break;
		}

		while (b->vehicles.size() > 0)
			remove_car(b->vehicles[0]);

		if (ui.selected_building == b)
			ui.selected_building = nullptr;

		delete b;
	}

	void update_buildings(TimeStep ts)
	{
		auto& buildings{ GameScene::ActiveGameScene->m_BuildingManager.m_Buildings };
		auto& building_types{ GameScene::ActiveGameScene->MainApplication->building_types };
		constexpr f32 MAGIC_HEALTH_NUMBER{ 0.2f };
		for (auto& building : buildings)
		{
			const Building_Type& building_type{ building_types[building->type] };
			building->since_last_garbage_pick_up += ts;
			if (building_type.group != Building_Group::Hospital)
			{
				const f32 health_ratio{ building->current_health / building->max_health };
				const f32 garbage_ratio{ building->current_garbage / building->garbage_capacity };
				const f32 garbage_space_left{ std::max(0.0f, 1.0f - garbage_ratio) };
				const f32 ratio_diff{ health_ratio - garbage_ratio };
				building->current_health -= ts * ratio_diff * MAGIC_HEALTH_NUMBER;
			}
		}
	}
}