#pragma once
#include "Can.h"

#include "Types/RoadSegment.h"
#include "Types/Transition.h"
#include "Scenes/GameScene.h"

namespace Can::Helper
{
#define TERRAIN_SCALE_DOWN 10.0f
#define COLOR_COUNT 5

	struct Dijkstra_Node {
		s64 distance;
		s64 road_segment_index;
		s64 prev_road_node_index;
		s64 next_road_node_index;
	};

	struct Visited_Dijkstra_Node {
		s64 road_segment_index;
		bool to_end;
	};

	bool check_if_ray_intersects_with_bounding_box(
		const v3& ray_start_point,
		const v3& ray,
		const v3& mins,
		const v3& maxs
	);
	v2 check_rotated_rectangle_collision(
		const Object* const obj1, 
		const Object* const obj2
	);
	v2 check_rotated_rectangle_collision(
		const v2& r1l, const v2& r1m, f32 rot1, const v2& pos1,
		const v2& r2l, const v2& r2m, f32 rot2, const v2& pos2
	);

	v3 GetRayHitPointOnTerrain(void* s, const v3& cameraPosition, const v3& cameraDirection);

	f32 DistanceBetweenLineSLineS(v2 p1, v2 p2, v2 p3, v2 p4);

	bool RayTriangleIntersection(const v3& camPos, const v3& ray, const v3& A, const v3& B, const v3& C, const v3& normal, v3& intersection);


	void UpdateTheTerrain(const std::vector < std::array<v3, 3>>& polygon, bool reset);
	void UpdateTheTerrain(RoadSegment* rs, bool reset);

	Prefab* GetPrefabForTerrain(const std::string& texturePath);

	std::vector<std::string> GetFiles(const std::string& folder, const std::string& filter, const std::string& fileType);

	void name_me_normals(u64 w, u64 h, u64 min_x, u64 max_x, u64 min_y, u64 max_y, f32* vertices);
	void name_me_cutting(u64 w, u64 h, v3 AB, v3 current_point, f32* vertices);
	std::array<u64, 4> name_me_digging(u64 w, u64 h, const std::vector<std::array<v3, 3>>& polygon, f32* vertices, bool reset);

	std::string trim_path_and_extension(std::string& path);

	std::vector<Transition*> get_path(Building* start, u8 dist);
	std::vector<RS_Transition_For_Vehicle*> get_path_for_a_car(Building* start, u8 dist);

	std::vector<Transition*> get_path(
		const Building* const start,
		const Building* const end
	);
	std::vector<RS_Transition_For_Vehicle*> get_path_for_a_car(
		const Building* const start,
		const Building* const end
	);
	std::vector<RS_Transition_For_Vehicle*> get_path_for_gargabe_vehicle(
		const Building* const building,
		Building*& to
	);
	void find_and_assign_a_policeman(Building* const& b);
	void find_and_assign_an_ambulance(Building* const& b);

	struct sort_by_angle
	{
		inline bool operator() (u64 roadSegment1, u64 roadSegment2)
		{
			auto& segments = GameScene::ActiveGameScene->m_RoadManager.road_segments;
			RoadSegment& rs1 = segments[roadSegment1];
			RoadSegment& rs2 = segments[roadSegment2];
			f32 roadSegmentR1 = 0.002f;
			f32 roadSegmentR2 = 0.001f;
			if (rs1.EndNode == rs2.EndNode)
			{
				roadSegmentR1 = rs1.GetEndRotation().y;
				roadSegmentR2 = rs2.GetEndRotation().y;
			}
			else if (rs1.EndNode == rs2.StartNode)
			{
				roadSegmentR1 = rs1.GetEndRotation().y;
				roadSegmentR2 = rs2.GetStartRotation().y;
			}
			else if (rs1.StartNode == rs2.EndNode)
			{
				roadSegmentR1 = rs1.GetStartRotation().y;
				roadSegmentR2 = rs2.GetEndRotation().y;
			}
			else if (rs1.StartNode == rs2.StartNode)
			{
				roadSegmentR1 = rs1.GetStartRotation().y;
				roadSegmentR2 = rs2.GetStartRotation().y;
			}
			else
				assert(false); // Why are you here

			roadSegmentR1 = std::fmod(roadSegmentR1 + glm::radians(360.0f), glm::radians(360.0f));
			roadSegmentR2 = std::fmod(roadSegmentR2 + glm::radians(360.0f), glm::radians(360.0f));
			return (roadSegmentR1 < roadSegmentR2);
		}
	};
	struct sort_by_distance
	{
		inline bool operator() (std::pair<u64, std::vector<u64>> path1, std::pair<u64, std::vector<u64>> path2)
		{
			return path1.first > path2.first;
		}
		inline bool operator() (Dijkstra_Node path1, Dijkstra_Node path2)
		{
			return path1.distance > path2.distance;
		}
	};
	struct sort_by_left_journey
	{
		inline bool operator() (const Car* const c1, const Car* const c2)
		{
			u64 size1 = c1->path[0]->points_stack.size();
			u64 size2 = c2->path[0]->points_stack.size();

			if (size1 == size2)
				return c1->path[0]->left_journey < c2->path[0]->left_journey;
			else
				return size1 < size2;
		}
	};
}