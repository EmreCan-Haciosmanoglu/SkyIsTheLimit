#pragma once

namespace Can
{
	struct Transition
	{};

	struct RS_Transition : Transition
	{
		u64 from_index = 0;
		u64 to_index = 0;
		f32 from_t = 0.0f;
		f32 to_t = 0.0f;
		f32 distance_from_middle = 0.0f;
		bool from_right = true;
	};

	struct RN_Transition : Transition
	{

	};
}