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

	PersonManager::~PersonManager()
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
					if (p->status == PersonStatus::AtWork)
					{
						building_from = p->work;
						building_to = p->home;
					}
					if (building_to)
					{
						p->path = Helper::get_path(building_from, building_to);
						p->target_building = building_to;
					}
					else
					{
						p->path = Helper::get_path(building_from, 5);
						p->target_building = building_from;
					}
					RS_Transition* rs_transition = (RS_Transition*)p->path[0];
					p->road_segment = rs_transition->road_segment_index;
					RoadSegment& road_segment = road_segments[p->road_segment];
					RoadType& road_segment_type = road_types[road_segment.type];
					road_segment.people.push_back(p);
					p->position = building_from->position;
					p->object->SetTransform(p->position);
					p->status = PersonStatus::Walking;
					p->object->enabled = true;
					p->in_junction = false;

					assert(building_from->snapped_t_index < road_segment.curve_samples.size() - 1);
					v3 target_position = road_segment.curve_samples[building_from->snapped_t_index];
					assert(road_segment.curve_samples.size() - 1 >= building_from->snapped_t_index + 1);
					v3 target_position_plus_one = road_segment.curve_samples[building_from->snapped_t_index + 1];

					v3 dir = target_position_plus_one - target_position;
					v3 offsetted = target_position + dir * building_from->snapped_t;

					v3 sidewalf_position_offset = glm::normalize(v3{ dir.y, -dir.x, 0.0f }) * road_segment_type.lanes_from_left[0].distance_from_center;
					if (building_from->snapped_to_right == false)
						sidewalf_position_offset *= -1.0f;
					p->target = offsetted + sidewalf_position_offset;
					((RS_Transition*)p->path[0])->at_path_array_index = building_from->snapped_t_index;
					walking_people.push_back(p);
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

					if (p->in_junction)
					{
						auto& road_node = road_nodes[p->road_node];
						auto& connected_road_segments = road_node.roadSegments;

						RN_Transition* rn_transition = (RN_Transition*)p->path[0];
						RS_Transition* rs_transition = (RS_Transition*)p->path[1];

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

								((RS_Transition*)p->path[0])->at_path_array_index = at_index;
								p->in_junction = false;
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
								p->target = ps[2];
							else if (rn_transition->sub_index == 1)
								p->target = ps[1];
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

							p->target = ps[rn_transition->sub_index];

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
						v3 direction = glm::normalize(p->target - p->position);
						v2 dir = glm::normalize((v2)direction);
						f32 yaw = glm::acos(dir.x) * ((float)(dir.y > 0.0f) * 2.0f - 1.0f);
						p->object->SetTransform(p->position, v3{ 0.0f, 0.0f, yaw + glm::radians(180.0f) });
					}
					else if (p->heading_to_a_building)
					{
						if (p->target_building == p->home)
							p->status = PersonStatus::AtHome;
						else
							p->status = PersonStatus::AtWork;
						p->time_left = Utility::Random::Float(1.0f, 5.0f);
						p->object->enabled = false;
						p->heading_to_a_building = false;
						RoadSegment& segment = road_segments[p->road_segment];
						p->road_segment = -1;
						assert(remove_person_from(segment, p));
						assert(remove_walking_person_from(walking_people, p));
					}
					else
					{
						RS_Transition* rs_transition = (RS_Transition*)p->path[0];
						RoadSegment& road_segment = road_segments[p->road_segment];
						RoadType& road_segment_type = road_types[road_segment.type];

						u64 target_path_array_index;
						if (p->path.size() == 1)
							target_path_array_index = p->target_building->snapped_t_index;
						else if (rs_transition->from_start)
							target_path_array_index = road_segment.curve_samples.size() - 1;
						else
							target_path_array_index = 0;

						if ((rs_transition->from_start && (rs_transition->at_path_array_index >= target_path_array_index)) || 
							(!rs_transition->from_start && (rs_transition->at_path_array_index <= target_path_array_index)))
						{
							if (p->path.size() == 1)
							{
								p->target = p->target_building->position;
								p->heading_to_a_building = true;
							}
							else
							{
								p->road_node = rs_transition->from_start ? road_segment.EndNode : road_segment.StartNode;

								RoadNode& road_node = road_nodes[p->road_node];
								road_node.people.push_back(p);
								assert(remove_person_from(road_segment, p));
								p->in_junction = true;
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

						v3 offset = glm::normalize(v3{ dir2.y,-dir2.x, 0.0f }) * road_segment_type.lanes_from_left[0].distance_from_center;
						if (rs_transition->from_right)
							p->target = p2 + offset;
						else
							p->target = p2 - offset;

						v3 direction = glm::normalize(p->target - p->position);
						v2 dir = glm::normalize((v2)direction);
						f32 yaw = glm::acos(dir.x) * ((float)(dir.y > 0.0f) * 2.0f - 1.0f);
						p->object->SetTransform(p->position, v3{ 0.0f, 0.0f, yaw + glm::radians(180.0f) });
					}
				}
			}
			else if (p->status == PersonStatus::Driving)
			{
				// Arabalı A*
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
	void reset_person(Person* p)
	{
		GameScene* game = GameScene::ActiveGameScene;
		auto& walking_people = game->m_PersonManager.walking_people;
		auto& road_segments = game->m_RoadManager.road_segments;
		auto& road_nodes = game->m_RoadManager.road_nodes;

		if (p->road_segment != -1)
		{
			auto& people_on_the_road_segment = road_segments[p->road_segment].people;
			auto it = std::find(people_on_the_road_segment.begin(), people_on_the_road_segment.end(), p);
			if (it == people_on_the_road_segment.end()) assert(false);

			people_on_the_road_segment.erase(it);
		}
		else if (p->road_node != -1)
		{
			auto& people_on_the_road_node = road_nodes[p->road_node].people;
			auto it = std::find(people_on_the_road_node.begin(), people_on_the_road_node.end(), p);
			if (it == people_on_the_road_node.end()) assert(false);

			people_on_the_road_node.erase(it);
		}

		auto it = std::find(walking_people.begin(), walking_people.end(), p);
		if (it == walking_people.end()) assert(false);
		walking_people.erase(it);

		p->road_segment = -1;
		p->road_node = -1;
		while (p->path.size() > 0)
		{
			delete p->path[p->path.size() - 1];
			p->path.pop_back();
		}
		p->status = PersonStatus::AtHome;
		p->time_left = Utility::Random::Float(1.0f, 2.0f);
		p->object->enabled = false;
		p->heading_to_a_building = false;
		p->in_junction = false;
		p->target_building = nullptr;
	}
	void remove_person(Person* p)
	{
		GameScene* game = GameScene::ActiveGameScene;
		auto& people = game->m_PersonManager.m_People;
		auto& walking_people = game->m_PersonManager.walking_people;
		auto& road_segments = game->m_RoadManager.road_segments;
		auto& road_nodes = game->m_RoadManager.road_nodes;

		if (p->road_segment != -1)
		{
			auto& walking_people_road_segments = road_segments[p->road_segment].people;
			auto it = std::find(walking_people_road_segments.begin(), walking_people_road_segments.end(), p);
			assert(it != walking_people_road_segments.end());

			walking_people_road_segments.erase(it);
		}
		else if (p->road_node != -1)
		{
			auto& walking_people_road_node = road_nodes[p->road_node].people;
			auto it = std::find(walking_people_road_node.begin(), walking_people_road_node.end(), p);
			assert(it != walking_people_road_node.end());

			walking_people_road_node.erase(it);
		}
		if (p->iCar)
		{
			// When the time comes
		}
		if (p->home)
		{
			auto& resident = p->home->people;
			auto it = std::find(resident.begin(), resident.end(), p);
			assert(it != resident.end());

			resident.erase(it);
		}
		if (p->work)
		{
			auto& workers = p->work->people;
			auto it = std::find(workers.begin(), workers.end(), p);
			assert(it != workers.end());

			workers.erase(it);
		}
		if (p->status == PersonStatus::Walking)
		{
			auto it = std::find(walking_people.begin(), walking_people.end(), p);
			assert(it != walking_people.end());
			walking_people.erase(it);
		}
		auto it = std::find(people.begin(), people.end(), p);
		assert(it != people.end());
		people.erase(it);

		delete p;
	}
	bool remove_walking_person_from(std::vector<Person*>& walking_people, Person* person)
	{
		auto it = std::find(walking_people.begin(), walking_people.end(), person);
		if (it != walking_people.end())
		{
			walking_people.erase(it);
			return true;
		}
		return false;
	}
}