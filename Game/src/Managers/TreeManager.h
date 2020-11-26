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

		void SetSelectedTree(size_t index);

		void SetConstructionMode(TreeConstructionMode mode);

		inline const TreeConstructionMode GetConstructionMode() const { return m_ConstructionMode; }
		inline TreeConstructionMode GetConstructionMode() { return m_ConstructionMode; }

		void ResetStates();

	public:

		static std::array<bool, 1> treeRestrictionOptions;
		// 0 : Collision

		TreeConstructionMode m_ConstructionMode = TreeConstructionMode::None;

		size_t m_TreeType = 0;

		static std::vector<Object*> m_Trees;

	private:

		// Tree Adding Transforms
		glm::vec3 m_TreeAddingCoordinate = glm::vec3(-1.0f);

		// Tree Removing Snap
		Object* m_TreeRemovingSnappedTree = nullptr;

		Object* m_TreeGuideline = nullptr;

		bool b_ConstructionRestricted = false;
	};
}