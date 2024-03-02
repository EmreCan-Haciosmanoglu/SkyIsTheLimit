#pragma once
#include "Can.h"

namespace Can
{
	class Car
	{
	public:
		Car(Prefab* prefab, u64 type, f32 speed_in_kmh = 100.0f);
		~Car() { delete object; }

	public:
		Object* object;
		u64 type;
		f32 speed_in_kmh;
		class Person* owner = nullptr;
		class Building* building = nullptr;
	};
}
