#include "canpch.h"
#include "PersonManager.h"
#include "Scenes/GameScene.h"
#include "Types/RoadSegment.h"
#include "Types/RoadNode.h"
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
		auto& segments = m_Scene->m_RoadManager.m_Segments;
		auto& nodes = m_Scene->m_RoadManager.m_Nodes;
		for (size_t i = 0; i < m_People.size(); i++)
		{
			Person* p = m_People[i];
			if (p->status == PersonStatus::AtHome)
			{
				p->time_left -= ts;
				if (p->time_left <= 0.0f)
				{
					// go to work work work work
					walking_people.push_back(p);
					p->road_segment = p->home->connectedRoadSegment;
					p->t = p->home->snappedT;
					p->drift_points = {};
					p->position = p->home->position;
					p->status = PersonStatus::Walking;
					p->object->SetTransform(p->position);
					p->object->enabled = true;
					p->in_junction = false;
					if (p->work)
					{
						p->path = Helper::get_path((u64)p->home->connectedRoadSegment, (u64)p->work->connectedRoadSegment);
						p->target_building = p->work;
					}
					else
					{
						p->path = Helper::get_path((u64)p->home->connectedRoadSegment, (u8)5);
						p->target_building = p->home;
					}

					RoadSegment& rs = segments[p->road_segment];
					rs.peoples.push_back(p);
					std::vector<f32>& t_samples = rs.curve_t_samples;
					for (u64 i = 0; i < t_samples.size(); i++)
					{
						if (p->t < t_samples[i])
						{
							p->t_index = i - 1;
							p->target = rs.curve_samples[i];
							if (p->path.size() == 1)
								p->from_start = p->work->snappedT > p->home->snappedT;
							else
								p->from_start = rs.EndNode == p->path[1];
							break;
						}
					}
				}

			}
			else if (p->status == PersonStatus::AtWork)
			{
				p->time_left -= ts;
				if (p->time_left <= 0.0f)
				{
					walking_people.push_back(p);
					p->road_segment = p->work->connectedRoadSegment;
					p->t = p->work->snappedT;
					p->drift_points = {};
					p->position = p->work->position;
					p->status = PersonStatus::Walking;
					p->object->SetTransform(p->position);
					p->object->enabled = true;
					p->in_junction = false;
					if (p->home)
					{
						p->path = Helper::get_path((u64)p->work->connectedRoadSegment, (u64)p->home->connectedRoadSegment);
						p->target_building = p->home;
					}
					else
					{
						p->path = Helper::get_path((u64)p->work->connectedRoadSegment, (u8)5);
						p->target_building = p->work;
					}

					RoadSegment& rs = segments[p->road_segment];
					rs.peoples.push_back(p);
					std::vector<f32>& t_samples = rs.curve_t_samples;
					for (u64 i = 0; i < t_samples.size(); i++)
					{
						if (p->t < t_samples[i])
						{
							p->t_index = i - 1;
							p->target = rs.curve_samples[i];
							if (p->path.size() == 1)
								p->from_start = p->home->snappedT > p->work->snappedT;
							else
								p->from_start = rs.EndNode == p->path[1];
							break;
						}
					}
				}
			}
			else if (p->status == PersonStatus::Walking)
			{
				if (p->in_junction)
				{
					f32 some_value = 1.0f / 1.5f;
					p->junction_t += ts * some_value;
					v3 new_position = Math::QuadraticCurve(p->drift_points, p->junction_t);
					v3 direction = glm::normalize(new_position - p->position);


					v2 dir = glm::normalize((v2)direction);
					f32 yaw = glm::acos(dir.x) * ((float)(dir.y > 0.0f) * 2.0f - 1.0f);
					p->object->SetTransform(p->position, v3{ 0.0f, 0.0f, yaw + glm::radians(180.0f) });
					p->position = new_position;

					p->object->SetTransform(new_position);
					if (p->junction_t >= 1.0f)
					{
						p->junction_t = 0.0f;
						p->in_junction = false;
					}
				}
				else if (p->heading_to_a_building)
				{
					v3 ab = p->target - p->position;
					float leftLenght = glm::length(ab);
					v3 unit = ab / leftLenght;
					float journeyLength = ts * p->speed;
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
					float leftLenght = glm::length(ab);
					v3 unit = ab / leftLenght;
					float journeyLength = ts * p->speed;
					if (journeyLength < leftLenght)
					{
						p->position = p->position + unit * journeyLength;
						p->object->SetTransform(p->position);
					}
					else
					{
						RoadSegment& current_road = segments[p->road_segment];
						p->position = p->target;
						p->object->SetTransform(p->position);
						float length_of_the_road_prefab = app->road_types[current_road.type].road_length;
						std::vector<v3>& curve_samples = current_road.curve_samples;
						std::vector<f32>& curve_t_samples = current_road.curve_t_samples;
						if (p->path.size() == 1)
						{
							if ((p->from_start && p->t >= p->target_building->snappedT) ||
								(!p->from_start && p->t <= p->target_building->snappedT))
							{
								p->target = p->target_building->position;
								p->heading_to_a_building = true;
							}
							else
							{
								p->t_index += (p->from_start ? +1 : -1);
								p->t = curve_t_samples[p->t_index];
								p->target = curve_samples[p->t_index];

								v3 direction = glm::normalize(p->target - p->position);

								v2 dir = glm::normalize((v2)direction);
								f32 yaw = glm::acos(dir.x) * ((float)(dir.y > 0.0f) * 2.0f - 1.0f);
								p->object->SetTransform(p->position, v3{ 0.0f, 0.0f, yaw + glm::radians(180.0f) });
							}
						}
						else
						{
							if ((curve_samples.size() - 2 <= p->t_index && p->from_start) || (1 >= p->t_index && !p->from_start))
							{
								//////new road
								if (nodes[p->path[1]].roadSegments.size() > 1)
								{
									p->drift_points[0] = p->position;
									p->drift_points[1] = nodes[p->path[1]].position;

									RoadSegment& old_rs = segments[p->path[0]];
									RoadSegment& new_rs = segments[p->path[2]];

									old_rs.peoples.erase(std::find(old_rs.peoples.begin(), old_rs.peoples.end(), p));
									p->road_segment = p->path[2];
									new_rs.peoples.push_back(p);

									std::vector<v3>& samples2 = new_rs.curve_samples;

									if (p->path[1] == new_rs.StartNode)
									{
										p->t_index = 0;
										p->target = samples2[1];
										p->from_start = true;
										p->drift_points[2] = new_rs.GetStartPosition();
									}
									else
									{
										p->t_index = samples2.size() - 1;
										p->target = samples2[samples2.size() - 2];
										p->from_start = false;
										p->drift_points[2] = new_rs.GetEndPosition();
									}
									p->t = 0.0f;
									p->in_junction = true;
									p->path.erase(p->path.begin(), std::next(p->path.begin(), 2));
								}
								else
								{
									if (!p->from_start)
									{
										p->t_index = 0;
										p->t = 0.0f;
										p->target = curve_samples[1];
										p->from_start = true;
									}
									else
									{
										p->t_index = curve_samples.size();
										p->t = 1.0f;
										p->target = curve_samples[curve_samples.size() - 2];
										p->from_start = false;
									}
									p->path.erase(p->path.begin(), std::next(p->path.begin(), 2));
								}
							}
							else
							{
								v3 oldTarget = p->target;
								p->t_index += (p->from_start ? +1 : -1);
								p->target = curve_samples[p->t_index];
								p->t = curve_t_samples[p->t_index];

								v3 d1rection = glm::normalize(p->target - oldTarget);

								v2 dir = glm::normalize((v2)d1rection);
								f32 yaw = glm::acos(dir.x) * ((float)(dir.y > 0.0f) * 2.0f - 1.0f);

								p->object->SetTransform(p->position, v3{ 0.0f, 0.0f, yaw + glm::radians(180.0f) });
							}
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
		auto& segments = game->m_RoadManager.m_Segments;

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
		auto it = std::find(people.begin(), people.end(), p);
		if (it != people.end())
			people.erase(it);
		else
			assert(false);

		delete p;
	}
}