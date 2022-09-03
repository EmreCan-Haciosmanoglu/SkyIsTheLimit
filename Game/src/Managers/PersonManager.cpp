#include "canpch.h"
#include "PersonManager.h"
#include "Scenes/GameScene.h"
#include "Types/RoadSegment.h"
#include "Types/RoadNode.h"
#include "Types/Transition.h"
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
		GameApp* app = m_Scene->MainApplication;
		auto& road_segments = m_Scene->m_RoadManager.road_segments;
		auto& road_nodes = m_Scene->m_RoadManager.road_nodes;
		auto& road_types = m_Scene->MainApplication->road_types;
		for (size_t i = 0; i < m_People.size(); i++)
		{
			Person* p = m_People[i];
			if ((p->status == PersonStatus::AtHome) ||
				(p->status == PersonStatus::AtWork))
			{
				p->time_left -= ts;
				if (p->time_left <= 0.0f)
				{
					// go to work work work work
					Building* building_from = p->home;
					Building* building_to = p->work;
					bool is_buildings_connected = true;
					if (p->status == PersonStatus::AtWork)
					{
						building_from = p->work;
						building_to = p->home;
					}
					if (building_to)
					{
						p->path_end_building = building_to;
						p->path_start_building = building_from;
						if (p->car)
							p->path = Helper::get_path_for_a_car(building_from, building_to);
						else
							p->path = Helper::get_path(building_from, building_to);

						if (p->path.size() == 0)
						{
							if (building_from == p->work)
							{
								reset_person_back_to_home(p);
								continue;
							}
							is_buildings_connected = false;
							p->path = Helper::get_path(building_from, 5);
							p->path_end_building = building_from;
						}
					}
					else
					{
						p->path = Helper::get_path(building_from, 5);
						p->path_end_building = building_from;
						p->path_start_building = building_from;
					}

					p->position = building_from->position;
					p->object->SetTransform(p->position);
					p->object->enabled = true;
					p->heading_to_a_building_or_parking = false;
					p->heading_to_a_car = false;
					p->road_segment = building_from->connectedRoadSegment;
					RoadSegment& road_segment = road_segments[p->road_segment];
					road_segment.people.push_back(p);
					p->status = PersonStatus::Walking;
					people_on_the_road.push_back(p);

					if (p->car && building_to && building_from != p->path_end_building)
					{
						RS_Transition_For_Driving* rs_transition = (RS_Transition_For_Driving*)p->path[0];
						p->heading_to_a_car = true;
						rs_transition->at_path_array_index = building_from->snapped_t_index;

						set_person_target(p, p->car->object->position);
					}
					else
					{
						RS_Transition_For_Walking* rs_transition = (RS_Transition_For_Walking*)p->path[0];
						RoadType& road_segment_type = road_types[road_segment.type];

						assert(building_from->snapped_t_index < (s64)road_segment.curve_samples.size() - 1);
						v3 target_position = road_segment.curve_samples[building_from->snapped_t_index];
						assert(road_segment.curve_samples.size() - 1 >= building_from->snapped_t_index + 1);
						v3 target_position_plus_one = road_segment.curve_samples[building_from->snapped_t_index + 1];

						v3 dir = target_position_plus_one - target_position;
						v3 offsetted = target_position + dir * building_from->snapped_t;

						v3 sidewalf_position_offset = glm::normalize(v3{ dir.y, -dir.x, 0.0f });
						if (building_from->snapped_to_right)
							sidewalf_position_offset *= road_segment_type.lanes_forward[road_segment_type.lanes_forward.size() - 1].distance_from_center;
						else
							sidewalf_position_offset *= road_segment_type.lanes_backward[0].distance_from_center;

						((RS_Transition_For_Walking*)p->path[0])->at_path_array_index = building_from->snapped_t_index;

						set_person_target(p, offsetted + sidewalf_position_offset);
					}
				}

			}
			else if (p->status == PersonStatus::Walking)
			{
				v3 left = p->target - p->position;
				f32 left_length = glm::length(left);
				f32 journey_length = ts * p->speed;
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
						RoadType& current_road_segment_type = road_types[current_road_segment.type];
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

							RoadType& prev_road_segment_type = road_types[prev_road_segment.type];
							RoadType& next_road_segment_type = road_types[next_road_segment.type];

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
					else if (p->heading_to_a_building_or_parking)
					{
						if (p->path_end_building == p->home)
							p->status = PersonStatus::AtHome;
						else
							p->status = PersonStatus::AtWork;
						p->time_left = Utility::Random::Float(1.0f, 5.0f);
						p->object->enabled = false;
						p->heading_to_a_building_or_parking = false;
						RoadSegment& segment = road_segments[p->road_segment];
						p->road_segment = -1;

						delete ((RS_Transition_For_Driving*)p->path[0]);
						p->path.pop_back();

						assert(remove_person_from(segment, p));

						auto it = std::find(people_on_the_road.begin(), people_on_the_road.end(), p);
						assert(it != people_on_the_road.end());
						people_on_the_road.erase(it);
					}
					else if (p->heading_to_a_car)
					{
						p->object->enabled = false;
						p->status = PersonStatus::Driving;
						p->heading_to_a_car = false;

						RS_Transition_For_Driving* rs_transition = (RS_Transition_For_Driving*)p->path[0];
						RoadSegment& road_segment = road_segments[rs_transition->road_segment_index];
						RoadType& road_segment_type = road_types[road_segment.type];

						v3 target_position = road_segment.curve_samples[rs_transition->at_path_array_index];
						assert(rs_transition->at_path_array_index + 1 < road_segment.curve_samples.size());
						v3 target_position_plus_one = road_segment.curve_samples[rs_transition->at_path_array_index + 1];

						v3 dir = target_position_plus_one - target_position;
						v3 sidewalf_position_offset = glm::normalize(v3{ dir.y, -dir.x, 0.0f });
						if (rs_transition->lane_index < road_segment_type.lanes_backward.size())
							sidewalf_position_offset *= road_segment_type.lanes_backward[rs_transition->lane_index].distance_from_center;
						else
							sidewalf_position_offset *= road_segment_type.lanes_forward[rs_transition->lane_index - road_segment_type.lanes_backward.size()].distance_from_center;
						p->position = p->car->object->position;
						set_target_and_car_direction(p, target_position + sidewalf_position_offset);
					}
					else
					{
						RS_Transition_For_Walking* rs_transition = (RS_Transition_For_Walking*)p->path[0];
						RoadSegment& road_segment = road_segments[p->road_segment];
						RoadType& road_segment_type = road_types[road_segment.type];

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
								p->target = p->path_end_building->position;
								p->heading_to_a_building_or_parking = true;
							}
							else
							{
								p->road_node = rs_transition->from_start ? road_segment.EndNode : road_segment.StartNode;

								RoadNode& road_node = road_nodes[p->road_node];
								road_node.people.push_back(p);
								assert(remove_person_from(road_segment, p));
								p->road_segment = -1;

								auto path = p->path[0];
								p->path.erase(p->path.begin());
								delete path;
							}
							continue;
						}

						v3 p2{};
						v3 dir2{};
						if (rs_transition->from_start)
						{
							u64 next_index = std::min(rs_transition->at_path_array_index + 1, road_segment.curve_samples.size() - 1);
							p2 = road_segment.curve_samples[next_index];
							if (next_index < road_segment.curve_samples.size() - 1)
							{
								v3 p3 = road_segment.curve_samples[next_index + 1];
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
							u64 next_index = std::min(rs_transition->at_path_array_index - 1, road_segment.curve_samples.size() - 1);
							p2 = road_segment.curve_samples[next_index];
							if (next_index > 0)
							{
								v3 p3 = road_segment.curve_samples[next_index - 1];
								dir2 = glm::normalize(p3 - p2);
							}
							else
							{
								dir2 = road_segment.GetStartDirection() * -1.0f;
							}
							rs_transition->at_path_array_index = next_index;
						}
						v3 offset = glm::normalize(v3{ dir2.y,-dir2.x, 0.0f });
						if (rs_transition->from_right)
							offset *= road_segment_type.lanes_forward[road_segment_type.lanes_forward.size() - 1].distance_from_center;
						else
							offset *= road_segment_type.lanes_backward[0].distance_from_center;

						set_person_target(p, p2 + offset);
					}
				}
			}
			else if (p->status == PersonStatus::Driving)
			{
				Car* c = p->car;
				v3 left = p->target - c->object->position;
				f32 left_length = glm::length(left);
				f32 journey_length = ts * p->speed;
				if (journey_length < left_length)
				{
					v3 unit = left / left_length;
					c->object->SetTransform(c->object->position + unit * journey_length);
				}
				else
				{
					if (p->heading_to_a_building_or_parking)
					{
						c->object->SetTransform(
							p->target,
							glm::rotateZ(
								p->path_end_building->object->rotation,
								glm::radians(p->path_end_building->car_park.rotation_in_degrees)
							)
						);
						p->position = c->object->position;
						p->object->SetTransform(p->position);
						p->object->enabled = true;
						p->status = PersonStatus::Walking;

						set_person_target(p, p->path_end_building->position);
					}
					else
					{
						c->object->SetTransform(p->target);
						p->position = p->target;
						assert(p->path.size() > 0);
						RS_Transition_For_Driving* rs_transition = (RS_Transition_For_Driving*)p->path[0];
						RoadSegment& road_segment = road_segments[rs_transition->road_segment_index];
						RoadType& road_type = road_types[road_segment.type];
						if (rs_transition->lane_index >= road_type.lanes_backward.size())
						{
							if (p->path.size() == 1)
							{
								if (rs_transition->at_path_array_index >= p->path_end_building->snapped_t_index)
								{
									v3 car_park_pos = p->path_end_building->position +
										(v3)(glm::rotate(m4(1.0f), p->path_end_building->object->rotation.z, v3{ 0.0f, 0.0f, 1.0f }) *
											glm::rotate(m4(1.0f), p->path_end_building->object->rotation.y, v3{ 0.0f, 1.0f, 0.0f }) *
											glm::rotate(m4(1.0f), p->path_end_building->object->rotation.x, v3{ 1.0f, 0.0f, 0.0f }) *
											v4(p->path_end_building->car_park.offset, 1.0f));
									set_target_and_car_direction(p, car_park_pos);
									p->heading_to_a_building_or_parking = true;
									continue;
								}
							}
							if (rs_transition->at_path_array_index < road_segment.curve_samples.size() - 1)
							{
								rs_transition->at_path_array_index++;
								v3 dir = (rs_transition->at_path_array_index < road_segment.curve_samples.size() - 1) ?
									road_segment.curve_samples[rs_transition->at_path_array_index + 1] - road_segment.curve_samples[rs_transition->at_path_array_index] :
									road_segment.GetEndDirection() * -1.0f;

								v3 rotated_dir = glm::normalize(v3{ dir.y, -dir.x, 0.0f });
								v3 target = road_segment.curve_samples[rs_transition->at_path_array_index];
								target += rotated_dir * road_type.lanes_forward[rs_transition->lane_index - road_type.lanes_backward.size()].distance_from_center;

								set_target_and_car_direction(p, target);
								continue;
							}
						}
						else
						{
							if (p->path.size() == 1)
							{
								if (rs_transition->at_path_array_index <= p->path_end_building->snapped_t_index)
								{
									v3 car_park_pos = p->path_end_building->position +
										(v3)(glm::rotate(m4(1.0f), p->path_end_building->object->rotation.z, v3{ 0.0f, 0.0f, 1.0f }) *
											glm::rotate(m4(1.0f), p->path_end_building->object->rotation.y, v3{ 0.0f, 1.0f, 0.0f }) *
											glm::rotate(m4(1.0f), p->path_end_building->object->rotation.x, v3{ 1.0f, 0.0f, 0.0f }) *
											v4(p->path_end_building->car_park.offset, 1.0f));
									set_target_and_car_direction(p, car_park_pos);
									p->heading_to_a_building_or_parking = true;
									continue;
								}
							}
							if (rs_transition->at_path_array_index > 0)
							{
								rs_transition->at_path_array_index--;
								assert(rs_transition->at_path_array_index + 1 < road_segment.curve_samples.size());
								v3 dir = road_segment.curve_samples[rs_transition->at_path_array_index + 1] - road_segment.curve_samples[rs_transition->at_path_array_index];

								v3 rotated_dir = glm::normalize(v3{ dir.y, -dir.x, 0.0f });
								v3 target = road_segment.curve_samples[rs_transition->at_path_array_index];
								target += rotated_dir * road_type.lanes_backward[rs_transition->lane_index].distance_from_center;

								set_target_and_car_direction(p, target);
								continue;
							}
						}

						/*Next Path*/ {
							s64 next_road_node_index = rs_transition->next_road_node_index;

							p->path.erase(p->path.begin());
							delete rs_transition;
							rs_transition = (RS_Transition_For_Driving*)p->path[0];

							auto& people_on_the_road = road_segments[p->road_segment].people;
							auto it = std::find(people_on_the_road.begin(), people_on_the_road.end(), p);
							assert(it != people_on_the_road.end());
							people_on_the_road.erase(it);

							RoadSegment& next_road_segment = road_segments[rs_transition->road_segment_index];
							p->road_segment = rs_transition->road_segment_index;
							next_road_segment.people.push_back(p);

							auto& curve_samples = next_road_segment.curve_samples;
							bool from_start = next_road_segment.EndNode == rs_transition->next_road_node_index;
							if (rs_transition->next_road_node_index == -1)
								from_start = next_road_segment.StartNode == next_road_node_index;
							rs_transition->at_path_array_index = from_start ? 0 : curve_samples.size() - 1;
							v3 end_point = curve_samples[rs_transition->at_path_array_index];
							v3 end_dir = from_start ? next_road_segment.GetStartDirection() : next_road_segment.GetEndDirection() * -1.0f;
							v3 rotated_dir = glm::normalize(v3{ end_dir.y, -end_dir.x, 0.0f });
							RoadType& next_road_type = road_types[next_road_segment.type];
							u64 lanes_backward_size = next_road_type.lanes_backward.size();
							if (rs_transition->lane_index < lanes_backward_size)
								rotated_dir *= next_road_type.lanes_backward[rs_transition->lane_index].distance_from_center;
							else
								rotated_dir *= next_road_type.lanes_forward[rs_transition->lane_index - lanes_backward_size].distance_from_center;

							set_target_and_car_direction(p, end_point + rotated_dir);
						}
					}
				}
			}
			else if (p->status == PersonStatus::WalkingDead)
			{
				//  slumpy A*
			}
		}
	}

	Person* PersonManager::get_worklessPerson()
	{
		for (Person* p : m_People)
		{
			if (!p->work)
			{
				return p;
			}
		}
		return nullptr;
	}
	void reset_person_back_to_building_from(Person* p)
	{
		GameScene* game = GameScene::ActiveGameScene;
		auto& people_on_the_road = game->m_PersonManager.people_on_the_road;
		auto& road_segments = game->m_RoadManager.road_segments;
		auto& road_nodes = game->m_RoadManager.road_nodes;

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
		p->position = p->path_start_building->position;
		p->object->SetTransform(p->position);
		p->object->enabled = false;
		if (p->path_start_building == p->home)
		{
			p->status = PersonStatus::AtHome;
			p->time_left = Utility::Random::Float(1.0f, 2.0f);	// home values
		}
		else if (p->path_start_building == p->work)
		{
			p->status = PersonStatus::AtWork;
			p->time_left = Utility::Random::Float(1.0f, 2.0f);	// work values
		}
		else
		{
			assert(false); // at some other building (for the future)
		}

		while (p->path.size() > 0)
		{
			delete p->path[p->path.size() - 1];
			p->path.pop_back();
		}

		if (p->car)
		{
			Building* b = p->path_start_building;
			v3 car_pos = b->position +
				(v3)(glm::rotate(m4(1.0f), b->object->rotation.z, v3{ 0.0f, 0.0f, 1.0f }) *
					glm::rotate(m4(1.0f), b->object->rotation.y, v3{ 0.0f, 1.0f, 0.0f }) *
					glm::rotate(m4(1.0f), b->object->rotation.x, v3{ 1.0f, 0.0f, 0.0f }) *
					v4(b->car_park.offset, 1.0f));
			p->car->object->SetTransform(
				car_pos,
				glm::rotateZ(
					b->object->rotation,
					glm::radians(b->car_park.rotation_in_degrees)
				)
			);
		}

		p->path_end_building = nullptr;
		p->path_start_building = nullptr;

		p->from_right = false;
		p->heading_to_a_building_or_parking = false;
		p->heading_to_a_car = false;

		auto it = std::find(people_on_the_road.begin(), people_on_the_road.end(), p);
		if (it == people_on_the_road.end()) assert(false);
		people_on_the_road.erase(it);

	}
	void reset_person_back_to_home(Person* p)
	{
		GameScene* game = GameScene::ActiveGameScene;
		auto& people_on_the_road = game->m_PersonManager.people_on_the_road;
		auto& road_segments = game->m_RoadManager.road_segments;
		auto& road_nodes = game->m_RoadManager.road_nodes;

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
		p->position = p->path_start_building->position;
		p->object->SetTransform(p->position);
		p->object->enabled = false;
		p->status = PersonStatus::AtHome;
		p->time_left = Utility::Random::Float(1.0f, 2.0f);	// home values

		while (p->path.size() > 0)
		{
			delete p->path[p->path.size() - 1];
			p->path.pop_back();
		}

		if (p->car)
		{
			Building* b = p->path_start_building;
			v3 car_pos = b->position +
				(v3)(glm::rotate(m4(1.0f), b->object->rotation.z, v3{ 0.0f, 0.0f, 1.0f }) *
					glm::rotate(m4(1.0f), b->object->rotation.y, v3{ 0.0f, 1.0f, 0.0f }) *
					glm::rotate(m4(1.0f), b->object->rotation.x, v3{ 1.0f, 0.0f, 0.0f }) *
					v4(b->car_park.offset, 1.0f));
			p->car->object->SetTransform(
				car_pos,
				glm::rotateZ(
					b->object->rotation,
					glm::radians(b->car_park.rotation_in_degrees)
				)
			);
		}

		p->path_end_building = nullptr;
		p->path_start_building = nullptr;

		p->from_right = false;
		p->heading_to_a_building_or_parking = false;
		p->heading_to_a_car = false;

		auto it = std::find(people_on_the_road.begin(), people_on_the_road.end(), p);
		if (it == people_on_the_road.end()) assert(false);
		people_on_the_road.erase(it);

	}
	void remove_person(Person* p)
	{
		GameScene* game = GameScene::ActiveGameScene;
		auto& people = game->m_PersonManager.m_People;
		auto& people_on_the_road = game->m_PersonManager.people_on_the_road;
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
			auto it = std::find(people_on_the_road.begin(), people_on_the_road.end(), p);
			assert(it != people_on_the_road.end());
			people_on_the_road.erase(it);
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

		auto it = std::find(people.begin(), people.end(), p);
		assert(it != people.end());
		people.erase(it);

		delete p;
	}

	void set_target_and_car_direction(Person* p, const v3& target)
	{
		p->target = target;
		v2 direction = glm::normalize((v2)(p->target - p->position));
		f32 yaw = glm::acos(direction.x) * ((float)(direction.y > 0.0f) * 2.0f - 1.0f);
		p->car->object->SetTransform(p->position, v3{ 0.0f, 0.0f, yaw + glm::radians(180.0f) });
	}

	void set_person_target(Person* p, const v3& target)
	{
		p->target = target;
		v2 direction = glm::normalize((v2)(p->target - p->position));
		f32 yaw = glm::acos(direction.x) * ((float)(direction.y > 0.0f) * 2.0f - 1.0f);
		p->object->SetTransform(p->position, v3{ 0.0f, 0.0f, yaw + glm::radians(180.0f) });
	}
}