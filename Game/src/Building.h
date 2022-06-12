#pragma once
#include "Can.h"

namespace Can
{
	class RoadSegment;
	class Person;

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
		Building(
			Object* object,
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
		u64 snapped_t_index = 0;
		f32 snapped_t = 0.0f;
		u16 capacity = 0;
		bool snapped_to_right = true;
		std::vector<Person*> residents{};
		std::vector<Person*> workers{};
		Object* object = nullptr;
		v3 position{};
	};
}