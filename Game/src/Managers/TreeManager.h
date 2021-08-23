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

		void OnUpdate(v3& prevLocation, const v3& cameraPosition, const v3& cameraDirection);
		void OnUpdate_Adding(v3& prevLocation, const v3& cameraPosition, const v3& cameraDirection);
		void OnUpdate_Removing(v3& prevLocation, const v3& cameraPosition, const v3& cameraDirection);

		bool OnMousePressed(MouseCode button);
		bool OnMousePressed_Adding();
		bool OnMousePressed_Removing();

		void SetType(u64 type);
		inline u64 GetType() { return m_Type; }

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

		u64 m_Type = 0;

		std::vector<Object*> m_Trees = {};

		v3 m_GuidelinePosition = v3(0.0f);

		std::vector<Object*>::iterator& m_SelectedTreeToRemove = m_Trees.end();

		Object* m_Guideline = nullptr;

		bool b_AddingRestricted = false;
	};
}