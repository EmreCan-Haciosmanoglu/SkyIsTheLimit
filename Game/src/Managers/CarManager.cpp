#include "canpch.h"
#include "CarManager.h"
#include "Scenes/GameScene.h"
#include "Types/RoadSegment.h"
#include "Types/Road_Type.h"
#include "Types/Vehicle_Type.h"
#include "Types/RoadNode.h"
#include "Types/Transition.h"
#include "Building.h"
#include "GameApp.h"
#include "Helper.h"

#include "Can/Math.h"

namespace Can
{
	/*Anom*/ namespace
	{
		std::vector<Car*> get_driven_cars_with_no_path()
		{
			auto& cars{ GameScene::ActiveGameScene->m_CarManager.m_Cars };
			std::vector<Car*> result{};

			for (auto car : cars)
				if (car->driver && car->path.size() == 0)
					result.push_back(car);

			return result;
		}

		std::vector<std::vector<Car*>> get_ordered_driven_cars()
		{
			auto& road_segments{ GameScene::ActiveGameScene->m_RoadManager.road_segments };
			const auto& road_types{ GameScene::ActiveGameScene->MainApplication->road_types };
			std::vector<std::vector<Car*>> result{};

			for (u64 i = 0; i < road_segments.capacity; ++i) {
				if (road_segments.values[i].valid == false)
					continue;
				auto& road_segment{ road_segments[i] };
				auto& road_type{ road_types[road_segment.type] };

				std::vector<std::vector<Car*>> cars{};
				cars.resize(road_type.lanes_backward.size() + road_type.lanes_forward.size());

				for (auto car : road_segment.vehicles)
				{
					assert(car->path.size());
					cars[car->path[0]->lane_index].push_back(car);
				}
				for (auto& cs : cars)
				{
					if (cs.size())
						result.push_back(cs);
				}
			}
			return result;
		}

	}

	CarManager::CarManager(GameScene* scene)
		: m_Scene(scene) {}

	void set_car_target_and_direction(Car* car, const v3& target)
	{
		car->target = target;
		v2 direction{ glm::normalize((v2)car->target - (v2)car->object->position) };
		f32 yaw{ glm::acos(direction.x) * ((float)(direction.y > 0.0f) * 2.0f - 1.0f) };
		car->object->SetTransform(car->object->position, v3{ 0.0f, 0.0f, yaw + glm::radians(180.0f) });
	}
	std::vector<Car*> CarManager::get_cars_on_the_road()
	{
		std::vector<Car*> result{};

		for (Car* c : m_Cars)
			if (c->driver)
				result.push_back(c);

		return result;
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

	void update_cars(TimeStep ts)
	{
		GameApp* app{ GameScene::ActiveGameScene->MainApplication };
		auto& road_segments{ GameScene::ActiveGameScene->m_RoadManager.road_segments };
		auto& road_nodes{ GameScene::ActiveGameScene->m_RoadManager.road_nodes };
		const auto& building_types{ app->building_types };
		const auto& vehicle_types{ app->vehicle_types };
		const auto& road_types{ app->road_types };

		auto ordered_driven_cars{ get_ordered_driven_cars() };
		for (auto& cars : ordered_driven_cars) // Cars in same lane
		{
			std::sort(cars.begin(), cars.end(), Helper::sort_by_left_journey());
			for (u64 car_index = 0; car_index < cars.size(); ++car_index)
			{
				Car* car{ cars[car_index] };

				u64 path_count{ car->path.size() };
				assert(path_count > 0);
				RS_Transition_For_Vehicle* transition{ car->path[0] };

				v3 journey_left_vector{ car->target - car->object->position };
				f32 journey_left{ glm::length(journey_left_vector) };
				v3 journey_direction{ journey_left_vector / journey_left };
				f32 speed_in_meter_per_second{ (car->speed_in_kmh * 0.1f) * (1.0f / 3.6f) };
				f32 movement_length_in_next_frame{ ts * speed_in_meter_per_second };


				bool path_is_blocked{ false };
				if (car_index > 0)
				{
					v2 boundingL{ (v2)car->object->prefab->boundingBoxL };
					v2 boundingM{ (v2)car->object->prefab->boundingBoxM };
					f32 car_s_rotation_in_the_next_frame{ car->object->rotation.z };
					v3 car_s_position_in_the_next_frame{ 0.0f };

					if (car->t < 1.0f)
					{
						f32 new_t{ car->t + ts * 0.66f };
						car_s_position_in_the_next_frame = Math::QuadraticCurve(car->driftpoints, car->t);
					}
					else
					{
						if (movement_length_in_next_frame < journey_left)
						{
							car_s_position_in_the_next_frame = car->object->position + journey_direction * movement_length_in_next_frame;
						}
						else
						{
							car_s_position_in_the_next_frame = car->target;
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
									auto next_transition{ car->path[1] };
									next_target = next_transition->points_stack.back();
								}
							}
							v2 direction{ glm::normalize((v2)(next_target - car_s_position_in_the_next_frame)) };
							car_s_rotation_in_the_next_frame = glm::acos(direction.x) * ((f32)(direction.y > 0.0f) * 2.0f - 1.0f) + glm::radians(180.0f);
						}
					}

					Car* other_car{ cars[car_index - 1] };

					v2 bL{ (v2)other_car->object->prefab->boundingBoxL };
					v2 bM{ (v2)other_car->object->prefab->boundingBoxM };
					f32 other_car_s_rotation{ other_car->object->rotation.z };
					v3 other_car_s_position{ other_car->object->position };

					v2 mtv = Helper::check_rotated_rectangle_collision(
						bL,
						bM,
						other_car_s_rotation,
						other_car_s_position,
						boundingL,
						boundingM,
						car_s_rotation_in_the_next_frame,
						car_s_position_in_the_next_frame
					);

					if (glm::length(mtv) > 0.0f)
						path_is_blocked = true;
				}

				if (!path_is_blocked)
				{
					const Vehicle_Type& vehicle_type{ vehicle_types[car->type] };
					f32 lenght{ vehicle_type.object_length };

					RoadSegment& current_road_segment{ road_segments[transition->road_segment_index] };
					const Road_Type& current_road_type{ road_types[current_road_segment.type] };

					if (car->t < 1.0f)
					{
						car->t += ts.GetSeconds() * 0.66f;
						glm::vec3 driftPos{ Math::QuadraticCurve(car->driftpoints, car->t) };
						set_car_target_and_direction(car, driftPos);
						car->object->SetTransform(driftPos);
						transition->left_journey = glm::length(car->target - driftPos);
					}
					else
					{
						switch (vehicle_type.type)
						{
						case Car_Type::Personal:
						case Car_Type::Commercial:
						case Car_Type::Industrial:
						case Car_Type::Ambulance:
						case Car_Type::Garbage_Truck:
						{
							// Garbage_Truck -> collect trash while moving
							break;
						}
						case Car_Type::Police_Car:
						{
							Person* const& officer{ car->driver };
							if (officer->status == PersonStatus::Patrolling)
							{
								for (const Building* const& building : current_road_segment.buildings)
								{
									v3 dist_vector = building->object->position - car->object->position;
									f32 dist2 = glm::length2(dist_vector);

									constexpr f32 MAGICAL_DISTANCE = 5.0f;
									if (dist2 > MAGICAL_DISTANCE) continue;

									for (Person* person : building->people)
									{
										if (person->profession != Profession::Thief) continue;
										if (person->status != PersonStatus::AtWork) continue;

										// TODO: go to house instead then catch???


										// TODO: And make some other person Thief???
										reset_person(person);
										person->profession = Profession::Unemployed;
										person->status = PersonStatus::IsPassenger;
										car->passengers.push_back(person);
										if (car->passengers.size() >= vehicle_type.passenger_limit)
										{
											officer->status = PersonStatus::DrivingForWork;

											assert(officer->work);
											officer->path_end_building = officer->work;

											car->path = Helper::get_path_for_a_car(building, officer->work);

											if (car->path.size() == 0) // if no path available
											{
												// reset to police station
												assert(false, "TODO");
											}
											break;
										}
									}
									if (car->passengers.size() >= vehicle_type.passenger_limit)
										break;
								}
							}
							break;
						}
						default:
							assert(false, "Unimplemented Car_Type");
							break;
						}

						if (movement_length_in_next_frame < journey_left)
						{
							car->object->SetTransform(car->object->position + journey_direction * movement_length_in_next_frame);
							transition->left_journey = glm::length(car->target - car->object->position);
						}
						else
						{
							car->object->SetTransform(car->target);
							u64 points_count = transition->points_stack.size();
							if (points_count > 0)
							{
								v3 target{ transition->points_stack[points_count - 1] };
								transition->points_stack.pop_back();
								set_car_target_and_direction(car, target);
								transition->left_journey = glm::length(car->target - car->object->position);
							}
							else
							{
								if (car->path.size() == 1)
								{
									// TODO: cache target car park position in car
									auto building{ car->driver->path_end_building };
									auto& building_type{ building_types[building->type] };
									switch (vehicle_type.type)
									{
									case Car_Type::Personal:
									case Car_Type::Commercial:
									case Car_Type::Industrial:
									{
										assert(building_type.vehicle_parks.size());
										v3 car_park_pos{ building->object->position +
											(v3)(glm::rotate(m4(1.0f), building->object->rotation.z, v3{ 0.0f, 0.0f, 1.0f }) *
												glm::rotate(m4(1.0f), building->object->rotation.y, v3{ 0.0f, 1.0f, 0.0f }) *
												glm::rotate(m4(1.0f), building->object->rotation.x, v3{ 1.0f, 0.0f, 0.0f }) *
												v4(building_type.vehicle_parks[0].offset, 1.0f)) };
										set_car_target_and_direction(car, car_park_pos);
										car->heading_to_a_parking_spot = true;
										break;
									}
									case Car_Type::Ambulance:
									case Car_Type::Police_Car:
									case Car_Type::Garbage_Truck:
									{
										if (car->driver->path_end_building != car->driver->path_start_building)
										{
											// Ambulance     : Heal patients
											// Police_Car    : Catch thieves
											// Garbage_Truck : Collecting garbage
											v3 car_park_pos{ building->object->position +
												(v3)(glm::rotate(m4(1.0f), building->object->rotation.z, v3{ 0.0f, 0.0f, 1.0f }) *
													glm::rotate(m4(1.0f), building->object->rotation.y, v3{ 0.0f, 1.0f, 0.0f }) *
													glm::rotate(m4(1.0f), building->object->rotation.x, v3{ 1.0f, 0.0f, 0.0f }) *
													v4(building_type.visiting_spot, 1.0f)) };
											set_car_target_and_direction(car, car_park_pos);
											car->heading_to_a_visiting_spot = true;
										}
										else
										{
											// Returning to building
											assert(building_type.vehicle_parks.size());
											v3 car_park_pos{ building->object->position +
												(v3)(glm::rotate(m4(1.0f), building->object->rotation.z, v3{ 0.0f, 0.0f, 1.0f }) *
													glm::rotate(m4(1.0f), building->object->rotation.y, v3{ 0.0f, 1.0f, 0.0f }) *
													glm::rotate(m4(1.0f), building->object->rotation.x, v3{ 1.0f, 0.0f, 0.0f }) *
													v4(building_type.vehicle_parks[0].offset, 1.0f)) };
											set_car_target_and_direction(car, car_park_pos);
											car->heading_to_a_parking_spot = true;
										}
										break;
									}
									default:
										assert(false, "Unimplemented Car_Type!");
										break;
									}

									delete car->path[0];
									car->path.pop_back();
									car->road_segment = -1;

									auto res{ remove_car_from(current_road_segment, car) };
									assert(res);
								}
								else
								{/*Next Path*/
									s64 next_road_node_index{ transition->next_road_node_index };
									RoadNode& next_road_node{ road_nodes[next_road_node_index] };

									car->path.erase(car->path.begin());
									delete transition;
									transition = car->path[0];

									auto res{ remove_car_from(current_road_segment, car) };
									assert(res);

									RoadSegment& next_road_segment{ road_segments[transition->road_segment_index] };
									car->road_segment = transition->road_segment_index;
									next_road_segment.vehicles.push_back(car);

									v3 target{ transition->points_stack[transition->points_stack.size() - 1] };
									car->target = target;

									transition->left_journey = glm::length(target - car->object->position);

									car->driftpoints[0] = car->object->position;
									car->driftpoints[1] = next_road_node.position;
									car->driftpoints[2] = target;
									car->t = 0.0f;
								}
							}
						}
					}
				}
			}
		}

		auto driven_cars_with_no_path{ get_driven_cars_with_no_path() };
		for (auto car : driven_cars_with_no_path)
		{
			const Vehicle_Type& vehicle_type{ vehicle_types[car->type] };
			v3 journey_left_vector{ car->target - car->object->position };
			f32 journey_left{ glm::length(journey_left_vector) };
			v3 journey_direction{ journey_left_vector / journey_left };
			f32 speed_in_meter_per_second{ (car->speed_in_kmh * 0.1f) * (1.0f / 3.6f) };
			f32 movement_length_in_next_frame{ ts * speed_in_meter_per_second };

			if (movement_length_in_next_frame < journey_left)
			{
				car->object->SetTransform(car->object->position + journey_direction * movement_length_in_next_frame);
			}
			else
			{
				if (car->heading_to_a_visiting_spot == true)
				{
					auto building{ car->driver->path_end_building };
					switch (vehicle_type.type)
					{
					case Car_Type::Personal:
					case Car_Type::Commercial:
					case Car_Type::Industrial:
					{
						//Do Nothing
						break;
					}
					case Car_Type::Ambulance:
					{
						// Heal less sick people
						// Bring worse ones to hospital
						Person* doctor{ car->driver };

						for (Person* person : building->people)
						{
							f32 person_health_gone{ 1.0f - person->health };
							if (person_health_gone > 0.75f)
							{
								if (car->passengers.size() < vehicle_type.passenger_limit)
								{
									reset_person(person);
									person->status = PersonStatus::IsPassenger;
									car->passengers.push_back(person);
								}
							}
							else
							{
								if (person_health_gone < car->cargo)
								{
									person->health = 1.0f;
									car->cargo -= person_health_gone;
								}
								else
								{
									person->health += car->cargo;
									car->cargo = 0.0f;
									break;
								}
							}
						}

						doctor->status = PersonStatus::DrivingForWork;
						assert(doctor->work);
						doctor->path_end_building = doctor->work;

						car->path = Helper::get_path_for_a_car(building, doctor->work);
						if (car->path.size() == 0) // if no path available
						{
							// reset to hospital
							assert(false, "TODO");
						}
						break;
					}
					case Car_Type::Police_Car:
					{
						// Catch the thief if still at the house
						// Else return to police station
						// Or continue to patrolling somehow???
						Person* const& officer{ car->driver };

						for (Person* person : building->people)
						{
							if (person->profession != Profession::Thief) continue;
							if (person->status != PersonStatus::AtWork) continue;

							// TODO: And make some other person Thief???
							reset_person(person);
							person->profession = Profession::Unemployed;
							person->status = PersonStatus::IsPassenger;
							car->passengers.push_back(person);
							if (car->passengers.size() >= vehicle_type.passenger_limit)
							{
								officer->status = PersonStatus::DrivingForWork;

								assert(officer->work);
								officer->path_end_building = officer->work;

								car->path = Helper::get_path_for_a_car(building, officer->work);

								if (car->path.size() == 0) // if no path available
								{
									// reset to police station
									assert(false, "TODO");
								}
								break;
							}
						}
						break;
					}
					case Car_Type::Garbage_Truck:
					{
						f32 capacity_left{ vehicle_type.cargo_limit - car->cargo };
						if (building->current_garbage < capacity_left)
						{
							car->cargo += building->current_garbage;
							building->current_garbage = 0.0f;
							car->path = Helper::get_path_for_gargabe_vehicle(building, car->driver->path_end_building);
							if (car->path.size() == 0) // No Building to take garbage from
							{
								car->driver->path_end_building = car->driver->work;
								car->path = Helper::get_path_for_a_car(building, car->driver->work);
							}
						}
						else
						{
							car->cargo = vehicle_type.cargo_limit;
							building->current_garbage -= capacity_left;
							car->driver->path_end_building = car->driver->work;
							car->path = Helper::get_path_for_a_car(building, car->driver->work);
						}
						building->is_garbage_truck_on_the_way = false;
						building->since_last_garbage_pick_up = 0.0f;

						car->heading_to_a_visiting_spot = false;
						if (car->path.size())
						{
							car->road_segment = building->connected_road_segment;
							road_segments[building->connected_road_segment].vehicles.push_back(car);
						}
						else
						{
							reset_car_back_to_building_from(car);
						}
						break;
					}
					default:
						assert(false, "Unimplemented Car_Type");
						break;
					}
				}
				else if(car->heading_to_a_parking_spot)
				{
					auto driver = car->driver;
					car->driver = nullptr;
					driver->car_driving = nullptr;

					auto building = driver->path_end_building;
					auto& building_type{ building_types[building->type] };
					assert(building_type.vehicle_parks.size());
					car->heading_to_a_parking_spot = false;
					car->object->SetTransform(
						car->target,
						glm::rotateZ(building->object->rotation, glm::radians(building_type.vehicle_parks[0].rotation_in_degrees))
					);

					driver->heading_to_a_building = true;
					driver->position = car->object->position;
					driver->object->SetTransform(driver->position);
					driver->object->enabled = true;
					driver->status = PersonStatus::Walking;
					if (car != driver->car) // find if car driven is work car
					{
						building->vehicles.push_back(car);
						driver->drove_in_work = true;
					}
					set_person_target(driver, building->object->position);

					switch (vehicle_type.type)
					{
					case Car_Type::Personal:
					case Car_Type::Commercial:
					case Car_Type::Industrial:
					{
						break;
					}
					case Car_Type::Ambulance:
					{
						// TODO: move sick people inside
						// What if Hospital is full???
						// Extra people will just wait to open space
						while (car->passengers.size())
						{
							Person* patient{ car->passengers.back() };
							building->visitors.push_back(patient);
							car->passengers.pop_back();

							patient->status = PersonStatus::InHospital;
							patient->position = building->object->position;
							patient->object->SetTransform(patient->position);
							patient->hospital = building; // TODO: building_in
						}
						break;
					}
					case Car_Type::Police_Car:
					{
						// TODO: move arrested people inside
						// What if Police Station is full???
						while (car->passengers.size())
						{
							Person* thief{ car->passengers.back() };
							building->visitors.push_back(thief);
							car->passengers.pop_back();

							thief->status = PersonStatus::InJail;
							thief->time_left = random_f32(1.0f, 2.0f);
							thief->position = building->object->position;
							thief->object->SetTransform(thief->position);
							thief->work = building; // TODO: building_in
						}
						break;
					}
					case Car_Type::Garbage_Truck:
					{
						// TODO: do something more then just this
						car->cargo = 0.0f;
						break;
					}
					default:
						assert(false, "Unimplemented Car_Type");
						break;
					}

				}
				else
				{
					assert(false, "Imposible branch");
				}
			}
		}
	}
}
