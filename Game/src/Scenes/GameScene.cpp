#include "canpch.h"
#include "GameScene.h"

#include "GameApp.h"
#include "Helper.h"
#include "Can/Math.h"

#include "Types/RoadNode.h"
#include "Types/Car.h"
#include "Types/Tree.h"
#include "Types/Person.h"
#include "Building.h"
#include "Types/Transition.h"

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
		, m_PersonManager(this)
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
			case ConstructionMode::None:
				break;
			}
		}


		for (uint8_t i = 0; i < (uint8_t)e_SpeedMode; i++)
		{
			m_PersonManager.Update(ts);
		}

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
			m_RoadManager.SetConstructionMode(m_RoadManager.GetConstructionMode());
			break;
		case ConstructionMode::Building:
			m_RoadManager.ResetStates();
			m_TreeManager.ResetStates();
			m_BuildingManager.SetConstructionMode(m_BuildingManager.GetConstructionMode());
			break;
		case ConstructionMode::Tree:
			m_RoadManager.ResetStates();
			m_BuildingManager.ResetStates();
			m_TreeManager.SetConstructionMode(m_TreeManager.GetConstructionMode());
			break;
		case ConstructionMode::None:
			m_RoadManager.ResetStates();
			m_TreeManager.ResetStates();
			m_BuildingManager.ResetStates();
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
		auto& road_nodes = m_RoadManager.road_nodes;
		u64 capacity;
		fread(&capacity, sizeof(u64), 1, read_file);
		array_resize(&road_nodes, capacity);
		for (u64 i = 0; i < capacity; i++)
		{
			fread(&road_nodes.values[i].valid, sizeof(bool), 1, read_file);
			if (road_nodes.values[i].valid == false) continue;
			auto& road_node = road_nodes[i];
			fread(&road_node.position, sizeof(f32), 3, read_file);
			road_node.index = i;
			fread(&road_node.elevation_type, sizeof(s8), 1, read_file);
			road_nodes.size++;
		}
		auto& road_segments = m_RoadManager.road_segments;
		fread(&capacity, sizeof(u64), 1, read_file);
		array_resize(&road_segments, capacity);
		for (u64 i = 0; i < capacity; i++)
		{
			fread(&road_segments.values[i].valid, sizeof(bool), 1, read_file);
			if (road_segments.values[i].valid == false) continue;
			auto& segment = road_segments[i];
			fread(&segment.type, sizeof(u8), 1, read_file);
			fread(&segment.StartNode, sizeof(u64), 1, read_file);
			fread(&segment.EndNode, sizeof(u64), 1, read_file);
			fread(&segment.CurvePoints, sizeof(f32), 3 * 4, read_file);
			fread(&segment.elevation_type, sizeof(s8), 1, read_file);
			segment.CalcRotsAndDirs();
			road_segments.size++;

			road_nodes[segment.StartNode].roadSegments.push_back(i);
			road_nodes[segment.EndNode].roadSegments.push_back(i);
		}
		for (u64 i = 0; i < road_nodes.capacity; i++)
		{
			if (road_nodes.values[i].valid == false) continue;
			road_nodes[i].Reconstruct();
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

			// If we want it to be same all the time
			fread(&rot, sizeof(f32), 3, read_file);
			fread(&scale, sizeof(f32), 3, read_file);

			Object* tree = new Object(MainApplication->trees[type], pos, rot, scale);
			trees.push_back(new Tree{ type, tree });
		}
		///////////////////////////////////////////////////

		//BuildingManager
		fread(m_BuildingManager.snapOptions.data(), sizeof(bool), 2, read_file);
		fread(m_BuildingManager.restrictions.data(), sizeof(bool), 2, read_file);
		auto& buildings = m_BuildingManager.m_Buildings;
		auto& home_buildings = m_BuildingManager.m_HomeBuildings;
		auto& work_buildings = m_BuildingManager.m_WorkBuildings;
		u64 building_count;
		fread(&building_count, sizeof(u64), 1, read_file);
		buildings.reserve(building_count);
		for (u64 i = 0; i < building_count; i++)
		{
			u64 type;
			s64 connected_road_segment;
			u64 snapped_t_index;
			f32 snapped_t;
			u16 capacity;
			bool is_home = false;
			bool snapped_to_right;
			v3 position, rotation;// calculate it from snapped_t?
			fread(&type, sizeof(u64), 1, read_file);
			fread(&connected_road_segment, sizeof(s64), 1, read_file);
			fread(&snapped_t_index, sizeof(u64), 1, read_file);
			fread(&snapped_t, sizeof(f32), 1, read_file);
			fread(&capacity, sizeof(u16), 1, read_file);
			fread(&is_home, sizeof(bool), 1, read_file);
			fread(&snapped_to_right, sizeof(bool), 1, read_file);
			fread(&position, sizeof(f32), 3, read_file);
			fread(&rotation, sizeof(f32), 3, read_file);
			Building* building = new Building(
				MainApplication->buildings[type],
				connected_road_segment,
				snapped_t_index,
				snapped_t,
				position,
				rotation
			);
			building->type = type;
			building->capacity = capacity;
			building->is_home = is_home;
			building->snapped_to_right = snapped_to_right;
			buildings.push_back(building);
			if (is_home)
				home_buildings.push_back(building);
			else
				work_buildings.push_back(building);
			road_segments[connected_road_segment].Buildings.push_back(building);
		}
		///////////////////////////////////////////////////

		//CarManager
		auto& cars = m_CarManager.m_Cars;
		u64 car_count;
		fread(&car_count, sizeof(u64), 1, read_file);
		cars.reserve(car_count);
		for (u64 i = 0; i < car_count; i++)
		{
			u64 type;
			f32 speed_in_kmh;
			v3 position, rotation;
			fread(&type, sizeof(u64), 1, read_file);
			fread(&speed_in_kmh, sizeof(f32), 1, read_file);
			fread(&position, sizeof(f32), 3, read_file);
			fread(&rotation, sizeof(f32), 3, read_file);
			Car* car = new Car(
				MainApplication->cars[type],
				type,
				speed_in_kmh
			);
			car->object->SetTransform(position, rotation);
			cars.push_back(car);
		}
		///////////////////////////////////////////////////

		/*PersonManager*/ {
			auto& people = m_PersonManager.m_People;
			u64 people_count;
			fread(&people_count, sizeof(u64), 1, read_file);
			people.reserve(people_count);
			for (u64 i = 0; i < people_count; i++)
			{
				u64 path_count;
				u64 path_end_building_index, path_start_building_index;
				u64 type;
				u64 first_name_char_count;
				u64 middle_name_char_count;
				u64 last_name_char_count;
				u64 home_index, work_index, car_index;
				Person* person = new Person();
				fread(&type, sizeof(u64), 1, read_file);
				person->object = new Object(MainApplication->people[type]);
				fread(&person->object->enabled, sizeof(bool), 1, read_file);
				fread(&person->road_segment, sizeof(s64), 1, read_file);
				if (person->road_segment != -1)
					road_segments[person->road_segment].people.push_back(person);
				fread(&person->road_node, sizeof(s64), 1, read_file);
				if (person->road_node != -1)
					road_nodes[person->road_node].people.push_back(person);
				fread(&person->speed_in_kmh, sizeof(f32), 1, read_file);
				fread(&person->position, sizeof(f32), 3, read_file);
				person->object->SetTransform(person->position);
				fread(&person->target, sizeof(f32), 3, read_file);
				// set direction for target-position
				fread(&person->status, sizeof(PersonStatus), 1, read_file);
				fread(&path_count, sizeof(u64), 1, read_file);
				person->path.reserve(path_count);
				if (person->status == PersonStatus::Walking)
				{
					u64 j = 0;
					if (path_count % 2 == 1)
					{
						auto rst = new RS_Transition_For_Walking();
						fread(&rst->at_path_array_index, sizeof(u64), 1, read_file);
						fread(&rst->road_segment_index, sizeof(u64), 1, read_file);
						fread(&rst->from_start, sizeof(bool), 1, read_file);
						fread(&rst->from_right, sizeof(bool), 1, read_file);
						person->path.push_back(rst);
						j = 1;
					}
					for (; j < path_count; j++)
					{
						auto rnt = new RN_Transition_For_Walking();
						fread(&rnt->from_road_segments_array_index, sizeof(u64), 1, read_file);
						fread(&rnt->to_road_segments_array_index, sizeof(u64), 1, read_file);
						fread(&rnt->sub_index, sizeof(u64), 1, read_file);
						fread(&rnt->road_node_index, sizeof(u64), 1, read_file);
						fread(&rnt->accending, sizeof(bool), 1, read_file);
						person->path.push_back(rnt);
						j++;
						auto rst = new RS_Transition_For_Walking();
						fread(&rst->at_path_array_index, sizeof(u64), 1, read_file);
						fread(&rst->road_segment_index, sizeof(u64), 1, read_file);
						fread(&rst->from_start, sizeof(bool), 1, read_file);
						fread(&rst->from_right, sizeof(bool), 1, read_file);
						person->path.push_back(rst);
					}
				}
				else if (person->status == PersonStatus::Driving)
				{
					for (u64 j = 0; j < path_count; j++)
					{
						auto td = new RS_Transition_For_Driving();
						fread(&td->road_segment_index, sizeof(u64), 1, read_file);
						fread(&td->next_road_node_index, sizeof(s64), 1, read_file);
						u64 points_count = 0;
						fread(&points_count, sizeof(u64), 1, read_file);
						td->points_stack.reserve(points_count);
						for (u64 k = 0; k < points_count; k++)
						{
							v3 point{};
							fread(&point, sizeof(f32), 3, read_file);
							td->points_stack.push_back(point);
						}
						fread(&td->lane_index, sizeof(u32), 1, read_file);
						person->path.push_back(td);
					}
				}
				fread(&path_end_building_index, sizeof(s64), 1, read_file);
				if (path_end_building_index != -1)
					person->path_end_building = buildings[path_end_building_index];
				fread(&path_start_building_index, sizeof(s64), 1, read_file);
				if (path_start_building_index != -1)
					person->path_start_building = buildings[path_start_building_index];
				fread(&person->from_right, sizeof(bool), 1, read_file);
				fread(&person->heading_to_a_building_or_parking, sizeof(bool), 1, read_file);
				fread(&person->heading_to_a_car, sizeof(bool), 1, read_file);
				fread(&person->time_left, sizeof(f32), 1, read_file);

				fread(&first_name_char_count, sizeof(u64), 1, read_file);
				char* first_name = (char*)malloc(first_name_char_count + 1);
				fread(first_name, sizeof(char), first_name_char_count, read_file);
				first_name[first_name_char_count] = '\0';
				person->firstName = std::string(first_name);

				fread(&middle_name_char_count, sizeof(u64), 1, read_file);
				char* middle_name = (char*)malloc(middle_name_char_count + 1);
				fread(middle_name, sizeof(char), middle_name_char_count, read_file);
				middle_name[middle_name_char_count] = '\0';
				person->midName = std::string(middle_name);

				fread(&last_name_char_count, sizeof(u64), 1, read_file);
				char* last_name = (char*)malloc(last_name_char_count + 1);
				fread(last_name, sizeof(char), last_name_char_count, read_file);
				last_name[last_name_char_count] = '\0';
				person->surName = std::string(last_name);

				fread(&home_index, sizeof(s64), 1, read_file);
				if (home_index != -1)
				{
					person->home = buildings[home_index];
					person->home->people.push_back(person);
				}
				fread(&work_index, sizeof(s64), 1, read_file);
				if (work_index != -1)
				{
					person->work = buildings[work_index];
					person->work->people.push_back(person);
				}
				fread(&car_index, sizeof(s64), 1, read_file);
				if (car_index != -1)
				{
					person->car = cars[car_index];
					person->car->owner = person;
				}

				people.push_back(person);
			}
		}

		//Camera
		auto& camera = camera_controller.camera;
		v3 position, rotation;
		fread(&position, sizeof(f32), 3, read_file);
		fread(&rotation, sizeof(f32), 3, read_file);
		camera.set_position(position);
		camera.set_rotation(rotation);
		///////////////////////////////////////////////////


		fclose(read_file);
	}
	void GameScene::save_the_game()
	{
		FILE* save_file = fopen(std::string(save_name).append(".csf").c_str(), "wb");
		auto& road_nodes = m_RoadManager.road_nodes;
		auto& road_segments = m_RoadManager.road_segments;
		auto& trees = m_TreeManager.m_Trees;
		auto& buildings = m_BuildingManager.m_Buildings;
		auto& cars = m_CarManager.m_Cars;

		/*Road Manager*/ {
			fwrite(&m_RoadManager.snapFlags, sizeof(u8), 1, save_file);
			fwrite(&m_RoadManager.restrictionFlags, sizeof(u8), 1, save_file);
			u64 capacity = road_nodes.capacity;
			fwrite(&capacity, sizeof(u64), 1, save_file);
			for (u64 i = 0; i < capacity; i++)
			{
				fwrite(&road_nodes.values[i].valid, sizeof(bool), 1, save_file);
				if (road_nodes.values[i].valid == false) continue;
				fwrite(&road_nodes[i].position, sizeof(f32), 3, save_file);
				fwrite(&road_nodes[i].elevation_type, sizeof(s8), 1, save_file);
			}
			capacity = road_segments.capacity;
			fwrite(&capacity, sizeof(u64), 1, save_file);
			for (u64 i = 0; i < capacity; i++)
			{
				fwrite(&road_segments.values[i].valid, sizeof(bool), 1, save_file);
				if (road_segments.values[i].valid == false) continue;
				fwrite(&road_segments[i].type, sizeof(u8), 1, save_file);
				// an array of indices to  building objects
				// an array of indices to  Car objects
				fwrite(&road_segments[i].StartNode, sizeof(u64), 1, save_file);
				fwrite(&road_segments[i].EndNode, sizeof(u64), 1, save_file);
				fwrite(&road_segments[i].CurvePoints, sizeof(f32), 3 * 4, save_file);
				fwrite(&road_segments[i].elevation_type, sizeof(s8), 1, save_file);
			}
		}
		/*Tree Manager*/ {
			fwrite(m_TreeManager.restrictions.data(), sizeof(bool), 1, save_file);
			u64 tree_count = trees.size();
			fwrite(&tree_count, sizeof(u64), 1, save_file);
			for (u64 i = 0; i < tree_count; i++)
			{
				fwrite(&trees[i]->type, sizeof(u64), 1, save_file);
				fwrite(&trees[i]->object->position, sizeof(f32), 3, save_file);
				fwrite(&trees[i]->object->rotation, sizeof(f32), 3, save_file);
				fwrite(&trees[i]->object->scale, sizeof(f32), 3, save_file);
			}
		}
		/*Building Manager*/ {
			fwrite(m_BuildingManager.snapOptions.data(), sizeof(bool), 2, save_file);
			fwrite(m_BuildingManager.restrictions.data(), sizeof(bool), 2, save_file);
			u64 building_count = buildings.size();
			fwrite(&building_count, sizeof(u64), 1, save_file);
			for (u64 i = 0; i < building_count; i++)
			{
				fwrite(&buildings[i]->type, sizeof(u64), 1, save_file);
				fwrite(&buildings[i]->connectedRoadSegment, sizeof(s64), 1, save_file);
				fwrite(&buildings[i]->snapped_t_index, sizeof(u64), 1, save_file);
				fwrite(&buildings[i]->snapped_t, sizeof(f32), 1, save_file);
				fwrite(&buildings[i]->capacity, sizeof(u16), 1, save_file);
				fwrite(&buildings[i]->is_home, sizeof(bool), 1, save_file);
				fwrite(&buildings[i]->snapped_to_right, sizeof(bool), 1, save_file);
				fwrite(&buildings[i]->object->position, sizeof(f32), 3, save_file);
				fwrite(&buildings[i]->object->rotation, sizeof(f32), 3, save_file);
			}
		}
		/*Car Manager*/ {
			u64 car_count = cars.size();
			fwrite(&car_count, sizeof(u64), 1, save_file);
			for (u64 i = 0; i < car_count; i++)
			{
				fwrite(&cars[i]->type, sizeof(u64), 1, save_file);
				fwrite(&cars[i]->speed_in_kmh, sizeof(f32), 1, save_file);
				fwrite(&cars[i]->object->position, sizeof(f32), 3, save_file);
				fwrite(&cars[i]->object->rotation, sizeof(f32), 3, save_file);
			}
		}
		/*PersonManager*/ {
			auto& people = m_PersonManager.m_People;
			u64 people_count = people.size();
			fwrite(&people_count, sizeof(u64), 1, save_file);
			for (u64 i = 0; i < people_count; i++)
			{
				Person* p = people[i];
				auto home_it = std::find(buildings.begin(), buildings.end(), p->home);

				s64 path_end_building_index = -1;
				s64 path_start_building_index = -1;
				s64 home_index = -1;
				s64 work_index = -1;
				s64 car_index = -1;
				u64 first_name_char_count = p->firstName.size();
				u64 middle_name_char_count = p->midName.size();
				u64 last_name_char_count = p->surName.size();
				u64 path_count = p->path.size();
				if (p->path_end_building)
				{
					auto path_end_building_it = std::find(buildings.begin(), buildings.end(), p->path_end_building);
					path_end_building_index = std::distance(buildings.begin(), path_end_building_it);
				}
				if (p->path_start_building)
				{
					auto path_start_building_it = std::find(buildings.begin(), buildings.end(), p->path_start_building);
					path_start_building_index = std::distance(buildings.begin(), path_start_building_it);
				}
				if (p->home)
				{
					auto home_it = std::find(buildings.begin(), buildings.end(), p->home);
					home_index = std::distance(buildings.begin(), home_it);
				}
				if (p->work)
				{
					auto work_it = std::find(buildings.begin(), buildings.end(), p->work);
					work_index = std::distance(buildings.begin(), work_it);
				}
				if (p->car)
				{
					auto car_it = std::find(cars.begin(), cars.end(), p->car);
					car_index = std::distance(cars.begin(), car_it);
				}
				fwrite(&p->type, sizeof(u64), 1, save_file);
				fwrite(&p->object->enabled, sizeof(bool), 1, save_file);
				fwrite(&p->road_segment, sizeof(s64), 1, save_file);
				fwrite(&p->road_node, sizeof(s64), 1, save_file);
				fwrite(&p->speed_in_kmh, sizeof(f32), 1, save_file);
				fwrite(&p->position, sizeof(f32), 3, save_file);
				fwrite(&p->target, sizeof(f32), 3, save_file);
				fwrite(&p->status, sizeof(PersonStatus), 1, save_file);
				fwrite(&path_count, sizeof(u64), 1, save_file);
				if (p->status == PersonStatus::Walking)
				{
					u64 j = 0;
					if (path_count % 2 == 1)
					{
						auto rst = (RS_Transition_For_Walking*)p->path[0];
						fwrite(&rst->at_path_array_index, sizeof(u64), 1, save_file);
						fwrite(&rst->road_segment_index, sizeof(u64), 1, save_file);
						fwrite(&rst->from_start, sizeof(bool), 1, save_file);
						fwrite(&rst->from_right, sizeof(bool), 1, save_file);
						j = 1;
					}
					for (; j < path_count; j++)
					{
						auto rnt = (RN_Transition_For_Walking*)p->path[j];
						fwrite(&rnt->from_road_segments_array_index, sizeof(u64), 1, save_file);
						fwrite(&rnt->to_road_segments_array_index, sizeof(u64), 1, save_file);
						fwrite(&rnt->sub_index, sizeof(u64), 1, save_file);
						fwrite(&rnt->road_node_index, sizeof(u64), 1, save_file);
						fwrite(&rnt->accending, sizeof(bool), 1, save_file);
						j++;
						auto rst = (RS_Transition_For_Walking*)p->path[j];
						fwrite(&rst->at_path_array_index, sizeof(u64), 1, save_file);
						fwrite(&rst->road_segment_index, sizeof(u64), 1, save_file);
						fwrite(&rst->from_start, sizeof(bool), 1, save_file);
						fwrite(&rst->from_right, sizeof(bool), 1, save_file);
					}
				}
				else if (p->status == PersonStatus::Driving)
				{
					for (u64 j = 0; j < path_count; j++)
					{
						auto td = (RS_Transition_For_Driving*)p->path[j];
						fwrite(&td->road_segment_index, sizeof(u64), 1, save_file);
						fwrite(&td->next_road_node_index, sizeof(s64), 1, save_file);
						u64 points_count = td->points_stack.size();
						fwrite(&points_count, sizeof(u64), 1, save_file);
						for (v3& point : td->points_stack)
							fwrite(&point, sizeof(f32), 3, save_file);
						fwrite(&td->lane_index, sizeof(u32), 1, save_file);
					}
				}
				fwrite(&path_end_building_index, sizeof(s64), 1, save_file);
				fwrite(&path_start_building_index, sizeof(s64), 1, save_file);
				fwrite(&p->from_right, sizeof(bool), 1, save_file);
				fwrite(&p->heading_to_a_building_or_parking, sizeof(bool), 1, save_file);
				fwrite(&p->heading_to_a_car, sizeof(bool), 1, save_file);
				fwrite(&p->time_left, sizeof(f32), 1, save_file);
				fwrite(&first_name_char_count, sizeof(u64), 1, save_file);
				fwrite(p->firstName.data(), sizeof(char), first_name_char_count, save_file);
				fwrite(&middle_name_char_count, sizeof(u64), 1, save_file);
				fwrite(p->midName.data(), sizeof(char), middle_name_char_count, save_file);
				fwrite(&last_name_char_count, sizeof(u64), 1, save_file);
				fwrite(p->surName.data(), sizeof(char), last_name_char_count, save_file);
				fwrite(&home_index, sizeof(s64), 1, save_file);
				fwrite(&work_index, sizeof(s64), 1, save_file);
				fwrite(&car_index, sizeof(s64), 1, save_file);
			}
		}

		//Camera
		auto& camera = camera_controller.camera;
		fwrite(&camera.position, sizeof(f32), 3, save_file);
		fwrite(&camera.rotation, sizeof(f32), 3, save_file);
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

}
