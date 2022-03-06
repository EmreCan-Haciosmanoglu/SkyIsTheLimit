#include "canpch.h"
#include "GameScene.h"

#include "GameApp.h"
#include "Helper.h"
#include "Can/Math.h"

#include "Types/RoadNode.h"
#include "Types/Car.h"
#include "Types/Tree.h"
#include "Building.h"

namespace Can
{
	GameScene* GameScene::ActiveGameScene = nullptr;

	GameScene::GameScene(GameApp* application, std::string& save_name)
		: MainApplication(application)
		, save_name(save_name)
		, m_Terrain(new Object(MainApplication->terrainPrefab))
		, m_RoadManager(this)
		, m_TreeManager(this)
		, m_BuildingManager(this)
		, m_CarManager(this)
		, camera_controller(
			45.0f,
			16.0f / 9.0f,
			0.1f,
			1000.0f,
			v3{ 2.0f, -3.0f, 5.0f },
			v3{ 0.0f, -45.0f, 90.0f }
		)
	{
		m_Terrain->owns_prefab = true;
		m_LightDirection = glm::normalize(m_LightDirection);
		m_ShadowMapMasterRenderer = new ShadowMapMasterRenderer(&camera_controller);

	}
	GameScene::~GameScene()
	{
		delete m_Terrain;
	}
	void GameScene::OnAttach()
	{
		ActiveGameScene = this;
	}
	void GameScene::OnDetach()
	{
	}
	bool GameScene::OnUpdate(TimeStep ts)
	{
		camera_controller.on_update(ts);

		RenderCommand::SetClearColor({ 0.9f, 0.9f, 0.9f, 1.0f });
		RenderCommand::Clear();

		v3 camPos = camera_controller.camera.position;
		v3 forward = GetRayCastedFromScreen();

		v3 I = Helper::RayPlaneIntersection(
			camPos,
			forward,
			v3{ 0.0f, 0.0f, 0.0f },
			v3{ 0.0f, 0.0f, 1.0f }
		);

		if (isnan(I.x) == false)
		{
			switch (e_ConstructionMode)
			{
			case ConstructionMode::Road:
				m_RoadManager.OnUpdate(I, ts);
				break;
			case ConstructionMode::Building:
				m_BuildingManager.OnUpdate(I, camPos, forward);
				break;
			case ConstructionMode::Tree:
				m_TreeManager.OnUpdate(I, camPos, forward);
				break;
			case ConstructionMode::Car:
				m_CarManager.OnUpdate(I, camPos, forward);
				break;
			case ConstructionMode::None:
				break;
			}
		}

		for (uint8_t i = 0; i < (uint8_t)e_SpeedMode; i++)
			MoveMe2AnotherFile(ts);


		Renderer3D::BeginScene(camera_controller.camera);
		m_ShadowMapMasterRenderer->Render(m_LightDirection);

		Renderer3D::DrawObjects(
			m_ShadowMapMasterRenderer->GetLS(),
			m_ShadowMapMasterRenderer->GetShadowMap(),
			camera_controller.camera,
			m_LightPosition
		);

		Renderer3D::EndScene();
		//m_Framebuffer->Unbind();

		return false;
	}
	void GameScene::OnEvent(Event::Event& event)
	{
		camera_controller.on_event(event);
		Event::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Event::MouseButtonPressedEvent>(CAN_BIND_EVENT_FN(GameScene::OnMousePressed));
	}
	bool GameScene::OnMousePressed(Event::MouseButtonPressedEvent& event)
	{
		MouseCode button = event.GetMouseButton();
		v3 camPos = camera_controller.camera.position;
		v3 forward = GetRayCastedFromScreen();


		v3 bottomPlaneCollisionPoint = Helper::RayPlaneIntersection(
			camPos,
			forward,
			v3{ 0.0f, 0.0f, 0.0f },
			v3{ 0.0f, 0.0f, 1.0f }
		);
		v3 topPlaneCollisionPoint = Helper::RayPlaneIntersection(
			camPos,
			forward,
			v3{ 0.0f, 0.0f, 1.0f * COLOR_COUNT },
			v3{ 0.0f, 0.0f, 1.0f }
		);
		bool inside_game_zone = Helper::CheckBoundingBoxHit(
			camPos,
			forward,
			m_Terrain->prefab->boundingBoxL,
			m_Terrain->prefab->boundingBoxM
		);
		if (!inside_game_zone)
			return false;

		switch (e_ConstructionMode)
		{
		case ConstructionMode::Road:
			m_RoadManager.OnMousePressed(button);
			break;
		case ConstructionMode::Building:
			m_BuildingManager.OnMousePressed(button);
			break;
		case ConstructionMode::Tree:
			m_TreeManager.OnMousePressed(button);
			break;
		case ConstructionMode::Car:
			m_CarManager.OnMousePressed(button);
			break;
		case ConstructionMode::None:
			break;
		}
		return false;
	}
	void GameScene::SetConstructionMode(ConstructionMode mode)
	{
		if (e_ConstructionMode == mode) return;
		e_ConstructionMode = mode;
		switch (e_ConstructionMode)
		{
		case ConstructionMode::Road:
			m_TreeManager.ResetStates();
			m_BuildingManager.ResetStates();
			m_CarManager.ResetStates();
			m_RoadManager.SetConstructionMode(m_RoadManager.GetConstructionMode());
			break;
		case ConstructionMode::Building:
			m_RoadManager.ResetStates();
			m_TreeManager.ResetStates();
			m_CarManager.ResetStates();
			m_BuildingManager.SetConstructionMode(m_BuildingManager.GetConstructionMode());
			break;
		case ConstructionMode::Tree:
			m_RoadManager.ResetStates();
			m_BuildingManager.ResetStates();
			m_CarManager.ResetStates();
			m_TreeManager.SetConstructionMode(m_TreeManager.GetConstructionMode());
			break;
		case ConstructionMode::Car:
			m_RoadManager.ResetStates();
			m_BuildingManager.ResetStates();
			m_TreeManager.ResetStates();
			m_CarManager.SetConstructionMode(m_CarManager.GetConstructionMode());
			break;
		case ConstructionMode::None:
			m_RoadManager.ResetStates();
			m_TreeManager.ResetStates();
			m_BuildingManager.ResetStates();
			m_CarManager.ResetStates();
			break;
		default:
			break;
		}
	}
	void GameScene::SetSpeedMode(SpeedMode mode)
	{
		// More Stuff???
		e_SpeedMode = mode;
	}
	void GameScene::load_the_game()
	{
		namespace fs = std::filesystem;
		std::string s = fs::current_path().string();
		std::string path = s.append("\\");
		path.append(save_name).append(".csf");
		FILE* read_file = fopen(path.c_str(), "rb");
		if (read_file == NULL) return;

		auto& road_types = MainApplication->road_types;
		//RoadManager
		fread(&m_RoadManager.snapFlags, sizeof(u8), 1, read_file);
		fread(&m_RoadManager.restrictionFlags, sizeof(u8), 1, read_file);
		auto& nodes = m_RoadManager.m_Nodes;
		u64 node_count;
		fread(&node_count, sizeof(u64), 1, read_file);
		nodes.reserve(node_count);
		for (u64 i = 0; i < node_count; i++)
		{
			nodes.push_back(RoadNode({}, v3{ 0.0f, 0.0f, 0.0f }, 0));
			fread(&nodes[i].position, sizeof(f32), 3, read_file);
			fread(&nodes[i].index, sizeof(u64), 1, read_file);
			fread(&nodes[i].elevation_type, sizeof(s8), 1, read_file);
		}
		auto& segments = m_RoadManager.m_Segments;
		u64 segment_count;
		fread(&segment_count, sizeof(u64), 1, read_file);
		segments.reserve(segment_count);
		for (u64 i = 0; i < segment_count; i++)
		{
			u8 road_type;
			fread(&road_type, sizeof(u8), 1, read_file);
			u64 start_node, end_node;
			std::array<v3, 4> curve_points;
			s8 elevation_type;
			// an array of indices to  building objects
			// an array of indices to  Car objects
			fread(&start_node, sizeof(u64), 1, read_file);
			fread(&end_node, sizeof(u64), 1, read_file);
			fread(&curve_points, sizeof(f32), 3 * 4, read_file);
			fread(&elevation_type, sizeof(s8), 1, read_file);
			segments.push_back(RoadSegment{
				road_type,
				curve_points,
				elevation_type
				});
			segments[i].StartNode = start_node;
			segments[i].EndNode = end_node;
			nodes[start_node].AddRoadSegment({ i });
			nodes[end_node].AddRoadSegment({ i });
			segments[i].ReConstruct();
		}
		///////////////////////////////////////////////////

		//TreeManager
		fread(m_TreeManager.restrictions.data(), sizeof(bool), 1, read_file);
		auto& trees = m_TreeManager.m_Trees;
		u64 tree_count;
		fread(&tree_count, sizeof(u64), 1, read_file);
		m_TreeManager.m_Trees.reserve(tree_count);
		for (u64 i = 0; i < tree_count; i++)
		{
			u64 type;
			v3 pos, rot, scale;
			fread(&type, sizeof(u64), 1, read_file);
			fread(&pos, sizeof(f32), 3, read_file);
			fread(&rot, sizeof(f32), 3, read_file);
			fread(&scale, sizeof(f32), 3, read_file);

			Object* tree = new Object(MainApplication->trees[type], pos, rot, scale);
			trees.push_back(new Tree{ type, tree });
		}
		///////////////////////////////////////////////////


		fclose(read_file);
	}
	void GameScene::save_the_game()
	{
		FILE* save_file = fopen(std::string(save_name).append(".csf").c_str(), "wb");

		//RoadManager
		fwrite(&m_RoadManager.snapFlags, sizeof(u8), 1, save_file);
		fwrite(&m_RoadManager.restrictionFlags, sizeof(u8), 1, save_file);
		auto& nodes = m_RoadManager.m_Nodes;
		u64 node_count = nodes.size();
		fwrite(&node_count, sizeof(u64), 1, save_file);
		for (u64 i = 0; i < node_count; i++)
		{
			fwrite(&nodes[i].position, sizeof(f32), 3, save_file);
			fwrite(&nodes[i].index, sizeof(u64), 1, save_file);
			fwrite(&nodes[i].elevation_type, sizeof(s8), 1, save_file);
		}
		auto& segments = m_RoadManager.m_Segments;
		u64 segment_count = segments.size();
		fwrite(&segment_count, sizeof(u64), 1, save_file);
		for (u64 i = 0; i < segment_count; i++)
		{
			fwrite(&segments[i].type, sizeof(u8), 1, save_file);
			// an array of indices to  building objects
			// an array of indices to  Car objects
			fwrite(&segments[i].StartNode, sizeof(u64), 1, save_file);
			fwrite(&segments[i].EndNode, sizeof(u64), 1, save_file);
			fwrite(&segments[i].CurvePoints, sizeof(f32), 3 * 4, save_file);
			fwrite(&segments[i].elevation_type, sizeof(s8), 1, save_file);
		}
		///////////////////////////////////////////////////

		//TreeManager
		fwrite(m_TreeManager.restrictions.data(), sizeof(bool), 1, save_file);
		auto& trees = m_TreeManager.m_Trees;
		u64 tree_count = trees.size();
		fwrite(&tree_count, sizeof(u64), 1, save_file);
		for (u64 i = 0; i < tree_count; i++)
		{
			fwrite(&trees[i]->type, sizeof(u64), 1, save_file);
			fwrite(&trees[i]->object->position, sizeof(f32), 3, save_file);
			fwrite(&trees[i]->object->rotation, sizeof(f32), 3, save_file);
			fwrite(&trees[i]->object->scale, sizeof(f32), 3, save_file);
		}
		///////////////////////////////////////////////////

		//BuildingManager
		fwrite(m_BuildingManager.snapOptions.data(), sizeof(bool), 2, save_file);
		fwrite(m_BuildingManager.restrictions.data(), sizeof(bool), 2, save_file);
		auto& buildings = m_BuildingManager.m_Buildings;
		u64 building_count = buildings.size();
		fwrite(&building_count, sizeof(u64), 1, save_file);
		for (u64 i = 0; i < building_count; i++)
		{
			fwrite(&buildings[i]->type, sizeof(u64), 1, save_file);
			fwrite(&buildings[i]->connectedRoadSegment, sizeof(s64), 1, save_file);
			fwrite(&buildings[i]->snappedT, sizeof(f32), 1, save_file);
			fwrite(&buildings[i]->position, sizeof(f32), 3, save_file);
		}
		///////////////////////////////////////////////////

		//CarManager
		auto& cars = m_CarManager.m_Cars;
		u64 car_count = cars.size();
		fwrite(&car_count, sizeof(u64), 1, save_file);
		for (u64 i = 0; i < car_count; i++)
		{
			fwrite(&cars[i]->roadSegment, sizeof(s64), 1, save_file);
			fwrite(&cars[i]->t_index, sizeof(u64), 1, save_file);
			fwrite(&cars[i]->speed, sizeof(f32), 1, save_file);
			fwrite(&cars[i]->t, sizeof(f32), 1, save_file);
			fwrite(cars[i]->driftpoints.data(), sizeof(f32), 3 * 3, save_file);
			fwrite(&cars[i]->position, sizeof(f32), 3, save_file);
			fwrite(&cars[i]->fromStart, sizeof(bool), 1, save_file);
			fwrite(&cars[i]->inJunction, sizeof(bool), 1, save_file);
		}
		///////////////////////////////////////////////////

		fclose(save_file);
		printf("Game is saved.\n");
	}
	v3 GameScene::GetRayCastedFromScreen()
	{
		auto [mouseX, mouseY] = Can::Input::get_mouse_pos_float();
		Application& app = Application::Get();
		float w = (float)(app.GetWindow().GetWidth());
		float h = (float)(app.GetWindow().GetHeight());

		auto camera = camera_controller.camera;
		v3 camPos = camera.position;
		v3 camRot = camera.rotation;

		float fovyX = camera.field_of_view_angle;
		float xoffSet = glm::degrees(glm::atan(glm::tan(glm::radians(fovyX)) * (((w / 2.0f) - mouseX) / (w / 2.0f))));
		float yoffSet = glm::degrees(glm::atan(((h - 2.0f * mouseY) * glm::sin(glm::radians(xoffSet))) / (w - 2.0f * mouseX)));

		v2 offsetDegrees = {
			xoffSet,
			yoffSet
		};

		v3 forward = camera.forward;
		v3 up = camera.up;
		v3 right = camera.right;

		forward = glm::rotate(forward, glm::radians(offsetDegrees.x), up);
		right = glm::rotate(right, glm::radians(offsetDegrees.x), up);
		forward = glm::rotate(forward, glm::radians(offsetDegrees.y), right);
		return forward;
	}
	void GameScene::MoveMe2AnotherFile(float ts)
	{
		auto& road_types = MainApplication->road_types;
		auto& cars = m_CarManager.GetCars();
		for (Car* car : cars)
		{
			v3 targeT = car->target;
			v3 pos = car->position;
			v3 ab = targeT - pos;
			v3 unit = glm::normalize(ab);
			float journeyLength = ts * car->speed;
			float leftLenght = glm::length(ab);
			RoadSegment& road = m_RoadManager.m_Segments[car->roadSegment];

			if (car->inJunction)
			{
				car->t += ts / 1.5f;
				v3 driftPos = Math::QuadraticCurve(car->driftpoints, car->t);
				v3 d1rection = glm::normalize(driftPos - car->position);
				car->position = driftPos;


				v2 dir = glm::normalize((v2)d1rection);
				f32 yaw = glm::acos(dir.x) * ((float)(dir.y > 0.0f) * 2.0f - 1.0f);
				v3 dirR = glm::rotateZ(d1rection, -yaw);
				dir = glm::normalize(v2{ dirR.x, dirR.z });
				f32 pitch = glm::acos(std::abs(dir.x)) * ((float)(dir.y < 0.0f) * 2.0f - 1.0f);


				car->object->SetTransform(car->position, v3{ 0.0f, -pitch, yaw + glm::radians(180.0f) });
				if (car->t >= 1.0f)
				{
					car->inJunction = false;
				}

			}
			else
			{
				if (journeyLength < leftLenght)
				{
					car->position = car->position + unit * journeyLength;
					car->object->SetTransform(car->position);
				}
				else
				{
					car->position = car->target;
					car->object->SetTransform(car->position);
					std::vector<float> ts{ 0 };

					float lengthRoad = road_types[road.type].road_length;
					std::vector<v3> samples = Math::GetCubicCurveSamples(road.GetCurvePoints(), lengthRoad, ts);

					if ((samples.size() - 2 == car->t_index && car->fromStart) || (1 == car->t_index && !car->fromStart))
					{
						//////new road
						u64 nodeIndex = car->fromStart ? road.EndNode : road.StartNode;
						RoadNode& node = m_RoadManager.m_Nodes[nodeIndex];
						if (node.roadSegments.size() > 1)
						{
							car->driftpoints[0] = car->position;
							car->driftpoints[1] = node.position;

							std::vector<u64>& roads = node.roadSegments;
							int newRoadIndex = Utility::Random::Integer((int)roads.size());
							RoadSegment& rs = m_RoadManager.m_Segments[car->roadSegment];

							while (car->roadSegment == roads[newRoadIndex])
							{
								newRoadIndex = Utility::Random::Integer((int)roads.size());
							}

							rs.Cars.erase(std::find(rs.Cars.begin(), rs.Cars.end(), car));
							car->roadSegment = roads[newRoadIndex];

							m_RoadManager.m_Segments[car->roadSegment].Cars.push_back(car);
							std::vector<float> ts2{ 0 };
							float lengthRoad2 = road_types[m_RoadManager.m_Segments[car->roadSegment].type].road_length;
							std::vector<v3> samples2 = Math::GetCubicCurveSamples(m_RoadManager.m_Segments[car->roadSegment].GetCurvePoints(), lengthRoad2, ts2);

							if (nodeIndex == m_RoadManager.m_Segments[car->roadSegment].StartNode)
							{
								car->t_index = 0;
								car->target = samples2[1];
								car->fromStart = true;
								car->driftpoints[2] = m_RoadManager.m_Segments[car->roadSegment].GetStartPosition();
							}
							else
							{
								car->t_index = samples2.size();
								car->target = samples2[samples2.size() - 1];
								car->fromStart = false;
								car->driftpoints[2] = m_RoadManager.m_Segments[car->roadSegment].GetEndPosition();
							}
							car->t = 0;
							car->inJunction = true;

						}
						else
						{
							if (!car->fromStart)
							{
								car->t_index = 0;
								car->target = samples[1];
								car->fromStart = true;
							}
							else
							{
								car->t_index = samples.size();
								car->target = samples[samples.size() - 1];
								car->fromStart = false;
							}
						}
					}
					else
					{
						v3 oldTarget = car->target;
						car->target = samples[car->t_index + (car->fromStart ? +1 : -1)];
						car->t_index += (car->fromStart ? +1 : -1);

						v3 d1rection = glm::normalize(car->target - oldTarget);

						v2 dir = glm::normalize((v2)d1rection);
						f32 yaw = glm::acos(dir.x) * ((float)(dir.y > 0.0f) * 2.0f - 1.0f);
						v3 dirR = glm::rotateZ(d1rection, -yaw);
						dir = glm::normalize(v2{ dirR.x, dirR.z });
						f32 pitch = glm::acos(std::abs(dir.x)) * ((float)(dir.y < 0.0f) * 2.0f - 1.0f);

						car->object->SetTransform(car->position, v3{ 0.0f, -pitch, yaw + glm::radians(180.0f) });
					}

				}
			}

		}
	}
}
