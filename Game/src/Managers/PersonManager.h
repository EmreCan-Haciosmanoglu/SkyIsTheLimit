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
	};
}