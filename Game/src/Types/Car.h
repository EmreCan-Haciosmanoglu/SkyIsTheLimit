#pragma once
#include "Can.h"

namespace Can
{
	class RoadSegment;
	class Person;

	class Car
	{
	public:
		Car(Prefab* prefab, u64 type, f32 speed = 100.0f);
		~Car() { delete object; }

	public:
		Object* object;
		u64 type;
		f32 speed;
		s64 roadSegment = -1;
		Person* owner = nullptr;
	};
}
