#include "canpch.h"
#include "PersonManager.h"
#include "Scenes/GameScene.h"
#include "Types/RoadSegment.h"
#include "Types/Road_Type.h"
#include "Types/RoadNode.h"
#include "Types/Transition.h"
#include "Types/Person.h"
#include "Building.h"
#include "Can/Math.h"

#include "GameApp.h"
#include "Helper.h"

namespace Can
{
	PersonManager::PersonManager(GameScene* scene)
		: m_Scene(scene)
	{
	}

	void PersonManager::Update(TimeStep ts)
	{
		GameApp* app{ m_Scene->MainApplication };
		auto& bm{ m_Scene->m_BuildingManager };
		auto& road_segments{ m_Scene->m_RoadManager.road_segments };
		auto& road_nodes{ m_Scene->m_RoadManager.road_nodes };
		auto& road_types{ m_Scene->MainApplication->road_types };
		auto& building_types{ m_Scene->MainApplication->building_types };
		constexpr f32 MAGIC_HEALTH_NUMBER{ 0.05f };
		constexpr f32 MAGIC_HEALING_NUMBER{ 0.5f };

		for (size_t person_index = 0; person_index < m_People.size(); person_index++)
		{
			Person* p{ m_People[person_index] };

			switch (p->status)
			{
			case PersonStatus::AtHome:
			case PersonStatus::AtWork:
			case PersonStatus::InJail:
			{
				Building* const building{ (p->status == PersonStatus::AtHome) ? p->home : p->work };
				assert(building);
				// building currently in
				// more educated less garbage
				// different amount according to age
				// if (building_types[p->work->type].group != Building_Group::Garbage_Collection_Center) we don't care tbh.
				building->current_garbage += 0.1f * ts;
				const f32 health_ratio{ building->current_health / building->max_health };
				const f32 health_missing{ 1.0f - health_ratio };
				p->health -= ts * health_missing * MAGIC_HEALTH_NUMBER;

				p->health = std::max(p->health, 0.2f); // TODO: Become sick
				break;
			}
			case PersonStatus::WalkingDead:
			case PersonStatus::Walking:
			case PersonStatus::Driving:
			case PersonStatus::DrivingForWork:
			case PersonStatus::IsPassenger:
			case PersonStatus::Patrolling:
			case PersonStatus::Responding:
			case PersonStatus::InHospital:
			{
				// Don't do anything
				break;
			}
			default:
				assert(false, "Unimplemented PersonStatus");
				break;
			}

			//	if (p->health < 0.0f)
			//	{
			//		// TODO: Die
			//		assert(false, "DIE");
			//	}

			switch (p->status)
			{
			case PersonStatus::AtHome:
			{
				if (p->health < 0.25f) // You are almost sick 
				{
					if (p->home->is_ambulance_on_the_way) break;
					// Call an ambulance
					Helper::find_and_assign_an_ambulance(p->home);
					break;
				}
				p->time_left -= ts;

				if (p->time_left <= 0.0f)
				{
					p->path_start_building = p->home;

					switch (p->profession)
					{
					case Profession::Unemployed:
					{
						// just walk around then come back to home
						p->path = Helper::get_path(p->home, 5);
						p->path_end_building = p->home;
						break;
					}
					case Profession::General_Commercial_Worker:
					case Profession::General_Industrial_Worker:
					case Profession::General_Office_Worker:
					case Profession::Doctor:
					case Profession::Policeman:
					case Profession::Waste_Management_Worker:
					{
						assert(p->work);
						p->path_end_building = p->work;

						if (p->car)
						{
							p->car_driving = p->car;
							p->car_driving->driver = p;
							p->car_driving->path = Helper::get_path_for_a_car(p->home, p->work);
						}
						else
						{
							p->path = Helper::get_path(p->home, p->work);
						}

						if (p->path.size() == 0 && p->car_driving && p->car_driving->path.size() == 0) // if no path available
						{
							// just walk around then come back to home
							p->path = Helper::get_path(p->home, 5);
							p->path_end_building = p->home;
							if (p->car_driving)
							{
								p->car_driving->driver = nullptr;
								p->car_driving = nullptr;
							}
						}
						break;
					}
					case Profession::Thief:
					{
						std::vector<Building*> ignored_buildings{ p->home };
						if (p->car)
						{
							p->car_driving = p->car;
							p->car_driving->driver = p;
							while (true)
							{
								p->work = bm.get_building_to_steal_from(ignored_buildings);
								if (p->work == nullptr)
									break;
								p->car_driving->path = Helper::get_path_for_a_car(p->home, p->work);
								if (p->car_driving->path.size()) break;
								ignored_buildings.push_back(p->work);
							}
						}
						else
						{
							while (true)
							{
								p->work = bm.get_building_to_steal_from(ignored_buildings);
								if (p->work == nullptr)
									break;
								p->path = Helper::get_path(p->home, p->work);
								if (p->path.size()) break;
								ignored_buildings.push_back(p->work);
							}
						}
						p->path_end_building = p->work;
						if (p->work == nullptr)// if no building to steal from
						{
							// just walk around then come back to home
							p->path = Helper::get_path(p->home, 5);
							p->path_end_building = p->home;
							if (p->car_driving)
							{
								p->car_driving->driver = nullptr;
								p->car_driving = nullptr;
							}
						}
						break;
					}
					default:
						assert(false, "Unimplemented Profession");
						break;
					}

					// TODO: Combine these two line into a function
					p->position = p->home->object->position;
					p->object->SetTransform(p->position);

					p->object->enabled = true;
					p->heading_to_a_building = false;
					p->heading_to_a_car = false;
					p->status = PersonStatus::Walking;

					if (p->car_driving)
					{
						p->heading_to_a_car = true;
						set_person_target(p, p->car_driving->object->position);
					}
					else
					{
						p->road_segment = p->home->connected_road_segment;
						RoadSegment& road_segment = road_segments[p->road_segment];
						road_segment.people.push_back(p);

						// TODO: Refactor this scope into a function
						RS_Transition_For_Walking* rs_transition = (RS_Transition_For_Walking*)p->path[0];
						Road_Type& road_segment_type = road_types[road_segment.type];

						assert(p->home->snapped_t_index < (s64)road_segment.curve_samples.size() - 1);
						v3 target_position = road_segment.curve_samples[p->home->snapped_t_index];
						assert(road_segment.curve_samples.size() - 1 >= p->home->snapped_t_index + 1);
						v3 target_position_plus_one = road_segment.curve_samples[p->home->snapped_t_index + 1];

						v3 dir = target_position_plus_one - target_position;
						v3 offsetted = target_position + dir * p->home->snapped_t;

						v3 sidewalk_position_offset = glm::normalize(v3{ dir.y, -dir.x, 0.0f });
						if (p->home->snapped_to_right)
							sidewalk_position_offset *= road_segment_type.lanes_forward[road_segment_type.lanes_forward.size() - 1].distance_from_center;
						else
							sidewalk_position_offset *= road_segment_type.lanes_backward[0].distance_from_center;

						((RS_Transition_For_Walking*)p->path[0])->at_path_array_index = p->home->snapped_t_index;

						set_person_target(p, offsetted + sidewalk_position_offset);
					}
				}
				break;
			}
			case PersonStatus::AtWork:
			{
				p->time_left -= ts;

				if (p->time_left <= 0.0f)
				{
					switch (p->profession)
					{
					case Profession::Unemployed:
					{
						assert(false, "Impossible Profession");
						break;
					}
					case Profession::General_Commercial_Worker:
					case Profession::General_Industrial_Worker:
					case Profession::General_Office_Worker:
					case Profession::Doctor:
					case Profession::Policeman:
					case Profession::Waste_Management_Worker:
					{
						if (
							p->drove_in_work ||

							/* just go home after times up */
							(p->profession == Profession::Doctor) ||
							(p->profession == Profession::General_Office_Worker)
							)
						{
							p->drove_in_work = false;
							if (p->car)
							{
								p->car_driving = p->car;
								p->car_driving->driver = p;
								p->car_driving->path = Helper::get_path_for_a_car(p->work, p->home);
							}
							else
							{
								p->path = Helper::get_path(p->work, p->home);
							}
							p->path_end_building = p->home;
						}
						else
						{
							Car* work_car = retrive_work_vehicle(p->work);
							if (work_car)
							{
								p->car_driving = work_car;
								p->car_driving->driver = p;
								p->path_end_building = p->work;

								if (p->profession == Profession::Waste_Management_Worker)
								{
									// TODO v2: go some buildings and comeback e.g. Ambulance, Police, Fire, Delivery
									Building* to = nullptr;
									auto path = Helper::get_path_for_gargabe_vehicle(p->work, to);
									if (to)
									{
										p->car_driving->path = path;
										p->path_end_building = to;
									}
									else
									{
										// TODO: Instead, just go home, no reason to roam around with garbage truck
										p->car_driving->path = Helper::get_path_for_a_car(p->work, 5);
									}
								}
								else if (p->profession == Profession::Policeman)
								{
									// TODO v2: Patrol intelligently not randomly
									p->car_driving->path = Helper::get_path_for_a_car(p->work, 5);
								}
								else
								{
									p->car_driving->path = Helper::get_path_for_a_car(p->work, 5);
								}
							}
							else
							{
								if (p->car)
								{
									p->car_driving = p->car;
									p->car_driving->driver = p;
									p->car_driving->path = Helper::get_path_for_a_car(p->work, p->home);
								}
								else
								{
									p->path = Helper::get_path(p->work, p->home);
								}
								p->path_end_building = p->home;
							}
						}
						break;
					}
					case Profession::Thief:
					{
						p->work->crime_reported++;

						if (p->car)
						{
							p->car_driving = p->car;
							p->car_driving->driver = p;
							p->car_driving->path = Helper::get_path_for_a_car(p->work, p->home);
						}
						else
						{
							p->path = Helper::get_path(p->work, p->home);
						}
						p->path_end_building = p->home;
						break;
					}
					default:
						assert(false, "Unimplemented Profession");
						break;
					}

					if (p->path.size() == 0 && p->car_driving && p->car_driving->path.size() == 0) // if no path available
					{
						// path to home is cut out / destroyed
						reset_person_back_to_home(p);
						continue;
					}

					// TODO: Combine these two line into a function
					p->position = p->work->object->position;
					p->object->SetTransform(p->position);

					p->path_start_building = p->work;

					p->object->enabled = true;
					p->heading_to_a_building = false;
					p->heading_to_a_car = false;
					p->status = PersonStatus::Walking;

					if (p->car_driving)
					{
						p->heading_to_a_car = true;
						set_person_target(p, p->car_driving->object->position);
					}
					else
					{
						p->road_segment = p->work->connected_road_segment;
						RoadSegment& road_segment = road_segments[p->road_segment];
						road_segment.people.push_back(p);

						// TODO: Refactor this scope into a function
						RS_Transition_For_Walking* rs_transition = (RS_Transition_For_Walking*)p->path[0];
						Road_Type& road_segment_type = road_types[road_segment.type];

						assert(p->work->snapped_t_index < (s64)road_segment.curve_samples.size() - 1);
						v3 target_position = road_segment.curve_samples[p->work->snapped_t_index];
						assert(road_segment.curve_samples.size() - 1 >= p->work->snapped_t_index + 1);
						v3 target_position_plus_one = road_segment.curve_samples[p->work->snapped_t_index + 1];

						v3 dir = target_position_plus_one - target_position;
						v3 offsetted = target_position + dir * p->work->snapped_t;

						v3 sidewalk_position_offset = glm::normalize(v3{ dir.y, -dir.x, 0.0f });
						if (p->work->snapped_to_right)
							sidewalk_position_offset *= road_segment_type.lanes_forward[road_segment_type.lanes_forward.size() - 1].distance_from_center;
						else
							sidewalk_position_offset *= road_segment_type.lanes_backward[0].distance_from_center;

						((RS_Transition_For_Walking*)p->path[0])->at_path_array_index = p->work->snapped_t_index;

						set_person_target(p, offsetted + sidewalk_position_offset);
					}
				}
				break;
			}
			case PersonStatus::Walking:
			{
				v3 left = p->target - p->position;
				f32 left_length = glm::length(left);
				f32 converted_speed = (p->speed_in_kmh / 10.f) / 3.6f;
				f32 journey_length = ts * converted_speed;
				if (journey_length < left_length)
				{
					v3 unit = left / left_length;
					p->position = p->position + unit * journey_length;
					p->object->SetTransform(p->position);
				}
				else
				{
					p->position = p->target;
					p->object->SetTransform(p->position);

					if (p->road_node != -1)
					{
						auto& road_node = road_nodes[p->road_node];
						auto& connected_road_segments = road_node.roadSegments;

						RN_Transition_For_Walking* rn_transition = (RN_Transition_For_Walking*)p->path[0];
						RS_Transition_For_Walking* rs_transition = (RS_Transition_For_Walking*)p->path[1];

						f32 half_crosswalk_width = 0.05f;

						RoadSegment& current_road_segment = road_segments[connected_road_segments[rn_transition->from_road_segments_array_index]];
						Road_Type& current_road_segment_type = road_types[current_road_segment.type];
						f32 current_road_segment_width = current_road_segment_type.road_width * 0.5f;
						f32 offset_from_current_road_segment_center = current_road_segment_width - half_crosswalk_width;

						v3 current_road_segment_position = p->road_node == current_road_segment.StartNode ? current_road_segment.GetStartPosition() : current_road_segment.GetEndPosition();
						v3 current_road_segment_direction = p->road_node == current_road_segment.StartNode ? current_road_segment.GetStartDirection() : current_road_segment.GetEndDirection();
						v3 rotated_current_road_segment_direction = glm::normalize(v3{ current_road_segment_direction.y, -current_road_segment_direction.x, 0.0f });
						std::array<v3, 4> ps{};
						ps[1] = current_road_segment_position + rotated_current_road_segment_direction * offset_from_current_road_segment_center;
						ps[2] = current_road_segment_position - rotated_current_road_segment_direction * offset_from_current_road_segment_center;

						if (rn_transition->from_road_segments_array_index == rn_transition->to_road_segments_array_index)
						{
							if (((rn_transition->sub_index == 1) && rs_transition->from_right) ||
								((rn_transition->sub_index == 2) && !rs_transition->from_right))
							{
								auto path = p->path[0];
								p->path.erase(p->path.begin());
								u64 at_index = p->road_node == current_road_segment.StartNode ? 0 : current_road_segment.curve_samples.size() - 1;

								((RS_Transition_For_Walking*)p->path[0])->at_path_array_index = at_index;
								assert(remove_person_from(road_node, p));

								p->road_segment = connected_road_segments[rn_transition->to_road_segments_array_index];
								current_road_segment.people.push_back(p);
								p->road_node = -1;
								delete path;
								continue;
							}

							if (rn_transition->accending)
								rn_transition->sub_index++;
							else
								rn_transition->sub_index--;

							if (rn_transition->sub_index == 2)
								set_person_target(p, ps[2]);
							else if (rn_transition->sub_index == 1)
								set_person_target(p, ps[1]);
							else
								assert(false);
						}
						else
						{
							if (rn_transition->accending)
								rn_transition->sub_index++;
							else
								rn_transition->sub_index--;

							RoadSegment& prev_road_segment = road_segments[connected_road_segments[(rn_transition->from_road_segments_array_index - 1 + connected_road_segments.size()) % connected_road_segments.size()]];
							RoadSegment& next_road_segment = road_segments[connected_road_segments[(rn_transition->from_road_segments_array_index + 1) % connected_road_segments.size()]];

							Road_Type& prev_road_segment_type = road_types[prev_road_segment.type];
							Road_Type& next_road_segment_type = road_types[next_road_segment.type];

							f32 prev_road_segment_width = prev_road_segment_type.road_width * 0.5f;
							f32 next_road_segment_width = next_road_segment_type.road_width * 0.5f;

							f32 offset_from_prev_road_segment_center = prev_road_segment_width - half_crosswalk_width;
							f32 offset_from_next_road_segment_center = next_road_segment_width - half_crosswalk_width;

							v3 prev_road_segment_position = p->road_node == prev_road_segment.StartNode ? prev_road_segment.GetStartPosition() : prev_road_segment.GetEndPosition();
							v3 next_road_segment_position = p->road_node == next_road_segment.StartNode ? next_road_segment.GetStartPosition() : next_road_segment.GetEndPosition();

							v3 prev_road_segment_direction = p->road_node == prev_road_segment.StartNode ? prev_road_segment.GetStartDirection() : prev_road_segment.GetEndDirection();
							v3 next_road_segment_direction = p->road_node == next_road_segment.StartNode ? next_road_segment.GetStartDirection() : next_road_segment.GetEndDirection();

							v3 rotated_prev_direction = glm::normalize(v3{ prev_road_segment_direction.y, -prev_road_segment_direction.x, 0.0f });
							v3 rotated_next_direction = glm::normalize(v3{ next_road_segment_direction.y, -next_road_segment_direction.x, 0.0f });
							rotated_prev_direction *= -1.0f;

							ps[0] = Math::safe_ray_plane_intersection(
								prev_road_segment_position + rotated_prev_direction * offset_from_prev_road_segment_center,
								prev_road_segment_direction,
								ps[1],
								rotated_current_road_segment_direction
							);
							ps[3] = Math::safe_ray_plane_intersection(
								next_road_segment_position + rotated_next_direction * offset_from_next_road_segment_center,
								next_road_segment_direction,
								ps[2],
								rotated_current_road_segment_direction
							);

							set_person_target(p, ps[rn_transition->sub_index]);

							if (rn_transition->sub_index == 0)
							{
								rn_transition->sub_index = 3;
								rn_transition->from_road_segments_array_index--;
							}
							else if (rn_transition->sub_index == 3)
							{
								rn_transition->sub_index = 0;
								rn_transition->from_road_segments_array_index++;
							}
							rn_transition->from_road_segments_array_index += connected_road_segments.size();
							rn_transition->from_road_segments_array_index = rn_transition->from_road_segments_array_index % connected_road_segments.size();
						}
					}
					else if (p->heading_to_a_building)
					{
						if (p->path_end_building == p->home)
						{
							p->status = PersonStatus::AtHome;
							// Check for thiefs
							if (p->home->is_police_on_the_way == false)
							{
								for (const Person* const& person_in_the_building : p->home->people)
								{
									if (person_in_the_building->home == p->home) continue;
									//Call the cops
									Helper::find_and_assign_a_policeman(p->home);
								}
							}
						}
						else
							p->status = PersonStatus::AtWork;
						p->time_left = random_f32(1.0f, 5.0f);
						p->object->enabled = false;
						p->heading_to_a_building = false;
						p->road_segment = -1;
					}
					else if (p->heading_to_a_car)
					{
						RS_Transition_For_Vehicle* rs_transition{ p->car_driving->path[0] };
						RoadSegment& road_segment{ road_segments[rs_transition->road_segment_index] };
						Road_Type& road_segment_type{ road_types[road_segment.type] };
						v3 target_position{ rs_transition->points_stack[rs_transition->points_stack.size() - 1] };

						if (p->car != p->car_driving) // car_driven is work car
						{
							if (p->profession == Profession::Policeman)
							{
								if (p->path_end_building == p->path_start_building)
									p->status = PersonStatus::Patrolling;
								else
									p->status = PersonStatus::Responding;
							}
							else if (p->profession == Profession::Doctor)
							{
								p->status = PersonStatus::Responding;
							}
							else
							{
								p->status = PersonStatus::DrivingForWork;
							}
						}
						else
						{
							p->status = PersonStatus::Driving;
						}
						p->position = p->car_driving->object->position;

						set_car_target_and_direction(p->car_driving, target_position);

						p->heading_to_a_car = false;
						p->object->enabled = false;

						p->car_driving->road_segment = rs_transition->road_segment_index;
						road_segment.vehicles.push_back(p->car_driving);
					}
					else if (p->road_segment != -1)
					{
						RS_Transition_For_Walking* rs_transition{ (RS_Transition_For_Walking*)p->path[0] };
						RoadSegment& road_segment{ road_segments[p->road_segment] };
						Road_Type& road_segment_type{ road_types[road_segment.type] };

						u64 target_path_array_index;
						if (p->path.size() == 1)
							target_path_array_index = p->path_end_building->snapped_t_index;
						else if (rs_transition->from_start)
							target_path_array_index = road_segment.curve_samples.size() - 1;
						else
							target_path_array_index = 0;

						if ((rs_transition->from_start && (rs_transition->at_path_array_index >= target_path_array_index)) ||
							(!rs_transition->from_start && (rs_transition->at_path_array_index <= target_path_array_index)))
						{
							if (p->path.size() == 1)
							{
								// We are at the building
								p->target = p->path_end_building->object->position;
								p->heading_to_a_building = true;

								RoadSegment& segment{ road_segments[p->road_segment] };
								auto res{ remove_person_from(segment, p) };
								assert(res);

								delete p->path[0];
								p->path.pop_back();
							}
							else
							{
								// We are at the end of the road
								// So move to RoadNode
								p->road_node = rs_transition->from_start ? road_segment.EndNode : road_segment.StartNode;

								RoadNode& road_node{ road_nodes[p->road_node] };
								road_node.people.push_back(p);
								auto res{ remove_person_from(road_segment, p) };
								assert(res);
								p->road_segment = -1;

								auto path{ p->path[0] };
								p->path.erase(p->path.begin());
								delete path;
							}
							continue;
						}

						v3 p2{};
						v3 dir2{};
						if (rs_transition->from_start)
						{
							u64 next_index{ std::min(rs_transition->at_path_array_index + 1, road_segment.curve_samples.size() - 1) };
							p2 = road_segment.curve_samples[next_index];
							if (next_index < road_segment.curve_samples.size() - 1)
							{
								v3 p3{ road_segment.curve_samples[next_index + 1] };
								dir2 = glm::normalize(p3 - p2);
							}
							else
							{
								dir2 = road_segment.GetEndDirection() * -1.0f;
							}
							rs_transition->at_path_array_index = next_index;
						}
						else
						{
							u64 next_index{ std::min(rs_transition->at_path_array_index - 1, road_segment.curve_samples.size() - 1) };
							p2 = road_segment.curve_samples[next_index];
							if (next_index > 0)
							{
								v3 p3{ road_segment.curve_samples[next_index - 1] };
								dir2 = glm::normalize(p3 - p2);
							}
							else
							{
								dir2 = road_segment.GetStartDirection() * -1.0f;
							}
							rs_transition->at_path_array_index = next_index;
						}
						v3 offset{ glm::normalize(v3{ dir2.y,-dir2.x, 0.0f }) };
						if (rs_transition->from_right)
							offset *= road_segment_type.lanes_forward[road_segment_type.lanes_forward.size() - 1].distance_from_center;
						else
							offset *= road_segment_type.lanes_backward[0].distance_from_center;

						set_person_target(p, p2 + offset);
					}
					else
					{
						assert(false, "This should be impossible!!!");
					}
				}
				break;
			}
			case PersonStatus::Driving:
			case PersonStatus::DrivingForWork:
			case PersonStatus::Patrolling:
			case PersonStatus::Responding:
				// Handled in CarManager
				break;
			case PersonStatus::IsPassenger:
				// Don't do anything for now
				break;
			case PersonStatus::WalkingDead:
				//  slumpy A*
				break;
			case PersonStatus::InJail:
			{
				p->time_left -= ts;

				if (p->time_left <= 0.0f)
				{
					Building* police_station = p->work;
					p->work = nullptr;
					p->path = Helper::get_path(police_station, p->home);

					p->path_start_building = police_station;
					p->path_end_building = p->home;

					if (p->path.size() == 0) // if no path available
					{
						// path to home is cut out / destroyed
						reset_person_back_to_home(p);
						continue;
					}

					// TODO: Combine these two line into a function
					p->position = police_station->object->position;
					p->object->SetTransform(p->position);
					p->object->enabled = true;

					p->heading_to_a_building = false;
					p->heading_to_a_car = false;
					p->status = PersonStatus::Walking;

					p->road_segment = police_station->connected_road_segment;
					RoadSegment& road_segment = road_segments[p->road_segment];
					road_segment.people.push_back(p);

					// TODO: Refactor this scope into a function
					RS_Transition_For_Walking* rs_transition = (RS_Transition_For_Walking*)p->path[0];
					Road_Type& road_segment_type = road_types[road_segment.type];

					assert(police_station->snapped_t_index < (s64)road_segment.curve_samples.size() - 1);
					v3 target_position = road_segment.curve_samples[police_station->snapped_t_index];
					assert(road_segment.curve_samples.size() - 1 >= police_station->snapped_t_index + 1);
					v3 target_position_plus_one = road_segment.curve_samples[police_station->snapped_t_index + 1];

					v3 dir = target_position_plus_one - target_position;
					v3 offsetted = target_position + dir * police_station->snapped_t;

					v3 sidewalk_position_offset = glm::normalize(v3{ dir.y, -dir.x, 0.0f });
					if (police_station->snapped_to_right)
						sidewalk_position_offset *= road_segment_type.lanes_forward[road_segment_type.lanes_forward.size() - 1].distance_from_center;
					else
						sidewalk_position_offset *= road_segment_type.lanes_backward[0].distance_from_center;

					((RS_Transition_For_Walking*)p->path[0])->at_path_array_index = police_station->snapped_t_index;

					set_person_target(p, offsetted + sidewalk_position_offset);
				}
				break;
			}
			case PersonStatus::InHospital:
			{
				p->health += 0.1f * ts * MAGIC_HEALING_NUMBER;

				if (p->health > 1.0f)
				{
					p->health = 1.0f;

					Building* hospital = p->hospital;
					p->hospital = nullptr;
					p->path = Helper::get_path(hospital, p->home);

					p->path_start_building = hospital;
					p->path_end_building = p->home;

					if (p->path.size() == 0) // if no path available
					{
						// path to home is cut out / destroyed
						reset_person_back_to_home(p);
						continue;
					}

					// TODO: Combine these two line into a function
					p->position = hospital->object->position;
					p->object->SetTransform(p->position);
					p->object->enabled = true;

					p->heading_to_a_building = false;
					p->heading_to_a_car = false;
					p->status = PersonStatus::Walking;

					p->road_segment = hospital->connected_road_segment;
					RoadSegment& road_segment = road_segments[p->road_segment];
					road_segment.people.push_back(p);

					// TODO: Refactor this scope into a function
					RS_Transition_For_Walking* rs_transition = (RS_Transition_For_Walking*)p->path[0];
					Road_Type& road_segment_type = road_types[road_segment.type];

					assert(hospital->snapped_t_index < (s64)road_segment.curve_samples.size() - 1);
					v3 target_position = road_segment.curve_samples[hospital->snapped_t_index];
					assert(road_segment.curve_samples.size() - 1 >= hospital->snapped_t_index + 1);
					v3 target_position_plus_one = road_segment.curve_samples[hospital->snapped_t_index + 1];

					v3 dir = target_position_plus_one - target_position;
					v3 offsetted = target_position + dir * hospital->snapped_t;

					v3 sidewalk_position_offset = glm::normalize(v3{ dir.y, -dir.x, 0.0f });
					if (hospital->snapped_to_right)
						sidewalk_position_offset *= road_segment_type.lanes_forward[road_segment_type.lanes_forward.size() - 1].distance_from_center;
					else
						sidewalk_position_offset *= road_segment_type.lanes_backward[0].distance_from_center;

					((RS_Transition_For_Walking*)p->path[0])->at_path_array_index = hospital->snapped_t_index;

					set_person_target(p, offsetted + sidewalk_position_offset);
				}
				break;
			}
			default:
				assert(false);
				break;
			}
		}
	}

	Person* PersonManager::get_unemployed_person()
	{
		for (Person* p : m_People)
			if (p->profession == Profession::Unemployed)
				return p;

		return nullptr;
	}
	std::vector<Person*> PersonManager::get_people_on_the_road()
	{
		std::vector<Person*> result{};
		for (Person* p : m_People)
		{
			switch (p->status)
			{
			case PersonStatus::AtHome:
			case PersonStatus::AtWork:
				// Do nothing
				break;
			case PersonStatus::Walking:
			case PersonStatus::WalkingDead:
			case PersonStatus::Driving:
			case PersonStatus::DrivingForWork:
			{
				result.push_back(p);
				break;
			}
			default:
				assert(false);
				break;
			}
		}
		return result;
	}

	void reset_person_back_to_building_from(Person* p)
	{
		p->position = p->path_start_building->object->position;
		p->object->SetTransform(p->position);
		if (p->path_start_building == p->home)
		{
			p->status = PersonStatus::AtHome;
			p->time_left = random_f32(1.0f, 2.0f);	// home values
		}
		else if (p->path_start_building == p->work)
		{
			p->status = PersonStatus::AtWork;
			p->time_left = random_f32(1.0f, 2.0f);	// work values
		}
		else
		{
			assert(false, "TODO"); // at some other building (for the future)
		}
		reset_person(p);
	}
	void reset_person(Person* p)
	{
		GameScene* game{ GameScene::ActiveGameScene };
		auto& road_segments{ game->m_RoadManager.road_segments };
		auto& road_nodes{ game->m_RoadManager.road_nodes };
		auto& building_types{ game->MainApplication->building_types };
		Building* start_building{ p->path_start_building };
		auto& building_type{ building_types[start_building->type] };

		p->object->enabled = false;
		if (p->road_segment != -1)
		{
			auto& people_on_the_road_segment{ road_segments[p->road_segment].people };
			auto it{ std::find(people_on_the_road_segment.begin(), people_on_the_road_segment.end(), p) };
			if (it == people_on_the_road_segment.end()) assert(false);

			people_on_the_road_segment.erase(it);
			p->road_segment = -1;
		}
		else if (p->road_node != -1)
		{
			auto& people_on_the_road_node{ road_nodes[p->road_node].people };
			auto it{ std::find(people_on_the_road_node.begin(), people_on_the_road_node.end(), p) };
			if (it == people_on_the_road_node.end()) assert(false);

			people_on_the_road_node.erase(it);
			p->road_node = -1;
		}
		while (p->path.size() > 0)
		{
			delete p->path[p->path.size() - 1];
			p->path.pop_back();
		}
		p->path_end_building = nullptr;
		p->path_start_building = nullptr;
		p->drove_in_work = false;
		p->heading_to_a_building = false;
		p->heading_to_a_car = false;
		p->time_left = 10.0f; // TODO: set according to "something" ???

		if (p->car)
		{
			Building* home{ p->home };
			auto& building_type{ building_types[home->type] };
			v3 car_driving_pos{ home->object->position +
				(v3)(glm::rotate(m4(1.0f), home->object->rotation.z, v3{ 0.0f, 0.0f, 1.0f }) *
					glm::rotate(m4(1.0f), home->object->rotation.y, v3{ 0.0f, 1.0f, 0.0f }) *
					glm::rotate(m4(1.0f), home->object->rotation.x, v3{ 1.0f, 0.0f, 0.0f }) *
					v4(building_type.vehicle_parks[0].offset, 1.0f)) };
			p->car->object->SetTransform(
				car_driving_pos,
				glm::rotateZ(
					home->object->rotation,
					glm::radians(building_type.vehicle_parks[0].rotation_in_degrees)
				)
			);
		}

		if (p->car_driving)
		{
			if (p->car_driving != p->car)
			{
				v3 car_driving_pos{ start_building->object->position +
					(v3)(glm::rotate(m4(1.0f), start_building->object->rotation.z, v3{ 0.0f, 0.0f, 1.0f }) *
						glm::rotate(m4(1.0f), start_building->object->rotation.y, v3{ 0.0f, 1.0f, 0.0f }) *
						glm::rotate(m4(1.0f), start_building->object->rotation.x, v3{ 1.0f, 0.0f, 0.0f }) *
						v4(building_type.vehicle_parks[0].offset, 1.0f)) };
				p->car_driving->object->SetTransform(
					car_driving_pos,
					glm::rotateZ(
						start_building->object->rotation,
						glm::radians(building_type.vehicle_parks[0].rotation_in_degrees)
					)
				);
			}
			p->car_driving->driver = nullptr;
			p->car_driving = nullptr;
		}

	}
	void reset_car_back_to_building_from(Car* c)
	{
		GameScene* game{ GameScene::ActiveGameScene };
		auto& road_segments{ game->m_RoadManager.road_segments };
		auto& building_types{ game->MainApplication->building_types };

		if (c->road_segment != -1)
		{
			auto& cars_on_the_road_segment{ road_segments[c->road_segment].vehicles };
			auto it{ std::find(cars_on_the_road_segment.begin(), cars_on_the_road_segment.end(), c) };
			assert(it != cars_on_the_road_segment.end());
			cars_on_the_road_segment.erase(it);
			c->road_segment = -1;
		}
		Building* building_from{ c->driver->path_start_building };
		auto& building_type{ building_types[building_from->type] };
		v3 car_park_pos{ building_from->object->position +
			(v3)(glm::rotate(m4(1.0f), building_from->object->rotation.z, v3{ 0.0f, 0.0f, 1.0f }) *
				glm::rotate(m4(1.0f), building_from->object->rotation.y, v3{ 0.0f, 1.0f, 0.0f }) *
				glm::rotate(m4(1.0f), building_from->object->rotation.x, v3{ 1.0f, 0.0f, 0.0f }) *
				v4(building_type.vehicle_parks[0].offset, 1.0f)) };
		c->object->SetTransform(
			car_park_pos,
			glm::rotateZ(
				building_from->object->rotation,
				glm::radians(building_type.vehicle_parks[0].rotation_in_degrees)
			)
		);
		while (c->path.size())
		{
			u64 size{ c->path.size() };
			delete c->path[size - 1];
			c->path.pop_back();
		}
		c->t = 1.0f;

		Person* p = c->driver;
		c->driver = nullptr;
		c->cargo = 0.0f;

		p->position = p->path_start_building->object->position;
		p->object->SetTransform(p->position);
		p->object->enabled = false;
		if (p->path_start_building == p->home)
		{
			p->status = PersonStatus::AtHome;
			p->time_left = random_f32(1.0f, 2.0f);	// home values
		}
		else if (p->path_start_building == p->work)
		{
			p->status = PersonStatus::AtWork;
			p->time_left = random_f32(1.0f, 2.0f);	// work values
		}
		else
		{
			assert(false); // at some other building (for the future)
		}
		p->path_end_building = nullptr;
		p->path_start_building = nullptr;

		p->from_right = false;
		p->heading_to_a_building = false;
		p->heading_to_a_car = false;
		p->car_driving = nullptr;
	}
	void reset_person_back_to_home(Person* p)
	{
		GameScene* game = GameScene::ActiveGameScene;
		auto& road_segments = game->m_RoadManager.road_segments;
		auto& road_nodes = game->m_RoadManager.road_nodes;
		auto& building_types{ game->MainApplication->building_types };

		if (p->road_segment != -1)
		{
			auto& people_on_the_road_segment = road_segments[p->road_segment].people;
			auto it = std::find(people_on_the_road_segment.begin(), people_on_the_road_segment.end(), p);
			if (it == people_on_the_road_segment.end()) assert(false);

			people_on_the_road_segment.erase(it);
			p->road_segment = -1;
		}
		else if (p->road_node != -1)
		{
			auto& people_on_the_road_node = road_nodes[p->road_node].people;
			auto it = std::find(people_on_the_road_node.begin(), people_on_the_road_node.end(), p);
			if (it == people_on_the_road_node.end()) assert(false);

			people_on_the_road_node.erase(it);
			p->road_node = -1;
		}
		p->position = p->path_start_building->object->position;
		p->object->SetTransform(p->position);
		p->object->enabled = false;
		p->status = PersonStatus::AtHome;
		p->time_left = random_f32(1.0f, 2.0f);	// home values

		while (p->path.size() > 0)
		{
			delete p->path[p->path.size() - 1];
			p->path.pop_back();
		}

		if (p->car)
		{
			Building* b = p->path_start_building;
			auto& building_type{ building_types[b->type] };
			v3 car_pos = b->object->position +
				(v3)(glm::rotate(m4(1.0f), b->object->rotation.z, v3{ 0.0f, 0.0f, 1.0f }) *
					glm::rotate(m4(1.0f), b->object->rotation.y, v3{ 0.0f, 1.0f, 0.0f }) *
					glm::rotate(m4(1.0f), b->object->rotation.x, v3{ 1.0f, 0.0f, 0.0f }) *
					v4(building_type.vehicle_parks[0].offset, 1.0f));
			p->car->object->SetTransform(
				car_pos,
				glm::rotateZ(
					b->object->rotation,
					glm::radians(building_type.vehicle_parks[0].rotation_in_degrees)
				)
			);
		}

		p->path_end_building = nullptr;
		p->path_start_building = nullptr;

		p->from_right = false;
		p->heading_to_a_building = false;
		p->heading_to_a_car = false;
		p->car_driving = nullptr;

	}
	void remove_person(Person* p)
	{
		GameScene* game = GameScene::ActiveGameScene;
		auto& people = game->m_PersonManager.m_People;
		auto& road_segments = game->m_RoadManager.road_segments;
		auto& road_nodes = game->m_RoadManager.road_nodes;

		if (p->road_segment != -1)
		{
			auto& people_on_the_road_segment = road_segments[p->road_segment].people;
			auto it = std::find(people_on_the_road_segment.begin(), people_on_the_road_segment.end(), p);
			assert(it != people_on_the_road_segment.end());

			people_on_the_road_segment.erase(it);
		}
		else if (p->road_node != -1)
		{
			auto& people_on_the_road_node = road_nodes[p->road_node].people;
			auto it = std::find(people_on_the_road_node.begin(), people_on_the_road_node.end(), p);
			assert(it != people_on_the_road_node.end());

			people_on_the_road_node.erase(it);
		}
		u64 path_count = p->path.size();
		if (path_count > 0)
		{
			while (path_count > 0)
			{
				auto t = p->path[path_count - 1];
				p->path.pop_back();
				delete t;
				path_count--;
			}
		}
		if (p->home)
		{
			auto& residents = p->home->people;
			auto it = std::find(residents.begin(), residents.end(), p);
			assert(it != residents.end());

			residents.erase(it);
		}
		if (p->work)
		{
			auto& workers = p->work->people;
			auto it = std::find(workers.begin(), workers.end(), p);
			assert(it != workers.end());

			workers.erase(it);
		}
		if (p->car)
		{
			remove_car(p->car);
			p->car = nullptr;
		}
		if (p->car_driving)
		{
			// TODO: reset car_driving
		}
		auto it = std::find(people.begin(), people.end(), p);
		assert(it != people.end());
		people.erase(it);

		delete p;
	}

	Car* retrive_work_vehicle(Building* work_building)
	{
		if (work_building->vehicles.empty())
			return nullptr;

		Car* car{ work_building->vehicles.back() };
		work_building->vehicles.pop_back();

		return car;
	}

	void set_person_target(Person* p, const v3& target)
	{
		p->target = target;
		v2 direction = glm::normalize((v2)(p->target - p->position));
		f32 yaw = glm::acos(direction.x) * ((float)(direction.y > 0.0f) * 2.0f - 1.0f);
		p->object->SetTransform(p->position, v3{ 0.0f, 0.0f, yaw + glm::radians(180.0f) });
	}
}