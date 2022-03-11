#include "canpch.h"
#include "PersonManager.h"
#include "Scenes/GameScene.h"
#include "Types/RoadSegment.h"
#include "Types/RoadNode.h"
#include "Building.h"
#include "Can/Math.h"

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
					p->status = PersonStatus::Walking;
					p->object->enabled = true;
					p->position = p->home->position;
					p->object->SetTransform(p->position);
					p->road_segment = p->home->connectedRoadSegment;
					f32 t = p->home->snappedT;
					
					RoadSegment& rs = segments[p->road_segment];
					rs.peoples.push_back(p);
					std::vector<f32>& t_samples = rs.curve_t_samples;
					for (u64 i = 0; i < t_samples.size(); i++)
					{
						if (t<t_samples[i])
						{
							p->t_index = i - 1;
							p->target = rs.curve_samples[i];
							p->from_start = true;
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
					// go to home home home home
				}
			}
			else if (p->status == PersonStatus::Walking)
			{
				// A(y)yıldız
				v3 targeT = p->target;
				v3 pos = p->position;
				v3 ab = targeT - pos;
				v3 unit = glm::normalize(ab);
				float journeyLength = ts * p->speed;
				float leftLenght = glm::length(ab);
				RoadSegment& road = segments[p->road_segment];

				if (p->in_junction)
				{
					p->t += ts / 1.5f;
					v3 driftPos = Math::QuadraticCurve(p->drift_points, p->t);
					v3 d1rection = glm::normalize(driftPos - p->position);
					p->position = driftPos;


					v2 dir = glm::normalize((v2)d1rection);
					f32 yaw = glm::acos(dir.x) * ((float)(dir.y > 0.0f) * 2.0f - 1.0f);
					v3 dirR = glm::rotateZ(d1rection, -yaw);
					dir = glm::normalize(v2{ dirR.x, dirR.z });
					f32 pitch = glm::acos(std::abs(dir.x)) * ((float)(dir.y < 0.0f) * 2.0f - 1.0f);


					p->object->SetTransform(p->position, v3{ 0.0f, -pitch, yaw + glm::radians(180.0f) });
					if (p->t >= 1.0f)
					{
						p->in_junction = false;
					}

				}
				else
				{
					if (journeyLength < leftLenght)
					{
						p->position = p->position + unit * journeyLength;
						p->object->SetTransform(p->position);
					}
					else
					{
						p->position = p->target;
						p->object->SetTransform(p->position);
						
						float lengthRoad = road.type.road_length;
						std::vector<v3>& samples = road.curve_samples;

						if ((samples.size() - 2 == p->t_index && p->from_start) || (1 == p->t_index && !p->from_start))
						{
							//////new road
							u64 nodeIndex = p->from_start ? road.EndNode : road.StartNode;
							RoadNode& node = nodes[nodeIndex];
							if (node.roadSegments.size() > 1)
							{
								p->drift_points[0] = p->position;
								p->drift_points[1] = node.position;

								std::vector<u64>& roads = node.roadSegments;
								int newRoadIndex = Utility::Random::Integer((int)roads.size());
								RoadSegment& rs = segments[p->road_segment];

								while (p->road_segment == roads[newRoadIndex])
								{
									newRoadIndex = Utility::Random::Integer((int)roads.size());
								}

								rs.peoples.erase(std::find(rs.peoples.begin(), rs.peoples.end(), p));
								p->road_segment = roads[newRoadIndex];

								segments[p->road_segment].peoples.push_back(p);

								std::vector<v3>& samples2 = segments[p->road_segment].curve_samples;

								if (nodeIndex == segments[p->road_segment].StartNode)
								{
									p->t_index = 0;
									p->target = samples2[1];
									p->from_start = true;
									p->drift_points[2] = segments[p->road_segment].GetStartPosition();
								}
								else
								{
									p->t_index = samples2.size();
									p->target = samples2[samples2.size() - 1];
									p->from_start = false;
									p->drift_points[2] = segments[p->road_segment].GetEndPosition();
								}
								p->t = 0;
								p->in_junction = true;

							}
							else
							{
								if (!p->from_start)
								{
									p->t_index = 0;
									p->target = samples[1];
									p->from_start = true;
								}
								else
								{
									p->t_index = samples.size();
									p->target = samples[samples.size() - 1];
									p->from_start = false;
								}
							}
						}
						else
						{
							v3 oldTarget = p->target;
							p->target = samples[p->t_index + (p->from_start ? +1 : -1)];
							p->t_index += (p->from_start ? +1 : -1);

							v3 d1rection = glm::normalize(p->target - oldTarget);

							v2 dir = glm::normalize((v2)d1rection);
							f32 yaw = glm::acos(dir.x) * ((float)(dir.y > 0.0f) * 2.0f - 1.0f);
							v3 dirR = glm::rotateZ(d1rection, -yaw);
							dir = glm::normalize(v2{ dirR.x, dirR.z });
							f32 pitch = glm::acos(std::abs(dir.x)) * ((float)(dir.y < 0.0f) * 2.0f - 1.0f);

							p->object->SetTransform(p->position, v3{ 0.0f, -pitch, yaw + glm::radians(180.0f) });
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
}