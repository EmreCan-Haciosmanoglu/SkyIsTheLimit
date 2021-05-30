#pragma once
#include "Can.h"

namespace Can
{
	class RoadSegment;
	class Car
	{
	public:
		Car(
			Prefab* type,
			s64 roadSegment,
			u64 t_index,
			f32 speed,
			const v3& position,
			const v3& target,
			const v3& rotation
		);
		~Car() { delete object; }

	public:
		s64 roadSegment;
		u64 t_index = 0;
		f32 speed = 100.0f;
		f32 t = 0;
		std::array<v3, 3> driftpoints;
		Object* object;
		v3 position;
		v3 target;
		bool fromStart = false;
		bool inJunction = false;
	};
}
