#pragma once
#include "Can.h"

namespace Can
{
	class RoadSegment;
	class Person;

	class Building
	{
	public:
		Building(Prefab* prefab, s64 connectedRoadSegment, f32 snappedT, const glm::vec3& position, const glm::vec3& rotation);
		Building(Object* object, s64 connectedRoadSegment, f32 snappedT, const glm::vec3& position, const glm::vec3& rotation);
		~Building() { delete object; }

	public:

		u64 type = 0;
		s64 connectedRoadSegment;
		f32 snappedT = 0.0f;
		u8 capacity = 0;
		std::vector<Person*> residents{};
		std::vector<Person*> workers{};
		Object* object;
		v3 position;
	};
}