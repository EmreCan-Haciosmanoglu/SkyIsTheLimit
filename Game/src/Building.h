#pragma once
#include "Can.h"

namespace Can
{
	struct Car_Park
	{
		v3 offset{ 0.5f, 0.0f, 0.0f };
		f32 rotation_in_degrees{ 0.0f };
	};

	class Building
	{
	public:
		Building(
			Prefab* prefab,
			s64 connectedRoadSegment,
			u64 snapped_t_index,
			f32 snapped_t,
			const glm::vec3& position,
			const glm::vec3& rotation
		);
		Building() {}
		~Building() { delete object; }

	public:

		u64 type{ 0 };
		s64 connectedRoadSegment{ -1 };
		s64 snapped_t_index{ 0 };
		f32 snapped_t{ 0.0f };
		u16 capacity{ 0 };
		bool snapped_to_right{ true };

		bool is_home{ true };
		bool is_hospital{ false };
		bool is_police_station{ false };
		bool is_gmf{ false };

		std::vector<class Person*> people{};
		std::vector<class Car*> vehicles{};
		Object* object{ nullptr };
		v3 position{};
		Car_Park car_park{};

		f32 max_health{ 500.0f };
		f32 current_health{ 350.0f };

		f32 electricity_need{ 50.0f };
		f32 electricity_provided{ 80.0f };

		f32 garbage_capacity{ 100.0f };
		f32 current_garbage{ 23.0f };

		f32 water_need{ 20.0f };
		f32 water_provided{ 10.0f };

		f32 water_waste_need{ 8.0f };
		f32 water_waste_provided{ 10.0f };

		u16 crime_reported{ 5 };

		u16 needed_uneducated{ 4 };
		u16 needed_elementary_school{ 4 };
		u16 needed_high_school{ 5 };
		u16 needed_associate_s{ 5 };
		u16 needed_bachelor_s{ 5 };
		u16 needed_master{ 4 };
		u16 needed_doctorate{ 4 };

		// prisonars for police station
		// patients for hospitals
		// etc.
		std::vector<class Person*> visitors{};
		u16 visitor_capacity{ 30 };
		u16 stay_visitor_capacity{ 20 };

		std::string name{ "Adam's House" };
	};
}