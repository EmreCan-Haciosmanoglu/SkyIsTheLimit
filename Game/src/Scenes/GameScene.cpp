#include "canpch.h"
#include "GameScene.h"

#include "GameApp.h"
#include "Helper.h"
#include "Can/Math.h"
#include "Building.h"

#include "Types/RoadNode.h"
#include "Types/Car.h"
#include "Types/Tree.h"
#include "Types/Person.h"
#include "Types/Building_Type.h"
#include "Types/Vehicle_Type.h"
#include "Types/Road_Type.h"
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

		init_game_scene(*MainApplication, *this);
	}
	GameScene::~GameScene()
	{
		deinit_game_scene(*MainApplication, *this);
		delete m_Terrain;
	}
	void GameScene::OnAttach()
	{
		ActiveGameScene = this;
		load_game_scene(*MainApplication, *this);
	}
	void GameScene::OnDetach()
	{
		unload_game_scene(*MainApplication, *this);
	}
	bool GameScene::OnUpdate(TimeStep ts)
	{
		camera_controller.on_update(ts);

		RenderCommand::SetClearColor({ 0.9f, 0.9f, 0.9f, 1.0f });
		RenderCommand::Clear();

		v3 camPos = camera_controller.camera.position;
		v3 forward = GetRayCastedFromScreen();

		v3 I = Math::ray_plane_intersection(
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
			update_cars(ts);
			update_buildings(ts);
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


		v3 bottomPlaneCollisionPoint = Math::ray_plane_intersection(
			camPos,
			forward,
			v3{ 0.0f, 0.0f, 0.0f },
			v3{ 0.0f, 0.0f, 1.0f }
		);
		v3 topPlaneCollisionPoint = Math::ray_plane_intersection(
			camPos,
			forward,
			v3{ 0.0f, 0.0f, 1.0f * COLOR_COUNT },
			v3{ 0.0f, 0.0f, 1.0f }
		);
		bool inside_game_zone = Helper::check_if_ray_intersects_with_bounding_box(
			camPos,
			forward,
			m_Terrain->prefab->boundingBoxL + m_Terrain->position,
			m_Terrain->prefab->boundingBoxM + m_Terrain->position
		);
		if (!inside_game_zone)
			return false;

		switch (e_ConstructionMode)
		{
		case ConstructionMode::Road:
			if (m_RoadManager.m_ConstructionMode == RoadConstructionMode::None)
				if (button == MouseCode::Button0)
					does_select_object(*this);
			m_RoadManager.OnMousePressed(button);
			break;
		case ConstructionMode::Building:
			if (m_BuildingManager.m_ConstructionMode == BuildingConstructionMode::None)
				if (button == MouseCode::Button0)
					does_select_object(*this);
			m_BuildingManager.OnMousePressed(button);
			break;
		case ConstructionMode::Tree:
			if (m_TreeManager.m_ConstructionMode == TreeConstructionMode::None)
				if (button == MouseCode::Button0)
					does_select_object(*this);
			m_TreeManager.OnMousePressed(button);
			break;
		case ConstructionMode::None:
			if (button == MouseCode::Button0)
				does_select_object(*this);
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

		auto& road_nodes{ m_RoadManager.road_nodes };
		auto& road_segments{ m_RoadManager.road_segments };
		auto& trees{ m_TreeManager.m_Trees };
		auto& buildings{ m_BuildingManager.m_Buildings };
		auto& cars{ m_CarManager.m_Cars };
		auto& building_types{ MainApplication->building_types };

		/*RoadManager*/ {
			fread(&m_RoadManager.snapFlags, sizeof(u8), 1, read_file);
			fread(&m_RoadManager.restrictionFlags, sizeof(u8), 1, read_file);
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
				fread(&segment.CurvePoints, sizeof(f32), (3 * 4), read_file);
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
		}
		/*TreeManager*/ {
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
		}
		/*BuildingManager*/ {
			fread(m_BuildingManager.snapOptions.data(), sizeof(bool), 2, read_file);
			fread(m_BuildingManager.restrictions.data(), sizeof(bool), 2, read_file);

			auto& buildings_houses = m_BuildingManager.buildings_houses;
			auto& buildings_residential = m_BuildingManager.buildings_residential;
			auto& buildings_commercial = m_BuildingManager.buildings_commercial;
			auto& buildings_industrial = m_BuildingManager.buildings_industrial;
			auto& buildings_office = m_BuildingManager.buildings_office;
			auto& buildings_specials = m_BuildingManager.buildings_specials;

			u64 building_count;
			fread(&building_count, sizeof(u64), 1, read_file);
			buildings.reserve(building_count);
			for (u64 i = 0; i < building_count; i++)
			{
				Building* building = new Building();

				v3 position, rotation;// calculate it from snapped_t?
				fread(&building->type, sizeof(u64), 1, read_file);
				auto& building_type{ building_types[building->type] };

				fread(&position, sizeof(f32), 3, read_file);
				fread(&rotation, sizeof(f32), 3, read_file);
				building->object = new Object(building_type.prefab, position, rotation);

				fread(&building->connected_road_segment, sizeof(s64), 1, read_file);
				fread(&building->snapped_t_index, sizeof(u64), 1, read_file);
				fread(&building->snapped_t, sizeof(f32), 1, read_file);
				fread(&building->snapped_to_right, sizeof(bool), 1, read_file);

				fread(&building->max_health, sizeof(f32), 1, read_file);
				fread(&building->current_health, sizeof(f32), 1, read_file);
				fread(&building->electricity_need, sizeof(f32), 1, read_file);
				fread(&building->electricity_provided, sizeof(f32), 1, read_file);
				fread(&building->garbage_capacity, sizeof(f32), 1, read_file);
				fread(&building->current_garbage, sizeof(f32), 1, read_file);
				fread(&building->water_need, sizeof(f32), 1, read_file);
				fread(&building->water_provided, sizeof(f32), 1, read_file);
				fread(&building->water_waste_need, sizeof(f32), 1, read_file);
				fread(&building->water_waste_provided, sizeof(f32), 1, read_file);

				fread(&building->crime_reported, sizeof(u16), 1, read_file);

				u64 name_char_count{};
				fread(&name_char_count, sizeof(u64), 1, read_file);
				auto name{ (char*)malloc(name_char_count + 1) };
				fread(name, sizeof(char), name_char_count, read_file);
				name[name_char_count] = '\0';
				building->name = std::string(name);

				buildings.push_back(building);

				switch (building_type.group)
				{
				case Building_Group::House:
				{
					buildings_houses.push_back(building);
					break;
				}
				case Building_Group::Residential:
				{
					buildings_residential.push_back(building);
					break;
				}
				case Building_Group::Commercial:
				{
					buildings_commercial.push_back(building);
					break;
				}
				case Building_Group::Industrial:
				{
					buildings_industrial.push_back(building);
					break;
				}
				case Building_Group::Office:
				{
					buildings_office.push_back(building);
					break;
				}
				case Building_Group::Hospital:
				case Building_Group::Police_Station:
				case Building_Group::Garbage_Collection_Center:
				{
					buildings_specials.push_back(building);
					break;
				}
				default:
					assert(false, "Unimplemented Building_Group");
					break;
				}

				road_segments[building->connected_road_segment].buildings.push_back(building);
			}
		}
		/*CarManager*/ {
			u64 car_count;
			fread(&car_count, sizeof(u64), 1, read_file);
			cars.reserve(car_count);
			for (u64 i = 0; i < car_count; i++)
			{
				v3 position;
				v3 rotation;
				s64 building_index;

				Car* car{ new Car() };

				//object
				fread(&position, sizeof(f32), 3, read_file);
				fread(&rotation, sizeof(f32), 3, read_file);
				fread(&car->type, sizeof(u64), 1, read_file);
				car->object = new Object(
					MainApplication->vehicle_types[car->type].prefab,
					position,
					rotation
				);

				fread(&car->speed_in_kmh, sizeof(f32), 1, read_file);
				fread(&car->target, sizeof(f32), 3, read_file);
				fread(&car->target_park_pos, sizeof(f32), 3, read_file);
				fread(&car->heading_to_a_parking_spot, sizeof(bool), 1, read_file);
				fread(&car->road_segment, sizeof(s64), 1, read_file);
				if (car->road_segment != -1)
					road_segments[car->road_segment].vehicles.push_back(car);

				u64 path_count;
				fread(&path_count, sizeof(u64), 1, read_file);
				car->path.reserve(path_count);
				for (u64 j = 0; j < path_count; j++)
				{
					auto td = new RS_Transition_For_Vehicle();
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
					car->path.push_back(td);
				}

				fread(&building_index, sizeof(u64), 1, read_file);
				if (building_index != -1)
				{
					car->building = buildings[building_index];
					buildings[building_index]->vehicles.push_back(car);
					// this is a work car
					car->object->tintColor = v4{ 1.0f, 0.0f, 0.0f, 1.0f };
				}
				cars.push_back(car);
			}
		}
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
				u64 home_index, work_index, car_index, car_driving_index;

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
				fread(&person->heading_to_a_car, sizeof(bool), 1, read_file); // Read this before path
				fread(&path_count, sizeof(u64), 1, read_file);
				person->path.reserve(path_count);
				if (person->heading_to_a_car == false && person->status == PersonStatus::Walking)
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
				fread(&path_end_building_index, sizeof(s64), 1, read_file);
				if (path_end_building_index != -1)
					person->path_end_building = buildings[path_end_building_index];
				fread(&path_start_building_index, sizeof(s64), 1, read_file);
				if (path_start_building_index != -1)
					person->path_start_building = buildings[path_start_building_index];
				fread(&person->drove_in_work, sizeof(bool), 1, read_file);
				fread(&person->from_right, sizeof(bool), 1, read_file);
				fread(&person->heading_to_a_building, sizeof(bool), 1, read_file);
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
				fread(&car_driving_index, sizeof(s64), 1, read_file);
				if (car_driving_index != -1)
				{
					person->car_driving = cars[car_index];
					cars[car_index]->driver = person;
				}
				u8 education_level = Utility::Random::unsigned_8((u8)PersonEducationLevel::Doctorate + 1);
				person->education = (PersonEducationLevel)education_level;
				people.push_back(person);
			}
		}
		/*CameraController*/ {
			fread(&camera_controller.forward_key, sizeof(u16), 1, read_file);
			fread(&camera_controller.backward_key, sizeof(u16), 1, read_file);
			fread(&camera_controller.left_key, sizeof(u16), 1, read_file);
			fread(&camera_controller.right_key, sizeof(u16), 1, read_file);
			fread(&camera_controller.lower_key, sizeof(u16), 1, read_file);
			fread(&camera_controller.raise_key, sizeof(u16), 1, read_file);
			fread(&camera_controller.rotate_cw_key, sizeof(u16), 1, read_file);
			fread(&camera_controller.rotate_ccw_key, sizeof(u16), 1, read_file);
			fread(&camera_controller.pitch_down_key, sizeof(u16), 1, read_file);
			fread(&camera_controller.pitch_up_key, sizeof(u16), 1, read_file);
			fread(&camera_controller.increase_fov_key, sizeof(u16), 1, read_file);
			fread(&camera_controller.decrease_fov_key, sizeof(u16), 1, read_file);

			fread(&camera_controller.center_pos, sizeof(f32), 3, read_file);
			fread(&camera_controller.center_rot, sizeof(f32), 3, read_file);

			fread(&camera_controller.min_pos_z, sizeof(f32), 1, read_file);
			fread(&camera_controller.max_pos_z, sizeof(f32), 1, read_file);
			fread(&camera_controller.min_rot_y, sizeof(f32), 1, read_file);
			fread(&camera_controller.max_rot_y, sizeof(f32), 1, read_file);
			fread(&camera_controller.zoom_t, sizeof(f32), 1, read_file);

			fread(&camera_controller.translation_speed, sizeof(f32), 1, read_file);
			fread(&camera_controller.rotation_speed, sizeof(f32), 1, read_file);
			fread(&camera_controller.rotation_with_mouse_speed_multiplier_z, sizeof(f32), 1, read_file);
			fread(&camera_controller.rotation_with_mouse_speed_multiplier_y, sizeof(f32), 1, read_file);
			fread(&camera_controller.is_y_inverted, sizeof(bool), 1, read_file);
			fread(&camera_controller.zoom_speed, sizeof(f32), 1, read_file);
		}
		/*Camera*/ {
			auto& camera = camera_controller.camera;
			v3 position, rotation;
			fread(&position, sizeof(f32), 3, read_file);
			fread(&rotation, sizeof(f32), 3, read_file);
			camera.set_position(position);
			camera.set_rotation(rotation);
		}

		fclose(read_file);
	}
	void GameScene::save_the_game()
	{
		FILE* save_file{ fopen(std::format("{}.csf",save_name).c_str(), "wb") };
		auto& road_nodes{ m_RoadManager.road_nodes };
		auto& road_segments{ m_RoadManager.road_segments };
		auto& trees{ m_TreeManager.m_Trees };
		auto& buildings{ m_BuildingManager.m_Buildings };
		auto& cars{ m_CarManager.m_Cars };
		auto& building_types{ MainApplication->building_types };

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
				fwrite(&road_segments[i].CurvePoints, sizeof(f32), (3 * 4), save_file);
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
				auto& building{ buildings[i] };
				fwrite(&building->type, sizeof(u64), 1, save_file);
				fwrite(&building->object->position, sizeof(f32), 3, save_file);
				fwrite(&building->object->rotation, sizeof(f32), 3, save_file);

				fwrite(&building->connected_road_segment, sizeof(s64), 1, save_file);
				fwrite(&building->snapped_t_index, sizeof(u64), 1, save_file);
				fwrite(&building->snapped_t, sizeof(f32), 1, save_file);
				fwrite(&building->snapped_to_right, sizeof(bool), 1, save_file);

				fwrite(&building->max_health, sizeof(f32), 1, save_file);
				fwrite(&building->current_health, sizeof(f32), 1, save_file);
				fwrite(&building->electricity_need, sizeof(f32), 1, save_file);
				fwrite(&building->electricity_provided, sizeof(f32), 1, save_file);
				fwrite(&building->garbage_capacity, sizeof(f32), 1, save_file);
				fwrite(&building->current_garbage, sizeof(f32), 1, save_file);
				fwrite(&building->water_need, sizeof(f32), 1, save_file);
				fwrite(&building->water_provided, sizeof(f32), 1, save_file);
				fwrite(&building->water_waste_need, sizeof(f32), 1, save_file);
				fwrite(&building->water_waste_provided, sizeof(f32), 1, save_file);

				fwrite(&building->crime_reported, sizeof(u16), 1, save_file);

				u64 name_char_count{ building->name.size() };
				fwrite(&name_char_count, sizeof(u64), 1, save_file);
				fwrite(building->name.data(), sizeof(char), name_char_count, save_file);
			}
		}
		/*Car Manager*/ {
			u64 car_count = cars.size();
			fwrite(&car_count, sizeof(u64), 1, save_file);
			for (u64 i = 0; i < car_count; i++)
			{
				auto car = cars[i];
				s64 building_index = -1;

				//object
				fwrite(&car->object->position, sizeof(f32), 3, save_file);
				fwrite(&car->object->rotation, sizeof(f32), 3, save_file);

				fwrite(&car->type, sizeof(u64), 1, save_file);
				fwrite(&car->speed_in_kmh, sizeof(f32), 1, save_file);
				fwrite(&car->target, sizeof(f32), 3, save_file);
				fwrite(&car->target_park_pos, sizeof(f32), 3, save_file);
				fwrite(&car->heading_to_a_parking_spot, sizeof(bool), 1, save_file);
				fwrite(&car->road_segment, sizeof(s64), 1, save_file);

				//path
				u64 path_count = car->path.size();
				fwrite(&path_count, sizeof(u64), 1, save_file);
				for (u64 j = 0; j < path_count; j++)
				{
					auto td = car->path[j];
					fwrite(&td->road_segment_index, sizeof(u64), 1, save_file);
					fwrite(&td->next_road_node_index, sizeof(s64), 1, save_file);
					u64 points_count = td->points_stack.size();
					fwrite(&points_count, sizeof(u64), 1, save_file);
					for (v3& point : td->points_stack)
						fwrite(&point, sizeof(f32), 3, save_file);
					fwrite(&td->lane_index, sizeof(u32), 1, save_file);
				}

				if (car->building)
				{
					auto building_it = std::find(buildings.begin(), buildings.end(), car->building);
					building_index = std::distance(buildings.begin(), building_it);
				}
				fwrite(&building_index, sizeof(s64), 1, save_file);
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
				s64 work_car_index = -1;
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
				if (p->car_driving)
				{
					auto car_it = std::find(cars.begin(), cars.end(), p->car_driving);
					work_car_index = std::distance(cars.begin(), car_it);
				}
				fwrite(&p->type, sizeof(u64), 1, save_file);
				fwrite(&p->object->enabled, sizeof(bool), 1, save_file);
				fwrite(&p->road_segment, sizeof(s64), 1, save_file);
				fwrite(&p->road_node, sizeof(s64), 1, save_file);
				fwrite(&p->speed_in_kmh, sizeof(f32), 1, save_file);
				fwrite(&p->position, sizeof(f32), 3, save_file);
				fwrite(&p->target, sizeof(f32), 3, save_file);
				fwrite(&p->status, sizeof(PersonStatus), 1, save_file);
				fwrite(&p->heading_to_a_car, sizeof(bool), 1, save_file); // Write this before path
				fwrite(&path_count, sizeof(u64), 1, save_file);
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
				fwrite(&path_end_building_index, sizeof(s64), 1, save_file);
				fwrite(&path_start_building_index, sizeof(s64), 1, save_file);
				fwrite(&p->drove_in_work, sizeof(bool), 1, save_file);
				fwrite(&p->from_right, sizeof(bool), 1, save_file);
				fwrite(&p->heading_to_a_building, sizeof(bool), 1, save_file);
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
				fwrite(&work_car_index, sizeof(s64), 1, save_file);
			}
		}
		/*CameraController*/ {
			fwrite(&camera_controller.forward_key, sizeof(u16), 1, save_file);
			fwrite(&camera_controller.backward_key, sizeof(u16), 1, save_file);
			fwrite(&camera_controller.left_key, sizeof(u16), 1, save_file);
			fwrite(&camera_controller.right_key, sizeof(u16), 1, save_file);
			fwrite(&camera_controller.lower_key, sizeof(u16), 1, save_file);
			fwrite(&camera_controller.raise_key, sizeof(u16), 1, save_file);
			fwrite(&camera_controller.rotate_cw_key, sizeof(u16), 1, save_file);
			fwrite(&camera_controller.rotate_ccw_key, sizeof(u16), 1, save_file);
			fwrite(&camera_controller.pitch_down_key, sizeof(u16), 1, save_file);
			fwrite(&camera_controller.pitch_up_key, sizeof(u16), 1, save_file);
			fwrite(&camera_controller.increase_fov_key, sizeof(u16), 1, save_file);
			fwrite(&camera_controller.decrease_fov_key, sizeof(u16), 1, save_file);

			fwrite(&camera_controller.center_pos, sizeof(f32), 3, save_file);
			fwrite(&camera_controller.center_rot, sizeof(f32), 3, save_file);

			fwrite(&camera_controller.min_pos_z, sizeof(f32), 1, save_file);
			fwrite(&camera_controller.max_pos_z, sizeof(f32), 1, save_file);
			fwrite(&camera_controller.min_rot_y, sizeof(f32), 1, save_file);
			fwrite(&camera_controller.max_rot_y, sizeof(f32), 1, save_file);
			fwrite(&camera_controller.zoom_t, sizeof(f32), 1, save_file);

			fwrite(&camera_controller.translation_speed, sizeof(f32), 1, save_file);
			fwrite(&camera_controller.rotation_speed, sizeof(f32), 1, save_file);
			fwrite(&camera_controller.rotation_with_mouse_speed_multiplier_z, sizeof(f32), 1, save_file);
			fwrite(&camera_controller.rotation_with_mouse_speed_multiplier_y, sizeof(f32), 1, save_file);
			fwrite(&camera_controller.is_y_inverted, sizeof(bool), 1, save_file);
			fwrite(&camera_controller.zoom_speed, sizeof(f32), 1, save_file);
		}

		/*Camera*/
		auto& camera = camera_controller.camera;
		fwrite(&camera.position, sizeof(f32), 3, save_file);
		fwrite(&camera.rotation, sizeof(f32), 3, save_file);
		///////////////////////////////////////////////////

		fclose(save_file);
		printf("Game is saved.\n");
	}
	v3 GameScene::GetRayCastedFromScreen() const
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

	void init_game_scene(GameApp& app, GameScene& game_scene)
	{
		init_game_scene_ui_layer(game_scene.ui_layer, game_scene);
	}
	void load_game_scene(GameApp& app, GameScene& game_scene)
	{
		app.PushOverlay(&game_scene.ui_layer);
	}

	void unload_game_scene(GameApp& app, GameScene& game_scene)
	{
		app.PopOverlay(&game_scene.ui_layer);
	}
	void deinit_game_scene(GameApp& app, GameScene& game_scene)
	{
		deinit_game_scene_ui_layer(game_scene.ui_layer);
	}

	bool does_select_object(GameScene& game_scene)
	{
		auto& people{ game_scene.m_PersonManager.m_People };
		auto& cars{ game_scene.m_CarManager.m_Cars };
		auto& buildings{ game_scene.m_BuildingManager.m_Buildings };

		v3 cameraPosition{ game_scene.camera_controller.camera.position };
		v3 forward{ game_scene.GetRayCastedFromScreen() };

		for (auto person : people)
		{
			//TODO: Change to rotated bounding box collision
			if (Helper::check_if_ray_intersects_with_bounding_box(
				cameraPosition,
				forward,
				person->object->prefab->boundingBoxL + person->position,
				person->object->prefab->boundingBoxM + person->position
			))
			{
				game_scene.ui_layer.focused_person = person;
				return true;
			}
		}

		for (auto car : cars)
		{
			//TODO: Change to rotated bounding box collision
			if (Helper::check_if_ray_intersects_with_bounding_box(
				cameraPosition,
				forward,
				car->object->prefab->boundingBoxL + car->object->position,
				car->object->prefab->boundingBoxM + car->object->position
			))
			{
				game_scene.ui_layer.focused_car = car;
				return true;
			}
		}

		for (auto building : buildings)
		{
			//TODO: Change to rotated bounding box collision
			if (Helper::check_if_ray_intersects_with_bounding_box(
				cameraPosition,
				forward,
				building->object->prefab->boundingBoxL + building->object->position,
				building->object->prefab->boundingBoxM + building->object->position
			))
			{
				game_scene.ui_layer.selected_building = building;
				return true;
			}
		}


		return false;
	}
}
