#pragma once
#include "Can.h"

namespace Can
{
	class Car
	{
	public:
		~Car() { assert(object != nullptr); delete object; }

		Object* object{ nullptr };
		u64 type{ 0 };
		f32 speed_in_kmh{ 0.0f };
		class Person* owner = nullptr;
		class Building* building = nullptr;
	};
}
