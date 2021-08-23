#include "canpch.h"
#include "Person.h"

#include "RoadSegment.h"
#include "Building.h"
#include "Car.h"

namespace Can
{
	Person::Person(
		Prefab* type,
		f32 speed
		
	)
		: speed(speed)
		, object(new Object(type))
	{
		object->enabled = false;
	}

}