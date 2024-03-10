#pragma once
#include "Can.h"

namespace Can
{
	enum class Car_Type: u8 {
		Personal = 0,
		Work
	};
	class Car
	{
	public:
		Car(Prefab* prefab, u64 type, f32 speed_in_kmh = 100.0f);
		~Car() { delete object; }

	public:
		Object* object;
		u64 type;
		f32 speed_in_kmh;
		Car_Type car_type = Car_Type::Personal;
		class Person* owner = nullptr;
		class Building* building = nullptr;
	};
}
