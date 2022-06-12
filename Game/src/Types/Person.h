#pragma once
#include "Can.h"


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
	
	struct Transition;
	class Building;
	class Car;
	class Person
	{
	public:
		Person(Prefab* type, f32 speed);
		~Person() { delete object; }

		s64 road_segment = -1;
		s64 road_node = -1;
		f32 speed = 10.0f;
		v3 position;
		v3 target;
		std::vector<Transition*> path{};
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