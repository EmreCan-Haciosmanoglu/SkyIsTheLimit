#include "canpch.h"
#include "Car.h"

#include "RoadSegment.h"

namespace Can
{
	Car::Car(Prefab* prefab, u64 type, f32 speed)
		: object(new Object(prefab))
		, type(type)
		, speed(speed)
	{}
}