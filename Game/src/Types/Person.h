#pragma once
#include "Can.h"

#include "Types/Transition.h"

namespace Can
{
	enum class PersonStatus
	{
		AtHome,
		AtWork,
		WalkingDead,
		Walking,
		Driving
	};
	
	class Building;
	class Car;
	class Person
	{
	public:
		Person(Prefab* type, f32 speed);
		~Person() { delete object; }

		// Simulation
		s64 road_segment = -1;
		u64 t_index = 0;
		f32 speed = 10.0f;
		f32 t = 0.0f;
		f32 junction_t = 0.0f;
		std::array<v3, 3> drift_points;
		v3 position;
		v3 target;
		std::vector<Transiton*> path{};
		Building* target_building = nullptr;
		bool from_start = false;
		bool from_right = true;
		bool in_junction = false;
		bool heading_to_a_building = false;
		PersonStatus status = PersonStatus::AtHome;
		f32 time_left = 0.0f;
		// 
		Object* object;
		u64 type = 0;
		std::string firstName = "Adam";
		std::string midName = "Madam";
		std::string surName = "Tadaam";
		Building* home = nullptr;
		Building* work = nullptr;
		Car* iCar = nullptr;
	};
}