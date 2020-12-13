#pragma once

#include "Can/Renderer/Object.h"

namespace Can
{
	class GameScene;
	class Building;
	class RoadSegment;

	enum class BuildingConstructionMode
	{
		Construct,
		Upgrade,
		Destruct,
		None
	};

	class BuildingManager
	{
	public:
		BuildingManager(GameScene* scene);
		~BuildingManager();

		void OnUpdate(glm::vec3 prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);
		void OnUpdate_Construction(glm::vec3 prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);
		void OnUpdate_Destruction(glm::vec3 prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);

		bool OnMousePressed(MouseCode button);
		bool OnMousePressed_Construction();
		bool OnMousePressed_Destruction();

		void SetType(size_t type);
		inline size_t GetType() { return m_Type; }

		void SetConstructionMode(BuildingConstructionMode mode);
		
		inline const BuildingConstructionMode GetConstructionMode() const { return m_ConstructionMode; }
		inline BuildingConstructionMode GetConstructionMode() { return m_ConstructionMode; }

		inline const std::vector<Building*>& GetBuildings() const { return m_Buildings; }
		inline std::vector<Building*>& GetBuildings() { return m_Buildings; }

		void ResetStates();
	public:

		std::array<bool, 2> snapOptions;
		// 0 : Roads
		// 1 : Buildings

		std::array<bool, 2> restrictions;
		// 0 : Collision
		// 1 : Snapping to a road

	private:
		GameScene* m_Scene = nullptr;

		BuildingConstructionMode m_ConstructionMode = BuildingConstructionMode::None;

		size_t m_Type = 0;

		std::vector<Building*> m_Buildings;

		glm::vec3 m_GuidelinePosition = glm::vec3(0.0f);
		glm::vec3 m_GuidelineRotation = glm::vec3(0.0f);

		RoadSegment* m_SnappedRoadSegment = nullptr;

		std::vector<Building*>::iterator& m_SelectedBuildingToDestruct = m_Buildings.end();

		Object* m_Guideline = nullptr;

		bool b_ConstructionRestricted = false;
	};
}