#include "canpch.h"
#include "Person.h"

#include "RoadSegment.h"
#include "Building.h"
#include "Car.h"

namespace Can
{
	Person::Person(
		Prefab* type,
		RoadSegment* roadSegment,
		size_t t_index,
		float speed,
		const glm::vec3& position,
		const glm::vec3& target,
		const glm::vec3& rotation
	)
		: roadSegment(roadSegment)
		, t_index(t_index)
		, speed(speed)
		, object(new Object(type, type, position, glm::vec3(1.0f), rotation, true))
		, position(position)
		, target(target)
		, fromStart(true)
	{
	}

}