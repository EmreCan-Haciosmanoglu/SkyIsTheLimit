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
	void update_car(Car* const c, Person* const vehicle_operator, TimeStep ts)
	{
		GameApp* app{ GameScene::ActiveGameScene->MainApplication };
		auto& road_segments{ GameScene::ActiveGameScene->m_RoadManager.road_segments };
		const auto& vehicle_types{ app->vehicle_types };
		const auto& road_types{ app->road_types };

		const Vehicle_Type& type{ vehicle_types[c->type] };
		f32 lenght{ type.object_length };

		v3 journey_left_vector{ vehicle_operator->target - c->object->position };
		f32 journey_left{ glm::length(journey_left_vector) };
		v3 journey_direction{ journey_left_vector / journey_left };

		f32 speed_in_meter_per_second{ (c->speed_in_kmh * 0.1f) * (1.0f / 3.6f) };

		f32 movement_length_in_next_frame{ ts * speed_in_meter_per_second };

		u64 path_count{ vehicle_operator->path.size() };
		assert(path_count > 0);

		RS_Transition_For_Vehicle* transition{ (RS_Transition_For_Vehicle*)c->path[0] };
		RoadSegment& current_road_segment{ road_segments[transition->road_segment_index] };
		const RoadType& current_road_type{ road_types[current_road_segment.type] };

		if (c->heading_to_a_parking_spot == false)
		{
			bool don_t_ingore_this_person{ true };
			v3 car_s_position_in_the_next_frame{ 0.0f };
			f32 car_s_rotation_in_the_next_frame{ c->object->rotation.z };
			if (movement_length_in_next_frame < journey_left)
			{
				car_s_position_in_the_next_frame = c->object->position + journey_direction * movement_length_in_next_frame;
			}
			else
			{
				car_s_position_in_the_next_frame = vehicle_operator->target;
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
						auto next_transition{ (RS_Transition_For_Vehicle*)vehicle_operator->path[1] };
						next_target = next_transition->points_stack.back();
					}
					else
					{
						don_t_ingore_this_person = false;
					}
				}
				v2 direction{ glm::normalize((v2)(next_target - car_s_position_in_the_next_frame)) };
				car_s_rotation_in_the_next_frame = glm::acos(direction.x) * ((f32)(direction.y > 0.0f) * 2.0f - 1.0f) + glm::radians(180.0f);
			}

			if (don_t_ingore_this_person)
			{
				auto& people_on_the_road{ current_road_segment.people };
				u64 count{ people_on_the_road.size() };
				c->path_is_blocked = false;
				for (u64 in{ 0 }; in < count; ++in)
				{
					Car* car_on_the_road{ people_on_the_road[in]->car_driving };

					if (car_on_the_road == nullptr) continue;
					if (car_on_the_road == c) continue;

					RS_Transition_For_Vehicle* other_transition{ (RS_Transition_For_Vehicle*)car_on_the_road->path[0] };
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

					}
				}
			}
		}

		if (movement_length_in_next_frame < journey_left)
		{
			c->object->SetTransform(c->object->position + journey_direction * movement_length_in_next_frame);
		}
		else
		{
			if (c->heading_to_a_parking_spot)
			{
				vehicle_operator->heading_to_a_building = true;
				c->heading_to_a_parking_spot = false;

				c->object->SetTransform(
					c->target,
					glm::rotateZ(
						vehicle_operator->path_end_building->object->rotation,
						glm::radians(vehicle_operator->path_end_building->car_park.rotation_in_degrees)
					)
				);
				vehicle_operator->position = c->object->position;
				vehicle_operator->object->SetTransform(vehicle_operator->position);
				vehicle_operator->object->enabled = true;
				vehicle_operator->status = PersonStatus::Walking;
				if (vehicle_operator->car_driving != vehicle_operator->car) // find if car driven is work car
				{
					vehicle_operator->path_end_building->vehicles.push_back(vehicle_operator->car_driving);
					vehicle_operator->car_driving = nullptr;
					vehicle_operator->drove_in_work = true;
				}
				set_person_target(vehicle_operator, vehicle_operator->path_end_building->position);
			}
			else
			{
				c->object->SetTransform(c->target);
				//p->position = p->target; // all people in the car
				u64 points_count = transition->points_stack.size();
				if (points_count > 0)
				{
					v3 target = transition->points_stack[points_count - 1];
					transition->points_stack.pop_back();
					set_target_and_car_direction(vehicle_operator, c, target);
				}
				else
				{
					if (c->path.size() == 1)
					{
						// cache target car park position in car
						//v3 car_park_pos = p->path_end_building->position +
						//	(v3)(glm::rotate(m4(1.0f), p->path_end_building->object->rotation.z, v3{ 0.0f, 0.0f, 1.0f }) *
						//		glm::rotate(m4(1.0f), p->path_end_building->object->rotation.y, v3{ 0.0f, 1.0f, 0.0f }) *
						//		glm::rotate(m4(1.0f), p->path_end_building->object->rotation.x, v3{ 1.0f, 0.0f, 0.0f }) *
						//		v4(p->path_end_building->car_park.offset, 1.0f));
						set_target_and_car_direction(vehicle_operator, c, c->target_park_pos);
						c->heading_to_a_parking_spot = true;
					}
					else
					{/*Next Path*/
						s64 next_road_node_index = transition->next_road_node_index;

						c->path.erase(c->path.begin());
						delete transition;
						transition = (RS_Transition_For_Vehicle*)c->path[0];

						auto& vehicles_on_the_road = road_segments[c->road_segment].vehicles;
						auto it = std::find(vehicles_on_the_road.begin(), vehicles_on_the_road.end(), c);
						assert(it != vehicles_on_the_road.end());
						vehicles_on_the_road.erase(it);

						RoadSegment& next_road_segment = road_segments[transition->road_segment_index];
						c->road_segment = transition->road_segment_index;
						next_road_segment.vehicles.push_back(c);

						v3 target{ transition->points_stack[transition->points_stack.size() - 1] };
						set_target_and_car_direction(vehicle_operator, c, target);
					}
				}
			}
		}
	}
}