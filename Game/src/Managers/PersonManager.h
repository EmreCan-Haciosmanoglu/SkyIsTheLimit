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

		Person* get_unemployed_person();

		class GameScene* m_Scene = nullptr;
		std::vector<Person*> m_People = {};
		std::vector<Person*> get_people_on_the_road();
	};

	void reset_person(Person* p);
	void reset_person_back_to_building_from(Person* p);
	void reset_car_back_to_building_from(Car* c);
	void reset_person_back_to_home(Person* p);
	void remove_person(Person* p);
	class Car* retrive_work_vehicle(Building* work_building);
	void set_person_target(Person* p, const v3& target);
}