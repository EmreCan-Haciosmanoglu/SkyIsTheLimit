#pragma once

namespace Can
{
	struct Transition
	{};

	struct RS_Transition : Transition
	{
		u64 at_path_array_index = 0;
		u64 road_segment_index = 0;
		bool from_start = true;
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