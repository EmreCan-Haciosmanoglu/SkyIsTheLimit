#include "canpch.h"
#include "Car.h"

#include "RoadSegment.h"

namespace Can
{
	Car::Car(
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
	Car::~Car()
	{
		delete object;

	}
}