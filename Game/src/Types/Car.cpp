#include "canpch.h"
#include "Car.h"

#include "RoadSegment.h"

namespace Can
{
	Car::Car(
		Prefab* type,
		s64 roadSegment,
		u64 t_index,
		float speed,
		const v3& position,
		const v3& target,
		const v3& rotation
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
	Car::~Car()
	{
		delete object;

	}
}