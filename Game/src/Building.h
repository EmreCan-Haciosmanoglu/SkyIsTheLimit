#pragma once
#include "Can.h"

namespace Can
{
	struct Car_Park
	{
		v3 offset{ 0.5f, 0.0f, 0.0f};
		f32 rotation_in_degrees = 0.0f;
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

		u64 type = 0;
		s64 connectedRoadSegment = -1;
		s64 snapped_t_index = 0;
		f32 snapped_t = 0.0f;
		u16 capacity = 0;
		bool snapped_to_right = true;
		bool is_home = true;
		std::vector<class Person*> people{};
		std::vector<class Car*> vehicles{};
		Object* object = nullptr;
		v3 position{};
		Car_Park car_park{};
	};
}