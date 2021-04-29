#include "PersonManager.h"
namespace Can
{
	PersonManager::PersonManager(GameScene* scene)
		: m_Scene(scene)
	{
	}

	PersonManager::~PersonManager()
	{
	}

	void PersonManager::Update(TimeStep ts)
	{
		for (size_t i = 0; i < m_People.size(); i++)
		{
			Person* p = m_People[i];
			if (p->status == PersonStatus::AtHome)
			{
				p->time_left -= ts;
				if (p->time_left <= 0.0f)
				{
					// go to work work work work
				}

			}
			else if (p->status == PersonStatus::AtWork)
			{
				p->time_left -= ts;
				if (p->time_left <= 0.0f)
				{
					// go to home home home home
				}
			}
			else if (p->status == PersonStatus::Walking)
			{
				// A*
			}
			else if (p->status == PersonStatus::Driving)
			{
				// Arabalı A*
			}
		}
	}

	Person* PersonManager::get_worklessPerson()
	{
		for (Person* p : m_People)
		{
			if (!p->work)
			{
				return p;
			}
		}
		return nullptr;
	}
}