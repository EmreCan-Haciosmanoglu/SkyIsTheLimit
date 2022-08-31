#pragma once
#include "Types/Person.h"


namespace Can
{
	class GameScene;

	class PersonManager
	{
	public:
		PersonManager(GameScene* scene);
		~PersonManager(){}

		void Update(TimeStep ts);

		Person* get_worklessPerson();


		GameScene* m_Scene = nullptr;
		std::vector<Person*> m_People = {};
		std::vector<Person*> walking_people = {};
	};

	void reset_person_back_to_building_from(Person* p);
	void remove_person(Person* p);
	void set_target_and_car_direction(Person* p, const v3& target);
	void set_person_target(Person* p, const v3& target);
}