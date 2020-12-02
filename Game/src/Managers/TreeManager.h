#pragma once

#include "Can/Renderer/Object.h"

namespace Can
{
	class GameScene;

	enum class TreeConstructionMode
	{
		Adding,
		Removing,
		None
	};

	class TreeManager
	{
	public:
		TreeManager(GameScene* scene);
		~TreeManager();

		void OnUpdate(glm::vec3 prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);
		void OnUpdate_Adding(glm::vec3 prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);
		void OnUpdate_Removing(glm::vec3 prevLocation, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);

		bool OnMousePressed(MouseCode button);
		bool OnMousePressed_Adding();
		bool OnMousePressed_Removing();

		void SetType(size_t type);
		inline size_t GetType() { return m_Type; }

		void SetConstructionMode(TreeConstructionMode mode);

		inline const TreeConstructionMode GetConstructionMode() const { return m_ConstructionMode; }
		inline TreeConstructionMode GetConstructionMode() { return m_ConstructionMode; }

		inline const std::vector<Object*>& GetTrees() const { return m_Trees; }
		inline std::vector<Object*>& GetTrees() { return m_Trees; }

		void ResetStates();

	public:

		std::array<bool, 1> restrictions = { true };
		// 0 : Collision

	private:
		GameScene* m_Scene = nullptr;

		TreeConstructionMode m_ConstructionMode = TreeConstructionMode::None;

		size_t m_Type = 0;

		std::vector<Object*> m_Trees = {};

		glm::vec3 m_GuidelinePosition = glm::vec3(0.0f);

		std::vector<Object*>::iterator& m_SelectedTreeToRemove = m_Trees.end();

		Object* m_Guideline = nullptr;

		bool b_AddingRestricted = false;
	};
}