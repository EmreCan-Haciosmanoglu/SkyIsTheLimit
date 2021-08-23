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
		, object(new Object(type, position, rotation))
		, position(position)
		, target(target)
		, fromStart(true)
	{
	}
	
}