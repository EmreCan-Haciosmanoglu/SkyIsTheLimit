#pragma once
#include "Can.h"

namespace Can
{
	class Car
	{
	public:
		~Car() { assert(object != nullptr); delete object; }

		Object* object{ nullptr };
		u64 type{ 0 };
		f32 speed_in_kmh{ 0.0f };
		v3 target{};
		v3 target_park_pos{};
		bool heading_to_a_parking_spot{ false };
		s64 road_segment{ -1 };

		std::vector<struct RS_Transition_For_Vehicle*> path{};
		bool path_is_blocked{ false }; //@NoSerialize

		class Person* owner{ nullptr };
		class Person* driver{ nullptr };
		class Building* building{ nullptr };
	};
}
