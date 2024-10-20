#pragma once

namespace Can
{
	struct Vehicle_Park
	{
		v3 offset{ 0.5f, 0.0f, 0.0f };
		f32 rotation_in_degrees{ 0.0f };
	};

	enum class Building_Group : u8
	{
		House = 0,
		Residential,
		Commercial,
		Industrial,
		Office,
		Hospital,
		Police_Station,
		Garbage_Collection_Center
	};
	struct Building_Type
	{
		class Prefab* prefab{ nullptr };
		Ref<class Texture2D> thumbnail{ 0 };

		std::string name = "Unnamed Building";
		u16 capacity{ 0 }; // Occupants for Houses, and workers for others
		Building_Group group{ Building_Group::House };
		u16 vehicle_capacity{ 0 }; // Work Cars; Ambulances, Police Cars, Garbage Trucks etc.
		std::vector<Vehicle_Park> vehicle_parks{};
		v3 visiting_spot{};

		// Only for Works. aka not for Houses and Residentials
		u16 needed_uneducated{ 0 };
		u16 needed_elementary_school{ 0 };
		u16 needed_high_school{ 0 };
		u16 needed_associate_s{ 0 };
		u16 needed_bachelor_s{ 0 };
		u16 needed_master{ 0 };
		u16 needed_doctorate{ 0 };

		// prisonars for police station
		// patients for hospitals
		// etc.
		u16 visitor_capacity{ 0 };
		u16 stay_visitor_capacity{ 0 };

		f32 object_width{ 0.01f };
		f32 object_length{ 0.01f };
		f32 object_height{ 0.01f };
	};
}