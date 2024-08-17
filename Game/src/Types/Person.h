#pragma once
#include "Can.h"
#include "EducationLevel.h"
#include "Profession.h"

namespace Can
{
	enum class PersonStatus
	{
		AtHome,
		AtWork,
		WalkingDead,
		Walking,
		Driving,
		DrivingForWork,
		Arrested,
		InJail,
		Patrolling,
		Responding
	};

	struct Transition;
	class Building;
	class Car;
	class Person
	{
	public:
		Person() {}
		Person(Prefab* type, f32 speed_in_kmh);
		~Person() { delete object; }

		u64 type{ 0 };
		Object* object{ nullptr };
		s64 road_segment{ -1 };
		s64 road_node{ -1 };
		f32 speed_in_kmh{ 10.0f };
		v3 position{};
		v3 target{};
		PersonStatus status{ PersonStatus::AtHome };
		EducationLevel education{ EducationLevel::Uneducated };
		Profession profession{ Profession::Unemployed };
		std::vector<Transition*> path{};

		Building* path_end_building{ nullptr };
		Building* path_start_building{ nullptr };

		bool drove_in_work{ false };
		bool from_right{ false };
		bool heading_to_a_building{ false };
		bool heading_to_a_car{ false };
		f32 time_left{ 0.0f };
		std::string firstName{ "Adam" };
		std::string midName{ "Madam" };
		std::string surName{ "Tadaam" };
		Building* home{ nullptr };
		Building* work{ nullptr };
		Car* car{ nullptr };
		Car* car_driving{ nullptr }; // currently driven vehicle

		f32 happiness{ 0.7f };
	};
}