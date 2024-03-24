#pragma once

#include "Can/Renderer/Prefab.h"

namespace Can
{
	enum class Car_Type : u8 {
		Personal = 0,
		Work
	};

	struct Vehicle_Type
	{
		std::string name = "Unnamed Vehicle";

		Prefab* prefab{ nullptr };
		Ref<Texture2D> thumbnail{ 0 };
		f32 speed_range_min{ 0.f };
		f32 speed_range_max{ 0.f };
		u16 operator_count{ 0 };
		u16 passenger_limit{ 0 };
		Car_Type type{ Car_Type::Personal };

		f32 object_width{ 0.01f };
		f32 object_length{ 0.01f };
	};
}