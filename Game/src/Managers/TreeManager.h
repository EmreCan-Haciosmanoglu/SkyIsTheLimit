#pragma once

#include "Can/Renderer/Object.h"

namespace Can
{
	enum class TreeConstructionMode
	{
		Adding,
		Removing,
		None
	};


	class TreeManager
	{
	public:
		TreeManager();
		~TreeManager();

		void OnUpdate_Adding(glm::vec3 prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);
		void OnUpdate_Removing(glm::vec3 prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);

		bool OnMousePressed_Adding();
		bool OnMousePressed_Removing();

		void SetType(size_t type);

		void SetConstructionMode(TreeConstructionMode mode);

		inline const TreeConstructionMode GetConstructionMode() const { return m_ConstructionMode; }
		inline TreeConstructionMode GetConstructionMode() { return m_ConstructionMode; }

		void ResetStates();

	public:

		std::array<bool, 1> restrictions = { true };
		// 0 : Collision

	private:

		TreeConstructionMode m_ConstructionMode = TreeConstructionMode::None;

		size_t m_Type = 0;

		std::vector<Object*> m_Trees;

		glm::vec3 m_GuidelinePosition = glm::vec3(0.0f);

		std::vector<Object*>::iterator& m_SelectedTreeToRemove = m_Trees.end();

		Object* m_Guideline = nullptr;

		bool b_AddingRestricted = false;
	};
}