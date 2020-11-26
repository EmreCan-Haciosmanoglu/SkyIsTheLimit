#pragma once

#include "Can/Renderer/Object.h"

namespace Can
{
	class Building;

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
		BuildingManager();
		~BuildingManager();

		void OnUpdate_Construction(glm::vec3 prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);
		void OnUpdate_Destruction(glm::vec3 prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);

		bool OnMousePressed_Construction();
		bool OnMousePressed_Destruction();

		void SetType(size_t type);

		void SetConstructionMode(BuildingConstructionMode mode);
		
		inline const BuildingConstructionMode GetConstructionMode() const { return m_ConstructionMode; }
		inline BuildingConstructionMode GetConstructionMode() { return m_ConstructionMode; }

		void ResetStates();
	public:

		std::array<bool, 2> snapOptions;
		// 0 : Roads
		// 1 : Buildings

		std::array<bool, 2> restriction;
		// 0 : Collision
		// 1 : Snapping to a road

	private:

		BuildingConstructionMode m_ConstructionMode = BuildingConstructionMode::None;

		size_t m_BuildingType = 0;

		std::vector<Building*> m_Buildings;

		glm::vec3 m_GuidelinePosition = glm::vec3(0.0f);
		glm::vec3 m_GuidelineRotation = glm::vec3(0.0f);

		Road* m_SnappedRoad = nullptr;

		std::vector<Building*>::iterator& m_SelectedBuildingToDestruct = m_Buildings.end();

		Object* m_Guideline = nullptr;

		bool b_ConstructionRestricted = false;
	};
}