#include "canpch.h"
#include "CarManager.h"
#include "Scenes/GameScene.h"
#include "Types/RoadSegment.h"
#include "Types/Transition.h"
#include "Building.h"
#include "GameApp.h"
#include "Helper.h"

#include "Can/Math.h"

namespace Can
{
	CarManager::CarManager(GameScene* scene)
		: m_Scene(scene)
	{
	}
	void remove_car(Car* car)
	{
		auto& cars = GameScene::ActiveGameScene->m_CarManager.m_Cars;

		auto it = std::find(cars.begin(), cars.end(), car);
		assert(it != cars.end());
		cars.erase(it);

		if (car->building)
		{
			auto& vehicles = car->building->vehicles;
			auto it = std::find(vehicles.begin(), vehicles.end(), car);
			assert(it != vehicles.end());
			vehicles.erase(it);
		}

		delete car;
	}
	void update_car(Car* const car_driven, TimeStep ts)
	{
		GameApp* app{ GameScene::ActiveGameScene->MainApplication };
		auto& road_segments{ GameScene::ActiveGameScene->m_RoadManager.road_segments };
		const auto& vehicle_types{ app->vehicle_types };
		const auto& road_types{ app->road_types };

		const Vehicle_Type& type{ vehicle_types[car_driven->type] };
		f32 lenght{ type.object_length };

		v3 journey_left_vector{ car_driven->target - car_driven->object->position };
		f32 journey_left{ glm::length(journey_left_vector) };
		v3 journey_direction{ journey_left_vector / journey_left };

		f32 speed_in_meter_per_second{ (car_driven->speed_in_kmh * 0.1f) * (1.0f / 3.6f) };

		f32 movement_length_in_next_frame{ ts * speed_in_meter_per_second };

		if (movement_length_in_next_frame < journey_left)
		{
			car_driven->object->SetTransform(car_driven->object->position + journey_direction * movement_length_in_next_frame);
		}
		else
		{
			if (car_driven->heading_to_a_parking_spot)
			{
				auto driver = car_driven->driver;
				car_driven->driver = nullptr;
				driver->car_driving = nullptr;

				auto building = driver->path_end_building;
				car_driven->heading_to_a_parking_spot = false;
				car_driven->object->SetTransform(
					car_driven->target,
					glm::rotateZ(building->object->rotation, glm::radians(building->car_park.rotation_in_degrees))
				);

				driver->heading_to_a_building = true;
				driver->position = car_driven->object->position;
				driver->object->SetTransform(driver->position);
				driver->object->enabled = true;
				driver->status = PersonStatus::Walking;
				if (car_driven != driver->car) // find if car driven is work car
				{
					building->vehicles.push_back(car_driven);
					driver->drove_in_work = true;
				}
				set_person_target(driver, building->position);
			}
			else
			{
				u64 path_count{ car_driven->path.size() };
				assert(path_count > 0);

				RS_Transition_For_Vehicle* transition{ car_driven->path[0] };

				RoadSegment& current_road_segment{ road_segments[transition->road_segment_index] };
				const RoadType& current_road_type{ road_types[current_road_segment.type] };

				if (car_driven->heading_to_a_parking_spot == false)
				{
					bool don_t_ingore_this_car{ true };
					v3 car_s_position_in_the_next_frame{ 0.0f };
					f32 car_s_rotation_in_the_next_frame{ car_driven->object->rotation.z };
					if (movement_length_in_next_frame < journey_left)
					{
						car_s_position_in_the_next_frame = car_driven->object->position + journey_direction * movement_length_in_next_frame;
					}
					else
					{
						car_s_position_in_the_next_frame = car_driven->target;
						u64 points_count{ transition->points_stack.size() };
						v3 next_target{ 0.0f };
						if (points_count > 0)
						{
							next_target = transition->points_stack.back();
						}
						else
						{
							if (path_count > 1)
							{
								auto next_transition{ car_driven->path[1] };
								next_target = next_transition->points_stack.back();
							}
							else
							{
								don_t_ingore_this_car = false;
							}
						}
						v2 direction{ glm::normalize((v2)(next_target - car_s_position_in_the_next_frame)) };
						car_s_rotation_in_the_next_frame = glm::acos(direction.x) * ((f32)(direction.y > 0.0f) * 2.0f - 1.0f) + glm::radians(180.0f);
					}

					if (don_t_ingore_this_car)
					{
						auto& vehicles_on_the_road{ current_road_segment.vehicles };
						u64 count{ vehicles_on_the_road.size() };
						car_driven->path_is_blocked = false;
						for (u64 in{ 0 }; in < count; ++in)
						{
							Car* car_on_the_road{ vehicles_on_the_road[in] };

							if (car_on_the_road == car_driven) continue;

							RS_Transition_For_Vehicle* other_transition{ car_on_the_road->path[0] };
							if (other_transition->lane_index != transition->lane_index) continue;
							if (other_transition->points_stack.size() > transition->points_stack.size()) continue;


							v3 other_journey_left_vector{ car_on_the_road->target - car_on_the_road->object->position };
							f32 other_journey_left{ glm::length(other_journey_left_vector) };
							if (other_transition->points_stack.size() == transition->points_stack.size())
							{
								if (other_journey_left > journey_left) continue;
							}

							v3 other_car_s_position_in_the_next_frame{ 0.0f };
							f32 other_car_s_rotation_in_the_next_frame{ 0.0f };
							if (car_on_the_road->path_is_blocked)
							{
								other_car_s_position_in_the_next_frame = car_on_the_road->object->position;
								other_car_s_rotation_in_the_next_frame = car_on_the_road->object->rotation.z;
							}
							else
							{
								// TODO: ???
							}
						}
					}
				}

				car_driven->object->SetTransform(car_driven->target);
				//p->position = p->target; // all people in the car
				u64 points_count = transition->points_stack.size();
				if (points_count > 0)
				{
					v3 target = transition->points_stack[points_count - 1];
					transition->points_stack.pop_back();
					set_car_target_and_direction(car_driven, target);
				}
				else
				{
					if (car_driven->path.size() == 1)
					{
						// TODO: cache target car park position in car
						auto building = car_driven->driver->path_end_building;
						v3 car_park_pos = building->position +
							(v3)(glm::rotate(m4(1.0f), building->object->rotation.z, v3{ 0.0f, 0.0f, 1.0f }) *
								glm::rotate(m4(1.0f), building->object->rotation.y, v3{ 0.0f, 1.0f, 0.0f }) *
								glm::rotate(m4(1.0f), building->object->rotation.x, v3{ 1.0f, 0.0f, 0.0f }) *
								v4(building->car_park.offset, 1.0f));
						set_car_target_and_direction(car_driven, car_park_pos);
						car_driven->heading_to_a_parking_spot = true;

						delete car_driven->path[0];
						car_driven->path.pop_back();
						car_driven->road_segment = -1;

						auto res = remove_car_from(current_road_segment, car_driven);
						assert(res);
					}
					else
					{/*Next Path*/
						s64 next_road_node_index = transition->next_road_node_index;

						car_driven->path.erase(car_driven->path.begin());
						delete transition;
						transition = car_driven->path[0];

						auto res = remove_car_from(current_road_segment, car_driven);
						assert(res);

						RoadSegment& next_road_segment{ road_segments[transition->road_segment_index] };
						car_driven->road_segment = transition->road_segment_index;
						next_road_segment.vehicles.push_back(car_driven);

						v3 target{ transition->points_stack[transition->points_stack.size() - 1] };
						set_car_target_and_direction(car_driven, target);
					}
				}
			}
		}
	}
}