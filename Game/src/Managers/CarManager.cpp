#include "canpch.h"
#include "CarManager.h"
#include "Scenes/GameScene.h"
#include "Types/RoadSegment.h"
#include "Building.h"
#include "GameApp.h"
#include "Helper.h"

#include "Can/Math.h"

namespace Can
{
	CarManager::CarManager(GameScene* scene)
		: m_Scene(scene)
	{
	}
	void remove_car(Car* car)
	{
		auto& cars = GameScene::ActiveGameScene->m_CarManager.m_Cars;

		auto it = std::find(cars.begin(), cars.end(), car);
		assert(it != cars.end());
		cars.erase(it);

		if (car->building)
		{
			auto& vehicles = car->building->vehicles;
			auto it = std::find(vehicles.begin(), vehicles.end(), car);
			assert(it != vehicles.end());
			vehicles.erase(it);
		}

		delete car;
	}
}