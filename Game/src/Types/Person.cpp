#include "canpch.h"
#include "Person.h"

#include "Types/Transition.h"
#include "Types/RoadSegment.h"
#include "Building.h"
#include "Types/Car.h"

namespace Can
{
	Person::Person(Prefab* type, f32 speed_in_kmh)
		: speed_in_kmh(speed_in_kmh)
		, object(new Object(type))
	{
		object->enabled = false;
	}

}