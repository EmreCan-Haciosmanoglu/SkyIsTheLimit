#pragma once
#include "Types/Person.h"


namespace Can
{
	class PersonManager
	{
	public:
		PersonManager(class GameScene* scene);
		~PersonManager(){}

		void Update(TimeStep ts);

		Person* get_worklessPerson();


		class GameScene* m_Scene = nullptr;
		std::vector<Person*> m_People = {};
		std::vector<Person*> people_on_the_road = {};
	};

	void reset_person_back_to_building_from(Person* p);
	void reset_person_back_to_home(Person* p);
	void remove_person(Person* p);
	void set_target_and_car_direction(Person* p, Car* car, const v3& target);
	class Car* retrive_work_vehicle(Building* work_building);
	void set_person_target(Person* p, const v3& target);
}