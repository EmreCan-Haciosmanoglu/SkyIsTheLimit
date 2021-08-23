#pragma once
#include "Can.h"

namespace Can
{
	class RoadSegment;
	class Person;

	class Building
	{
	public:
		Building(Prefab* prefab, s64 connectedRoadSegment, float snappedT, const glm::vec3& position, const glm::vec3& rotation);
		Building(Object* object, s64 connectedRoadSegment, float snappedT, const glm::vec3& position, const glm::vec3& rotation);
		~Building();

	public:

		s64 connectedRoadSegment;
		float snappedT = 0.0f;
		u8 capacity = 0;
		std::vector<Person*> residents;
		Object* object;
		glm::vec3 position;
	};
}