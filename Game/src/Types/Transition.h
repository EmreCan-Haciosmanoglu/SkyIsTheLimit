#pragma once

namespace Can
{
	struct Transition
	{};

	struct RS_Transition_For_Walking : Transition
	{
		u64 at_path_array_index = 0;
		u64 road_segment_index = 0;
		bool from_start = true;
		bool from_right = true;
	};

	struct RN_Transition_For_Walking : Transition
	{
		u64 from_road_segments_array_index = 0;
		u64 to_road_segments_array_index = 0;
		u64 sub_index = 1;
		u64 road_node_index = 0;
		bool accending = false;
	};

	struct RS_Transition_For_Vehicle
	{
		u64 road_segment_index = 0;
		s64 next_road_node_index = -1;
		std::vector<v3> points_stack{};
		u32 lane_index = 0;
		f32 left_journey = 0.0f;
	};
}