#pragma once

#include "Can/Renderer/Object.h"

namespace Can
{
	class GameScene;
	class RoadSegment;

	enum class CarConstructionMode
	{
		Adding,
		Removing,
		None
	};

	class CarManager
	{

	public:
		CarManager(GameScene* scene);
		~CarManager();

		void OnUpdate(glm::vec3& prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);
		void OnUpdate_Adding(glm::vec3& prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);
		void OnUpdate_Removing(glm::vec3& prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);

		bool OnMousePressed(MouseCode button);
		bool OnMousePressed_Adding();
		bool OnMousePressed_Removing();

		void SetType(size_t type);
		inline size_t GetType() { return m_Type; }

		void SetConstructionMode(CarConstructionMode mode);

		inline const CarConstructionMode GetConstructionMode() const { return m_ConstructionMode; }
		inline CarConstructionMode GetConstructionMode() { return m_ConstructionMode; }

		inline const std::vector<Object*>& GetCars() const { return m_Cars; }
		inline std::vector<Object*>& GetCars() { return m_Cars; }

		void ResetStates();

	private:
		GameScene* m_Scene = nullptr;

		CarConstructionMode m_ConstructionMode = CarConstructionMode::None;

		size_t m_Type = 0;

		std::vector<Object*> m_Cars = {};

		RoadSegment* m_SnappedRoadSegment = nullptr;
		

		std::vector<Object*>::iterator& m_SelectedCarToRemove = m_Cars.end();

		Object* m_Guideline = nullptr;
	};
}