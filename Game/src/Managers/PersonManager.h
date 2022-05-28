#pragma once
#include "Types/Person.h"


namespace Can
{
	class GameScene;

	class PersonManager
	{
	public:
		PersonManager(GameScene* scene);
		~PersonManager();

		void Update(TimeStep ts);

		Person* get_worklessPerson();


		GameScene* m_Scene = nullptr;
		std::vector<Person*> m_People = {};
		std::vector<Person*> walking_people = {};
	};

	void reset_person(Person* p);
	void remove_person(Person* p);
	bool remove_walking_person_from(std::vector<Person*>& walking_people, Person* person);
}