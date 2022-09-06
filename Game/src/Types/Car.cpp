#include "canpch.h"
#include "Car.h"

#include "RoadSegment.h"

namespace Can
{
	Car::Car(Prefab* prefab, u64 type, f32 speed_in_kmh)
		: object(new Object(prefab))
		, type(type)
		, speed_in_kmh(speed_in_kmh)
	{}
}