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
		auto& segments = m_Scene->m_RoadManager.road_segments;
		auto& nodes = m_Scene->m_RoadManager.m_Nodes;
		auto& road_types = m_Scene->MainApplication->road_types;
		for (size_t i = 0; i < m_People.size(); i++)
		{
			Person* p = m_People[i];
			if (p->status == PersonStatus::AtHome)
			{
				p->time_left -= ts;
				if (p->time_left <= 0.0f)
				{
					// go to work work work work
					if (p->work)
					{
						p->path = Helper::get_path(p->home, p->work);
						p->target_building = p->work;
					}
					else
					{
						p->path = Helper::get_path(p->home, 5);
						p->target_building = p->home;
					}
					RS_Transition* rs_transition = (RS_Transition*)p->path[0];
					p->road_segment = rs_transition->road_segment;
					RoadSegment& rs = segments[p->road_segment];
					if (p->path.size() == 1)
					{
						if (p->work->snapped_t_index == p->home->snapped_t_index)
							p->from_start = p->work->snapped_t > p->home->snapped_t;
						else
							p->from_start = p->work->snapped_t_index > p->home->snapped_t_index;
					}
					else
					{
						RN_Transition* rn_transition = (RN_Transition*)p->path[1];
						p->from_start = rn_transition->road_node == rs.EndNode;
					}
					p->position = p->home->position;
					p->status = PersonStatus::Walking;
					p->object->SetTransform(p->position);
					p->object->enabled = true;
					p->in_junction = false;
					rs.peoples.push_back(p);

					v3 p1 = rs.curve_samples[(std::max)((s64)0, (s64)rs_transition->from_index - 1)];
					v3 p2 = rs.curve_samples[rs_transition->from_index];
					v3 p3 = rs.curve_samples[(std::min)(rs.curve_samples.size() - 2, rs_transition->from_index + 1)];

					v3 dir = (p->from_start) ? p3 - p2 : p1 - p2;
					v3 offset = glm::normalize(v3{ dir.y,-dir.x, 0.0f }) * rs_transition->distance_from_middle;
					p->target = p2 + offset;

					walking_people.push_back(p);
				}

			}
			else if (p->status == PersonStatus::AtWork)
			{
				p->time_left -= ts;
				if (p->time_left <= 0.0f)
				{
					p->road_segment = p->work->connectedRoadSegment;
					p->position = p->work->position;
					p->status = PersonStatus::Walking;
					p->object->SetTransform(p->position);
					p->object->enabled = true;
					p->in_junction = false;
					if (p->home)
					{
						p->path = Helper::get_path(p->work, p->home);
						p->target_building = p->home;
					}
					else
					{
						p->path = Helper::get_path(p->work, 5);
						p->target_building = p->work;
					}
					RS_Transition* rs_transition = (RS_Transition*)p->path[0];
					RoadSegment& rs = segments[p->road_segment];
					rs.peoples.push_back(p);
					walking_people.push_back(p);
					p->target = {};
				}
			}
			else if (p->status == PersonStatus::Walking)
			{
				if (p->in_junction)
				{
					v3 ab = p->target - p->position;
					f32  leftLenght = glm::length(ab);
					f32  journeyLength = ts * p->speed;
					if (journeyLength < leftLenght)
					{
						v3 unit = ab / leftLenght;
						p->position = p->position + unit * journeyLength;
						p->object->SetTransform(p->position);
					}
					else
					{
						p->position = p->target;
						p->object->SetTransform(p->position);
						auto& road_node = nodes[p->road_node];
						auto& road_segments = road_node.roadSegments;

						RN_Transition* rn_transition = (RN_Transition*)p->path[0];
						u64 road_segment_count = road_segments.size();
						if (road_segment_count == 1)
						{
							RoadSegment& curr_rs = segments[road_segments[rn_transition->from_index]];

							RS_Transition* rs_transition = (RS_Transition*)p->path[1];
							bool is_start = curr_rs.StartNode == p->road_node;
							bool enter_from_right = rs_transition->from_right == is_start;
							if ((rn_transition->sub_index == 2 && enter_from_right) ||
								(rn_transition->sub_index == 1 && !enter_from_right))
							{
								p->path.erase(p->path.begin());
								p->in_junction = false;
								auto it = std::find(road_node.people.begin(), road_node.people.end(), p);
								if (it != road_node.people.end())
									road_node.people.erase(it);
								else
									assert(false);

								p->road_segment = road_node.roadSegments[0];
								segments[p->road_segment].peoples.push_back(p);
								p->from_start = segments[p->road_segment].StartNode == p->road_node;
								p->road_node = -1;
							}
							else
							{
								if (rn_transition->sub_index == 0)
									assert(false);
								else if (rn_transition->sub_index == 3)
									assert(false);

								if (rn_transition->accending)
									rn_transition->sub_index++;
								else
									rn_transition->sub_index--;

								if (rn_transition->sub_index == 0)
									rn_transition->sub_index == 2;
								else if (rn_transition->sub_index == 3)
									rn_transition->sub_index == 1;

								RoadType& curr_rs_type = road_types[curr_rs.type];
								f32 curr_rs_width = curr_rs_type.road_width * 0.5f;
								v3 curr_rs_pos = p->road_node == curr_rs.StartNode ? curr_rs.GetStartPosition() : curr_rs.GetEndPosition();
								v3 curr_rs_dir = p->road_node == curr_rs.StartNode ? curr_rs.GetStartDirection() : curr_rs.GetEndDirection();
								v3 rotated_curr_rs_dir = glm::normalize(v3{ curr_rs_dir.y, -curr_rs_dir.x, 0.0f });
								std::array<v3, 4> ps{};
								ps[1] = curr_rs_pos - rotated_curr_rs_dir * curr_rs_width;
								ps[2] = curr_rs_pos + rotated_curr_rs_dir * curr_rs_width;
								p->target = ps[rn_transition->sub_index];

								v3 direction = glm::normalize(p->target - p->position);
								v2 dir = glm::normalize((v2)direction);
								f32 yaw = glm::acos(dir.x) * ((float)(dir.y > 0.0f) * 2.0f - 1.0f);
								p->object->SetTransform(p->position, v3{ 0.0f, 0.0f, yaw + glm::radians(180.0f) });
							}
						}
						else
						{
							RoadSegment& prev_rs = segments[road_segments[(rn_transition->from_index - 1 + road_segment_count) % road_segment_count]];
							RoadSegment& curr_rs = segments[road_segments[rn_transition->from_index]];
							RoadSegment& next_rs = segments[road_segments[(rn_transition->from_index + 1) % road_segment_count]];

							RS_Transition* rs_transition = (RS_Transition*)p->path[1];
							bool is_start = curr_rs.StartNode == p->road_node;
							bool enter_from_right = rs_transition->from_right == is_start;
							if (rn_transition->from_index == rn_transition->to_index)
							{
								if ((rn_transition->sub_index == 2 && enter_from_right) ||
									(rn_transition->sub_index == 1 && !enter_from_right))
								{
									p->path.erase(p->path.begin());
									p->in_junction = false;
									auto it = std::find(road_node.people.begin(), road_node.people.end(), p);
									if (it != road_node.people.end())
										road_node.people.erase(it);
									else
										assert(false);

									p->road_segment = road_node.roadSegments[rn_transition->to_index];
									segments[p->road_segment].peoples.push_back(p);
									p->from_start = segments[p->road_segment].StartNode == p->road_node;
									p->road_node = -1;
									continue;
								}
							}

							if (rn_transition->accending)
								rn_transition->sub_index++;
							else
								rn_transition->sub_index--;

							if (rn_transition->sub_index == 0)
							{
								rn_transition->sub_index = 3;
								rn_transition->from_index--;
								if (rn_transition->from_index == -1)
									rn_transition->from_index = road_segment_count - 1;
							}
							else if (rn_transition->sub_index == 3)
							{
								rn_transition->sub_index = 0;
								rn_transition->from_index++;
								if (rn_transition->from_index == road_segment_count)
									rn_transition->from_index = 0;
							}


							RoadType& prev_rs_type = road_types[prev_rs.type];
							RoadType& curr_rs_type = road_types[curr_rs.type];
							RoadType& next_rs_type = road_types[next_rs.type];

							f32 prev_rs_width = prev_rs_type.road_width * 0.5f;
							f32 curr_rs_width = curr_rs_type.road_width * 0.5f;
							f32 next_rs_width = next_rs_type.road_width * 0.5f;

							v3 prev_rs_pos = p->road_node == prev_rs.StartNode ? prev_rs.GetStartPosition() : prev_rs.GetEndPosition();
							v3 curr_rs_pos = p->road_node == curr_rs.StartNode ? curr_rs.GetStartPosition() : curr_rs.GetEndPosition();
							v3 next_rs_pos = p->road_node == next_rs.StartNode ? next_rs.GetStartPosition() : next_rs.GetEndPosition();

							v3 prev_rs_dir = p->road_node == prev_rs.StartNode ? prev_rs.GetStartDirection() : prev_rs.GetEndDirection();
							v3 curr_rs_dir = p->road_node == curr_rs.StartNode ? curr_rs.GetStartDirection() : curr_rs.GetEndDirection();
							v3 next_rs_dir = p->road_node == next_rs.StartNode ? next_rs.GetStartDirection() : next_rs.GetEndDirection();

							v3 rotated_prev_rs_dir = glm::normalize(v3{ prev_rs_dir.y, -prev_rs_dir.x, 0.0f });
							v3 rotated_curr_rs_dir = glm::normalize(v3{ curr_rs_dir.y, -curr_rs_dir.x, 0.0f });
							v3 rotated_next_rs_dir = glm::normalize(v3{ next_rs_dir.y, -next_rs_dir.x, 0.0f });

							std::array<v3, 4> ps{};
							ps[1] = curr_rs_pos - rotated_curr_rs_dir * curr_rs_width;
							ps[2] = curr_rs_pos + rotated_curr_rs_dir * curr_rs_width;
							ps[0] = Math::ray_plane_intersection(
								prev_rs_pos + rotated_prev_rs_dir * prev_rs_width,
								prev_rs_dir,
								ps[1],
								rotated_curr_rs_dir
							);
							ps[3] = Math::ray_plane_intersection(
								next_rs_pos + rotated_next_rs_dir * next_rs_width,
								next_rs_dir,
								ps[2],
								rotated_curr_rs_dir
							);
							v3 temp = ps[1]; // ???
							ps[1] = ps[2];	 // ???
							ps[2] = temp;	 // ???
							p->target = ps[rn_transition->sub_index];


							v3 direction = glm::normalize(p->target - p->position);
							v2 dir = glm::normalize((v2)direction);
							f32 yaw = glm::acos(dir.x) * ((float)(dir.y > 0.0f) * 2.0f - 1.0f);
							p->object->SetTransform(p->position, v3{ 0.0f, 0.0f, yaw + glm::radians(180.0f) });
						}
					}
				}
				else if (p->heading_to_a_building)
				{
					v3 ab = p->target - p->position;
					f32  leftLenght = glm::length(ab);
					v3 unit = ab / leftLenght;
					f32  journeyLength = ts * p->speed;
					if (journeyLength < leftLenght)
					{
						p->position = p->position + unit * journeyLength;
						p->object->SetTransform(p->position);
					}
					else
					{
						if (p->target_building == p->home)
							p->status = PersonStatus::AtHome;
						else
							p->status = PersonStatus::AtWork;
						p->time_left = Utility::Random::Float(1.0f, 5.0f);
						p->object->enabled = false;
						p->heading_to_a_building = false;
						RoadSegment& segment = segments[p->road_segment];
						p->road_segment = -1;
						segment.peoples.erase(std::find(segment.peoples.begin(), segment.peoples.end(), p));

						auto it = std::find(walking_people.begin(), walking_people.end(), p);
						if (it != walking_people.end())
							walking_people.erase(it);
						else
							assert(false);
					}
				}
				else
				{
					v3 ab = p->target - p->position;
					f32  leftLenght = glm::length(ab);
					v3 unit = ab / leftLenght;
					f32  journeyLength = ts * p->speed;
					RS_Transition* rs_transition = (RS_Transition*)p->path[0];
					RoadSegment& road_segment = segments[p->road_segment];
					if (journeyLength < leftLenght)
					{
						p->position = p->position + unit * journeyLength;
						p->object->SetTransform(p->position);
					}
					else
					{
						//f32  length_of_the_road_prefab = app->road_types[current_road.type].road_length;
						p->position = p->target;
						p->object->SetTransform(p->position);

						if (rs_transition->from_index == rs_transition->to_index)
						{
							if (p->path.size() == 1)
							{
								p->target = p->target_building->position;
								p->heading_to_a_building = true;
							}
							else
							{
								RoadSegment& road_segment = segments[p->road_segment];
								if (rs_transition->to_index == 0)
									p->road_node = road_segment.StartNode;
								else
									p->road_node = road_segment.EndNode;

								RoadNode& road_node = nodes[p->road_node];
								road_node.people.push_back(p);
								auto it = std::find(road_segment.peoples.begin(), road_segment.peoples.end(), p);
								if (it == road_segment.peoples.end()) assert(false);
								road_segment.peoples.erase(it);

								p->path.erase(p->path.begin());
								p->in_junction = true;
							}
							continue;
						}

						v3 p1 = road_segment.curve_samples[(std::max)((s64)rs_transition->from_index - 1, (s64)0)];
						v3 p2 = road_segment.curve_samples[rs_transition->from_index];
						v3 p3 = road_segment.curve_samples[(std::min)(rs_transition->from_index + 1, road_segment.curve_samples.size() - 1)];
						if (p->from_start)
						{
							v3 dir = p3 - p2;
							v3 offset = glm::normalize(v3{ dir.y,-dir.x, 0.0f }) * rs_transition->distance_from_middle;

							rs_transition->from_index++;
							p->target = p3;
							if (p->path.size() == 1)
								if (rs_transition->from_index >= rs_transition->to_index)
									p->target = (p3 - p2) * p->target_building->snapped_t;
							p->target += offset;
						}
						else
						{
							v3 dir = p1 - p2;
							v3 offset = glm::normalize(v3{ dir.y,-dir.x, 0.0f }) * rs_transition->distance_from_middle;

							rs_transition->from_index--;
							p->target = p1;
							if (p->path.size() == 1)
								if (rs_transition->from_index <= rs_transition->to_index)
									p->target = (p1 - p2) * (1.0f - p->target_building->snapped_t);
							p->target += offset;
						}

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

	void remove_person(Person* p)
	{
		GameScene* game = GameScene::ActiveGameScene;
		auto& people = game->m_PersonManager.m_People;
		auto& segments = game->m_RoadManager.road_segments;

		if (p->road_segment != -1)
		{
			auto& walking_people = segments[p->road_segment].peoples;
			auto it = std::find(walking_people.begin(), walking_people.end(), p);
			if (it != walking_people.end())
				walking_people.erase(it);
			else
				assert(false);
		}
		if (p->home)
		{
			auto& resident = p->home->residents;
			auto it = std::find(resident.begin(), resident.end(), p);
			if (it != resident.end())
				resident.erase(it);
			else
				assert(false);
		}
		if (p->work)
		{
			auto& workers = p->work->workers;
			auto it = std::find(workers.begin(), workers.end(), p);
			if (it != workers.end())
				workers.erase(it);
			else
				assert(false);
		}
		auto it = std::find(people.begin(), people.end(), p);
		if (it != people.end())
			people.erase(it);
		else
			assert(false);

		delete p;
	}
}