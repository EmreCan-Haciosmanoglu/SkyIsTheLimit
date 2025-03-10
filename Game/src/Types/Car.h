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
		f32 cargo{ 0.0f };
		v3 target{};
		v3 target_park_pos{};
		bool heading_to_a_parking_spot{ false };
		bool heading_to_a_visiting_spot{ false };
		s64 road_segment{ -1 };

		float t = 1.0f;
		std::array<glm::vec3, 3> driftpoints{};

		std::vector<struct RS_Transition_For_Vehicle*> path{};

		class Person* driver{ nullptr };
		std::vector<class Person*> passengers{};
		class Building* building{ nullptr };
	};
}
