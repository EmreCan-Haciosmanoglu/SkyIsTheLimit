#pragma once

namespace Can
{
	struct Transition
	{};

	struct RS_Transition : Transition
	{
		u64 from_path_array_index = 0;
		u64 to_path_array_index = 0;
		u64 road_segment_index = 0;
		f32 distance_from_middle = 0.0f;
		bool from_right = true;
	};

	struct RN_Transition : Transition
	{
		u64 from_road_segments_array_index = 0;
		u64 to_road_segments_array_index = 0;
		u64 sub_index = 1;
		u64 road_node_index = 0;
		bool accending = false;
	};
}