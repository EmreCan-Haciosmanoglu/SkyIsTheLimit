#include "canpch.h"
#include "GameApp.h"
#include "Can/EntryPoint.h"
#include "Can.h"
#include "Helper.h"

#include "Types/Vehicle_Type.h"
#include "Types/Road_Type.h"
#include "Types/Building_Type.h"

Can::Application* Can::CreateApplication(const Can::WindowProps& props)
{
	return new GameApp(props);
}

namespace Can
{
	GameApp* GameApp::instance = nullptr;

	/*Anom*/ namespace
	{
#define TEMP_SHADER_FILE_PATH "assets/shaders/3DTexturedObject.glsl"
		void load_road_types(std::vector<Road_Type>& road_types)
		{
			namespace fs = std::filesystem;
			std::string current_path{ fs::current_path().string() };
			std::string path_to_roads{ current_path.append("\\assets\\objects\\roads\\") };
			std::string path_to_road_identifier_list{ std::string(path_to_roads).append("list.txt") };

			std::ifstream file(path_to_road_identifier_list);

			if (file.is_open())
			{
				const std::string start_key{ "START" };
				const std::string end_key{ "END" };

				constexpr const char* name_key{ "Name" };
				constexpr const char* asym_key{ "Asym" };
				constexpr const char* zone_key{ "Zoneable" };
				constexpr const char* has_median_key{ "Has_Median" };

				constexpr const char* road_object_key{ "Road_Object" };
				constexpr const char* road_texture_key{ "Road_Texture" };
				constexpr const char* road_junction_object_key{ "Road_Junc_Object" };
				constexpr const char* road_junction_texture_key{ "Road_Junc_Texture" };
				constexpr const char* road_junction_mirror_object_key{ "Road_Junc_Mirror_Object" };
				constexpr const char* road_junction_mirror_texture_key{ "Road_Junc_Mirror_Texture" };
				constexpr const char* road_end_object_key{ "Road_End_Object" };
				constexpr const char* road_end_texture_key{ "Road_End_Texture" };
				constexpr const char* road_end_mirror_object_key{ "Road_End_Mirror_Object" };
				constexpr const char* road_end_mirror_texture_key{ "Road_End_Mirror_Texture" };

				constexpr const char* tunnel_object_key{ "Tunnel_Object" };
				constexpr const char* tunnel_texture_key{ "Tunnel_Texture" };
				constexpr const char* tunnel_junction_object_key{ "Tunnel_Junc_Object" };
				constexpr const char* tunnel_junction_texture_key{ "Tunnel_Junc_Texture" };
				constexpr const char* tunnel_junction_mirror_object_key{ "Tunnel_Junc_Mirror_Object" };
				constexpr const char* tunnel_junction_mirror_texture_key{ "Tunnel_Junc_Mirror_Texture" };
				constexpr const char* tunnel_end_object_key{ "Tunnel_End_Object" };
				constexpr const char* tunnel_end_texture_key{ "Tunnel_End_Texture" };
				constexpr const char* tunnel_end_mirror_object_key{ "Tunnel_End_Mirror_Object" };
				constexpr const char* tunnel_end_mirror_texture_key{ "Tunnel_End_Mirror_Texture" };

				constexpr const char* tunnel_entrance_object_key{ "Tunnel_Entrance_Object" };
				constexpr const char* tunnel_entrance_texture_key{ "Tunnel_Entrance_Texture" };
				constexpr const char* tunnel_entrance_mirror_object_key{ "Tunnel_Entrance_Mirror_Object" };
				constexpr const char* tunnel_entrance_mirror_texture_key{ "Tunnel_Entrance_Mirror_Texture" };

				constexpr const char* thumbnail_key{ "Thumbnail" };

				constexpr const char* lanes_backward_key{ "Lanes_Backward" };
				constexpr const char* lanes_forward_key{ "Lanes_Forward" };

				std::string line;
				while (std::getline(file, line))
				{
					// using printf() in all tests for consistency
					if (line != start_key)
						continue;
					bool tunnel_is_found{ false };

					Road_Type& road_type{ road_types.emplace_back() };

					std::string road_obj{ path_to_roads };
					std::string road_png{ path_to_roads };
					std::string road_junction_obj{ path_to_roads };
					std::string road_junction_png{ path_to_roads };
					std::string road_junction_mirror_obj{ path_to_roads };
					std::string road_junction_mirror_png{ path_to_roads };
					std::string road_end_obj{ path_to_roads };
					std::string road_end_png{ path_to_roads };
					std::string road_end_mirror_obj{ path_to_roads };
					std::string road_end_mirror_png{ path_to_roads };

					std::string tunnel_obj{ path_to_roads };
					std::string tunnel_png{ path_to_roads };
					std::string tunnel_junction_obj{ path_to_roads };
					std::string tunnel_junction_png{ path_to_roads };
					std::string tunnel_junction_mirror_obj{ path_to_roads };
					std::string tunnel_junction_mirror_png{ path_to_roads };
					std::string tunnel_end_obj{ path_to_roads };
					std::string tunnel_end_png{ path_to_roads };
					std::string tunnel_end_mirror_obj{ path_to_roads };
					std::string tunnel_end_mirror_png{ path_to_roads };

					std::string tunnel_entrance_obj{ path_to_roads };
					std::string tunnel_entrance_png{ path_to_roads };
					std::string tunnel_entrance_mirror_obj{ path_to_roads };
					std::string tunnel_entrance_mirror_png{ path_to_roads };

					while (std::getline(file, line))
					{
						if (line == end_key)
						{
							road_type.road = new Prefab(road_obj, TEMP_SHADER_FILE_PATH, road_png);
							road_type.road_junction = new Prefab(road_junction_obj, TEMP_SHADER_FILE_PATH, road_junction_png);
							road_type.road_end = new Prefab(road_end_obj, TEMP_SHADER_FILE_PATH, road_end_png);
							if (road_type.asymmetric)
							{
								road_type.road_junction_mirror = new Prefab(road_junction_mirror_obj, TEMP_SHADER_FILE_PATH, road_junction_mirror_png);
								road_type.road_end_mirror = new Prefab(road_end_mirror_obj, TEMP_SHADER_FILE_PATH, road_end_mirror_png);
							}
							if (tunnel_is_found)
							{
								road_type.tunnel = new Prefab(tunnel_obj, TEMP_SHADER_FILE_PATH, tunnel_png);
								road_type.tunnel_junction = new Prefab(tunnel_junction_obj, TEMP_SHADER_FILE_PATH, tunnel_junction_png);
								road_type.tunnel_end = new Prefab(tunnel_end_obj, TEMP_SHADER_FILE_PATH, tunnel_end_png);
								road_type.tunnel_entrance = new Prefab(tunnel_entrance_obj, TEMP_SHADER_FILE_PATH, tunnel_entrance_png);
								if (road_type.asymmetric)
								{
									road_type.tunnel_junction_mirror = new Prefab(tunnel_junction_mirror_obj, TEMP_SHADER_FILE_PATH, tunnel_junction_mirror_png);
									road_type.tunnel_end_mirror = new Prefab(tunnel_end_mirror_obj, TEMP_SHADER_FILE_PATH, tunnel_end_mirror_png);
									road_type.tunnel_entrance_mirror = new Prefab(tunnel_entrance_mirror_obj, TEMP_SHADER_FILE_PATH, tunnel_entrance_mirror_png);
								}
								road_type.tunnel_length = road_type.tunnel->boundingBoxM.x - road_type.tunnel->boundingBoxL.x;
								road_type.tunnel_width = road_type.tunnel->boundingBoxM.y - road_type.tunnel->boundingBoxL.y;
								road_type.tunnel_height = road_type.tunnel->boundingBoxM.z - road_type.tunnel->boundingBoxL.z;
								road_type.tunnel_junction_length = road_type.tunnel_junction->boundingBoxM.x - road_type.tunnel_junction->boundingBoxL.x;
							}
							road_type.road_length = road_type.road->boundingBoxM.x - road_type.road->boundingBoxL.x;
							road_type.road_width = road_type.road->boundingBoxM.y - road_type.road->boundingBoxL.y;
							road_type.road_height = road_type.road->boundingBoxM.z - road_type.road->boundingBoxL.z;
							road_type.road_junction_length = road_type.road_junction->boundingBoxM.x - road_type.road_junction->boundingBoxL.x;
							break;
						}
						std::string::iterator seperator{ std::find(line.begin(), line.end(), ':') };
						if (seperator == line.end())
							break;
						char* print_from = seperator._Unwrapped() + 1;
						if (std::equal(line.begin(), seperator, name_key))
							road_type.name = std::string(print_from);
						else if (std::equal(line.begin(), seperator, asym_key))
							road_type.asymmetric = std::string(print_from) != "False";
						else if (std::equal(line.begin(), seperator, zone_key))
							road_type.zoneable = std::string(print_from) != "False";
						else if (std::equal(line.begin(), seperator, has_median_key))
							road_type.has_median = std::string(print_from) != "False";
						else if (std::equal(line.begin(), seperator, road_object_key))
							road_obj.append(print_from);
						else if (std::equal(line.begin(), seperator, road_texture_key))
							road_png.append(print_from);
						else if (std::equal(line.begin(), seperator, road_junction_object_key))
							road_junction_obj.append(print_from);
						else if (std::equal(line.begin(), seperator, road_junction_texture_key))
							road_junction_png.append(print_from);
						else if (std::equal(line.begin(), seperator, road_junction_mirror_object_key))
							road_junction_mirror_obj.append(print_from);
						else if (std::equal(line.begin(), seperator, road_junction_mirror_texture_key))
							road_junction_mirror_png.append(print_from);
						else if (std::equal(line.begin(), seperator, road_end_object_key))
							road_end_obj.append(print_from);
						else if (std::equal(line.begin(), seperator, road_end_texture_key))
							road_end_png.append(print_from);
						else if (std::equal(line.begin(), seperator, road_end_mirror_object_key))
							road_end_mirror_obj.append(print_from);
						else if (std::equal(line.begin(), seperator, road_end_mirror_texture_key))
							road_end_mirror_png.append(print_from);
						else if (std::equal(line.begin(), seperator, tunnel_object_key))
						{
							tunnel_obj.append(print_from);
							tunnel_is_found = true;
						}
						else if (std::equal(line.begin(), seperator, tunnel_texture_key))
							tunnel_png.append(print_from);
						else if (std::equal(line.begin(), seperator, tunnel_junction_object_key))
							tunnel_junction_obj.append(print_from);
						else if (std::equal(line.begin(), seperator, tunnel_junction_texture_key))
							tunnel_junction_png.append(print_from);
						else if (std::equal(line.begin(), seperator, tunnel_junction_mirror_object_key))
							tunnel_junction_mirror_obj.append(print_from);
						else if (std::equal(line.begin(), seperator, tunnel_junction_mirror_texture_key))
							tunnel_junction_mirror_png.append(print_from);
						else if (std::equal(line.begin(), seperator, tunnel_end_object_key))
							tunnel_end_obj.append(print_from);
						else if (std::equal(line.begin(), seperator, tunnel_end_texture_key))
							tunnel_end_png.append(print_from);
						else if (std::equal(line.begin(), seperator, tunnel_end_mirror_object_key))
							tunnel_end_mirror_obj.append(print_from);
						else if (std::equal(line.begin(), seperator, tunnel_end_mirror_texture_key))
							tunnel_end_mirror_png.append(print_from);
						else if (std::equal(line.begin(), seperator, tunnel_entrance_object_key))
							tunnel_entrance_obj.append(print_from);
						else if (std::equal(line.begin(), seperator, tunnel_entrance_texture_key))
							tunnel_entrance_png.append(print_from);
						else if (std::equal(line.begin(), seperator, tunnel_entrance_mirror_object_key))
							tunnel_entrance_mirror_obj.append(print_from);
						else if (std::equal(line.begin(), seperator, tunnel_entrance_mirror_texture_key))
							tunnel_entrance_mirror_png.append(print_from);
						else if (std::equal(line.begin(), seperator, thumbnail_key))
							road_type.thumbnail = Texture2D::Create(std::format("{}{}", path_to_roads, print_from));
						else if (std::equal(line.begin(), seperator, lanes_backward_key))
						{
							std::stringstream ss;
							ss << std::string(print_from);

							u64 l_backward_count{ 0 };
							ss >> l_backward_count;
							road_type.lanes_backward.reserve(l_backward_count);
							u64 lanes_read = 0;
							while (std::getline(file, line))
							{
								std::string::iterator first_seperator = std::find(line.begin(), line.end(), ':');
								std::string::iterator second_seperator = std::find(first_seperator + 1, line.end(), ':');
								Lane& lane{ road_type.lanes_backward.emplace_back() };
								ss.clear();
								ss.str("");
								ss << std::string(line.begin(), first_seperator);
								ss >> lane.distance_from_center;

								ss.clear();
								ss.str("");
								ss << std::string(first_seperator + 1, second_seperator);
								ss >> lane.speed_limit;

								ss.clear();
								ss.str("");
								ss << std::string(second_seperator + 1, line.end());
								ss >> lane.width;
								lanes_read++;
								if (lanes_read >= l_backward_count)
									break;
							}

						}
						else if (std::equal(line.begin(), seperator, lanes_forward_key))
						{
							std::stringstream ss;
							ss << std::string(print_from);

							u64 l_forward_count{ 0 };
							ss >> l_forward_count;
							road_type.lanes_forward.reserve(l_forward_count);
							u64 lanes_read = 0;
							while (std::getline(file, line))
							{
								std::string::iterator first_seperator = std::find(line.begin(), line.end(), ':');
								std::string::iterator second_seperator = std::find(first_seperator + 1, line.end(), ':');
								Lane& lane{ road_type.lanes_forward.emplace_back() };
								ss.clear();
								ss.str("");
								ss << std::string(line.begin(), first_seperator);
								ss >> lane.distance_from_center;

								ss.clear();
								ss.str("");
								ss << std::string(first_seperator + 1, second_seperator);
								ss >> lane.speed_limit;

								ss.clear();
								ss.str("");
								ss << std::string(second_seperator + 1, line.end());
								ss >> lane.width;

								lanes_read++;
								if (lanes_read >= l_forward_count)
									break;
							}
						}
					}
				}
				file.close();
			}
		}
		void load_vehicle_types(std::vector<Vehicle_Type>& vehicle_types)
		{
			namespace fs = std::filesystem;
			std::string current_path = fs::current_path().string();
			std::string path_to_cars = current_path.append("\\assets\\objects\\cars\\");
			std::string path_to_car_identifier_list = std::string(path_to_cars).append("list.txt");

			std::ifstream file(path_to_car_identifier_list);

			if (file.is_open())
			{
				const std::string start_key{ "START" };
				const std::string end_key{ "END" };

				constexpr const char* name_key{ "Name" };
				constexpr const char* object_key{ "Object" };
				constexpr const char* texture_key{ "Texture" };
				constexpr const char* thumbnail_key{ "Thumbnail" };
				constexpr const char* speed_range_key{ "Speed_Range" };
				constexpr const char* operator_count_key{ "Operator_Count" };
				constexpr const char* passenger_limit_key{ "Passenger_Limit" };
				constexpr const char* type_key{ "Type" };

				std::string line;
				while (std::getline(file, line))
				{
					// using printf() in all tests for consistency
					if (line != start_key)
						continue;

					Vehicle_Type& vehicle_type{ vehicle_types.emplace_back() };
					std::string object{ path_to_cars };
					std::string texture{ path_to_cars };

					while (std::getline(file, line))
					{
						if (line == end_key)
						{
							vehicle_type.prefab = new Prefab(object, TEMP_SHADER_FILE_PATH, texture);

							vehicle_type.object_length = vehicle_type.prefab->boundingBoxM.x - vehicle_type.prefab->boundingBoxL.x;
							vehicle_type.object_width = vehicle_type.prefab->boundingBoxM.y - vehicle_type.prefab->boundingBoxL.y;
							break;
						}
						std::string::iterator seperator = std::find(line.begin(), line.end(), ':');
						if (seperator == line.end())
							break;
						char* print_from = seperator._Unwrapped() + 1;
						if (std::equal(line.begin(), seperator, name_key))
							vehicle_type.name = std::string(print_from);
						else if (std::equal(line.begin(), seperator, object_key))
							object.append(print_from);
						else if (std::equal(line.begin(), seperator, texture_key))
							texture.append(print_from);
						else if (std::equal(line.begin(), seperator, thumbnail_key))
							vehicle_type.thumbnail = Texture2D::Create(std::format("{}{}", path_to_cars, print_from));
						else if (std::equal(line.begin(), seperator, speed_range_key))
						{
							std::stringstream ss{};
							auto dash_seperator = std::find(seperator + 1, line.end(), '-');
							ss << std::string(seperator + 1, dash_seperator);
							ss >> vehicle_type.speed_range_min;

							ss.clear();
							ss.str("");
							ss << std::string(dash_seperator + 1, line.end());
							ss >> vehicle_type.speed_range_max;
						}
						else if (std::equal(line.begin(), seperator, operator_count_key))
						{
							std::stringstream ss{};
							ss << std::string(seperator + 1, line.end());
							ss >> vehicle_type.operator_count;
						}
						else if (std::equal(line.begin(), seperator, passenger_limit_key))
						{
							std::stringstream ss{};
							ss << std::string(seperator + 1, line.end());
							ss >> vehicle_type.passenger_limit;
						}
						else if (std::equal(line.begin(), seperator, type_key))
						{
							std::stringstream ss{};
							ss << std::string(seperator + 1, line.end());
							u8 t;
							ss >> t;
							vehicle_type.type = (Car_Type)t;
						}
					}
				}
				file.close();
			}
		}
		void load_building_types(std::vector<Building_Type>& building_types)
		{
			namespace fs = std::filesystem;
			const std::string current_path{ fs::current_path().string() };
			const std::string path_to_buildings{ std::format("{}\\assets\\objects\\buildings\\", current_path) };
			const std::string path_to_buildings_identifier_list{ std::format("{}list.txt", path_to_buildings) };

			std::ifstream file{ path_to_buildings_identifier_list };

			if (file.is_open())
			{
				const std::string start_key{ "START" };
				const std::string end_key{ "END" };

				constexpr const char* object_key{ "Object" };
				constexpr const char* texture_key{ "Texture" };
				constexpr const char* thumbnail_key{ "Thumbnail" };

				constexpr const char* name_key{ "Name" };
				constexpr const char* capacity_key{ "Capacity" };
				constexpr const char* group_key{ "Group" };
				constexpr const char* vehicle_capacity_key{ "Vehicle_Capacity" };

				constexpr const char* needed_uneducated_key{ "Neded_Uneducated" };
				constexpr const char* needed_elementary_school_key{ "Neded_Elementary_School" };
				constexpr const char* needed_high_school_key{ "Neded_High_School" };
				constexpr const char* needed_associate_s_key{ "Neded_Associate_s" };
				constexpr const char* needed_bachelor_s_key{ "Neded_Bachelor_s" };
				constexpr const char* needed_master_key{ "Neded_Master" };
				constexpr const char* needed_doctorate_key{ "Neded_Doctorate" };

				constexpr const char* visitor_capacity_key{ "Visitor_Capacity" };
				constexpr const char* stay_visitor_capacity_key{ "Stay_Visitor_Capacity" };

				constexpr const char* vehicle_parks_key{ "Vehicle_Parks" };

				std::string line;
				while (std::getline(file, line))
				{
					// using printf() in all tests for consistency
					if (line != start_key)
						continue;

					Building_Type& building_type{ building_types.emplace_back() };

					std::string object{ path_to_buildings };
					std::string texture{ path_to_buildings };

					while (std::getline(file, line))
					{
						if (line == end_key)
						{
							building_type.prefab = new Prefab(object, TEMP_SHADER_FILE_PATH, texture);

							building_type.object_length = building_type.prefab->boundingBoxM.x - building_type.prefab->boundingBoxL.x;
							building_type.object_width = building_type.prefab->boundingBoxM.y - building_type.prefab->boundingBoxL.y;
							building_type.object_height = building_type.prefab->boundingBoxM.z - building_type.prefab->boundingBoxL.z;
							break;
						}
						std::string::iterator seperator = std::find(line.begin(), line.end(), ':');
						if (seperator == line.end())
							break;
						char* print_from = seperator._Unwrapped() + 1;
						if (std::equal(line.begin(), seperator, name_key))
							building_type.name = print_from;
						else if (std::equal(line.begin(), seperator, object_key))
							object.append(print_from);
						else if (std::equal(line.begin(), seperator, texture_key))
							texture.append(print_from);
						else if (std::equal(line.begin(), seperator, thumbnail_key))
							building_type.thumbnail = Texture2D::Create(std::format("{}{}", path_to_buildings, print_from));
						else if (std::equal(line.begin(), seperator, capacity_key))
						{
							std::stringstream ss{};
							ss << std::string(seperator + 1, line.end());
							ss >> building_type.capacity;
						}
						else if (std::equal(line.begin(), seperator, group_key))
						{
							std::stringstream ss{};
							ss << std::string(seperator + 1, line.end());
							u8 group{ 0 };
							ss >> group;
							building_type.group = (Building_Group)(group - '0');
						}
						else if (std::equal(line.begin(), seperator, vehicle_capacity_key))
						{
							std::stringstream ss{};
							ss << std::string(seperator + 1, line.end());
							ss >> building_type.vehicle_capacity;
						}
						else if (std::equal(line.begin(), seperator, needed_uneducated_key))
						{
							std::stringstream ss{};
							ss << std::string(seperator + 1, line.end());
							ss >> building_type.needed_uneducated;
						}
						else if (std::equal(line.begin(), seperator, needed_elementary_school_key))
						{
							std::stringstream ss{};
							ss << std::string(seperator + 1, line.end());
							ss >> building_type.needed_elementary_school;
						}
						else if (std::equal(line.begin(), seperator, needed_high_school_key))
						{
							std::stringstream ss{};
							ss << std::string(seperator + 1, line.end());
							ss >> building_type.needed_high_school;
						}
						else if (std::equal(line.begin(), seperator, needed_associate_s_key))
						{
							std::stringstream ss{};
							ss << std::string(seperator + 1, line.end());
							ss >> building_type.needed_associate_s;
						}
						else if (std::equal(line.begin(), seperator, needed_bachelor_s_key))
						{
							std::stringstream ss{};
							ss << std::string(seperator + 1, line.end());
							ss >> building_type.needed_bachelor_s;
						}
						else if (std::equal(line.begin(), seperator, needed_master_key))
						{
							std::stringstream ss{};
							ss << std::string(seperator + 1, line.end());
							ss >> building_type.needed_master;
						}
						else if (std::equal(line.begin(), seperator, needed_doctorate_key))
						{
							std::stringstream ss{};
							ss << std::string(seperator + 1, line.end());
							ss >> building_type.needed_doctorate;
						}
						else if (std::equal(line.begin(), seperator, visitor_capacity_key))
						{
							std::stringstream ss{};
							ss << std::string(seperator + 1, line.end());
							ss >> building_type.visitor_capacity;
						}
						else if (std::equal(line.begin(), seperator, stay_visitor_capacity_key))
						{
							std::stringstream ss{};
							ss << std::string(seperator + 1, line.end());
							ss >> building_type.stay_visitor_capacity;
						}
						else if (std::equal(line.begin(), seperator, vehicle_parks_key))
						{
							std::stringstream ss;
							ss << std::string(print_from);

							u16 vehicle_parks_count = 0;
							ss >> vehicle_parks_count;
							building_type.vehicle_parks.reserve(vehicle_parks_count);
							u64 lanes_read = 0;
							while (std::getline(file, line))
							{
								std::string::iterator first_seperator = std::find(line.begin(), line.end(), ':');
								std::string::iterator second_seperator = std::find(first_seperator + 1, line.end(), ':');
								std::string::iterator third_seperator = std::find(second_seperator + 1, line.end(), ':');
								Vehicle_Park& vehicle_park{ building_type.vehicle_parks.emplace_back() };
								ss.clear();
								ss.str("");
								ss << std::string(line.begin(), first_seperator);
								ss >> vehicle_park.offset.x;

								ss.clear();
								ss.str("");
								ss << std::string(first_seperator + 1, second_seperator);
								ss >> vehicle_park.offset.y;

								ss.clear();
								ss.str("");
								ss << std::string(second_seperator + 1, third_seperator);
								ss >> vehicle_park.offset.z;

								ss.clear();
								ss.str("");
								ss << std::string(third_seperator + 1, line.end());
								ss >> vehicle_park.rotation_in_degrees;

								lanes_read++;
								if (lanes_read >= vehicle_parks_count)
									break;
							}
						}
					}
				}
				file.close();
			}
		}
	}

	GameApp::GameApp(const Can::WindowProps& props)
		: Application(props)
		, perspective_camera_controller(
			45.0f,
			16.0f / 9.0f,
			0.1f,
			1000.0f,
			v3{ 2.0f, -3.0f, 5.0f },
			v3{ 0.0f, -45.0f, 90.0f }
		)
	{
		instance = this;

		terrainPrefab = Helper::GetPrefabForTerrain("assets/objects/flat_land.png");
		terrainTexture = Texture2D::Create("assets/objects/flat_land.png");
		//terrainPrefab = Helper::GetPrefabForTerrain("assets/objects/flat_land_small.png");
		//terrainPrefab = Helper::GetPrefabForTerrain("assets/objects/heightmap_smallest.png");
		//terrainPrefab = Helper::GetPrefabForTerrain("assets/objects/heightmap.png");

		tree_map = Texture2D::Create("assets/textures/treeMap.png");
		add_texture = Texture2D::Create("assets/textures/Buttons/Add.png");
		save_texture = Texture2D::Create("assets/textures/Buttons/Save.png");
		pause_texture = Texture2D::Create("assets/textures/Buttons/Pause.png");
		remove_texture = Texture2D::Create("assets/textures/Buttons/Remove.png");
		cancel_texture = Texture2D::Create("assets/textures/Buttons/Cancel.png");
		change_texture = Texture2D::Create("assets/textures/Buttons/Change.png");
		upgrade_texture = Texture2D::Create("assets/textures/Buttons/Upgrade.png");
		straight_texture = Texture2D::Create("assets/textures/Buttons/Straight.png");
		quadratic_texture = Texture2D::Create("assets/textures/Buttons/Quadratic.png");
		downgrade_texture = Texture2D::Create("assets/textures/Buttons/Downgrade.png");
		cubic_1234_texture = Texture2D::Create("assets/textures/Buttons/Cubic1234.png");
		cubic_1243_texture = Texture2D::Create("assets/textures/Buttons/Cubic1243.png");
		cubic_1342_texture = Texture2D::Create("assets/textures/Buttons/Cubic1342.png");
		cubic_1432_texture = Texture2D::Create("assets/textures/Buttons/Cubic1432.png");
		normal_speed_texture = Texture2D::Create("assets/textures/Buttons/NormalSpeed.png");
		two_times_speed_texture = Texture2D::Create("assets/textures/Buttons/TwoTimesSpeed.png");
		four_times_speed_texture = Texture2D::Create("assets/textures/Buttons/FourTimesSpeed.png");

		houses_texture = Texture2D::Create("assets/textures/Buttons/Houses.png");
		residentials_texture = Texture2D::Create("assets/textures/Buttons/Residentials.png");
		industrials_texture = Texture2D::Create("assets/textures/Buttons/Industrials.png");
		offices_texture = Texture2D::Create("assets/textures/Buttons/Offices.png");
		hospitals_texture = Texture2D::Create("assets/textures/Buttons/Hospitals.png");

		load_road_types(road_types);
		load_building_types(building_types);
		LoadTrees();
		load_vehicle_types(vehicle_types);
		LoadPeople();

		init_main_menu(*this, main_menu);
		load_main_menu(*this, main_menu);
	}

	void GameApp::start_the_game(std::string& save_name, bool is_old_game)
	{
		unload_main_menu(*this, main_menu);
		deinit_main_menu(*this, main_menu);

		gameScene = new GameScene(this, save_name);
		PushLayer(gameScene);
		if (is_old_game)
			gameScene->load_the_game();

		uiScene = new UIScene(this);
		PushOverlay(uiScene);

		//debugScene = new Debug(this);
		//PushOverlay(debugScene);
	}

	GameApp::~GameApp()
	{
		//PopLayer(gameScene);
		//delete gameScene;
		//PopOverlay(uiScene);
		//delete uiScene;
		//PopOverlay(debugScene);
		//delete debugScene;

		//unload_main_menu(*this, main_menu);
	}

	void GameApp::LoadTrees()
	{
		trees = LoadPrefabs("\\assets\\objects\\trees", "Tree_");
	}
	void GameApp::LoadPeople()
	{
		people = LoadPrefabs("\\assets\\objects\\People", "Person_");
	}

	std::vector<Prefab*> GameApp::LoadPrefabs(const std::string& folder, const std::string& filter)
	{
#define TEMP_SHADER_FILE_PATH "assets/shaders/3DTexturedObject.glsl"
		std::vector<Prefab*> result;
		namespace fs = std::filesystem;
		std::string s = fs::current_path().string();
		std::string path = s.append(folder.c_str());

		std::vector<std::string> objfiles = Helper::GetFiles(path, filter, ".obj");
		std::vector<std::string> pngfiles = Helper::GetFiles(path, filter, ".png");

		size_t fileCount = objfiles.size();
		for (size_t i = 0; i < fileCount; i++)
			result.push_back(new Prefab(objfiles[i], TEMP_SHADER_FILE_PATH, pngfiles[i]));
		return result;
	}
}