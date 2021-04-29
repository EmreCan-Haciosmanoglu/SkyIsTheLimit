#include "canpch.h"
#include "Person.h"

#include "RoadSegment.h"
#include "Building.h"
#include "Car.h"

namespace Can
{
	Person::Person(
		Prefab* type,
		float speed
		
	)
		: speed(speed)
		, object(new Object(type))
	{
		object->enabled = false;
	}

}