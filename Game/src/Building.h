#pragma once
#include "Can.h"

namespace Can
{

	class Building
	{
	public:
		Building() = default;
		~Building() { delete object; }

	public:
		u64 type{ 0 };
		Object* object{ nullptr };

		s64 connected_road_segment{ -1 };
		s64 snapped_t_index{ 0 };
		f32 snapped_t{ 0.0f };
		bool snapped_to_right{ true };

		std::vector<class Person*> people{};
		std::vector<class Car*> vehicles{};

		f32 max_health{ 500.0f };
		f32 current_health{ 350.0f };

		f32 electricity_need{ 50.0f };
		f32 electricity_provided{ 80.0f };

		f32 garbage_capacity{ 10.0f };
		f32 current_garbage{ 0.0f };
		bool is_garbage_truck_on_the_way{ false };
		f32 since_last_garbage_pick_up{ 0.0f };		// TODO: do game time instead?

		f32 water_need{ 20.0f };
		f32 water_provided{ 10.0f };

		f32 water_waste_need{ 8.0f };
		f32 water_waste_provided{ 10.0f };

		u16 crime_reported{ 5 };
		f32 since_last_theft{ -1.0f };				// TODO: do game time instead?

		// prisonars for police station
		// patients for hospitals
		// etc.
		std::vector<class Person*> visitors{};

		std::string name{ "Adam's House" };
	};
}