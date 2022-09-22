#pragma once
#include "Can.h"

namespace Can
{
	class RoadSegment;
	class Person;

	class Car
	{
	public:
		Car(Prefab* prefab, u64 type, f32 speed_in_kmh = 100.0f);
		~Car() { delete object; }

	public:
		Object* object;
		u64 type;
		f32 speed_in_kmh;
		Person* owner = nullptr;
	};
}
