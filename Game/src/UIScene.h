#pragma once
#include "Can.h"
#include <glm/gtc/type_ptr.hpp>

namespace Can
{
	bool CheckCollision(entt::entity id, entt::registry* registry, const glm::vec2& clickPosition);
	void Draw(entt::entity id, entt::registry* registry, const glm::vec2& offset);
	
	class GameApp;
	class UIScene : public Layer::Layer
	{
	public:
		UIScene(GameApp* parent);
		virtual ~UIScene();

		virtual void OnAttach() override;
		virtual void OnDetach() override {}

		virtual void OnUpdate(TimeStep ts) override;
		virtual void OnEvent(Event::Event& event) override;

		bool OnMousePressed(Event::MouseButtonPressedEvent& event);

	public:
		Scene* m_Scene;

		ScrollBar* testBar = nullptr;
		ScrollView* testView = nullptr;
		std::array<Panel*, 10> testPanelsForView;
		Ref<Texture2D> testImage;

		Button* m_ButtonRoads = nullptr;
		Button* m_ButtonBuildings = nullptr;
		Button* m_ButtonTrees = nullptr;
		Button* m_ButtonNeeds = nullptr;
		Button* m_ButtonTools = nullptr;

		Panel* m_PanelRoads = nullptr;
		Panel* m_PanelBuildings = nullptr;
		Panel* m_PanelTrees = nullptr;
		Panel* m_PanelNeeds = nullptr;
		Panel* m_PanelTools = nullptr;

		Button* m_ButtonTools_01 = nullptr;
		Button* m_ButtonTools_02 = nullptr;
		Button* m_ButtonTools_03 = nullptr;
		Button* m_ButtonTools_04 = nullptr;
		Button* m_ButtonTools_05 = nullptr;
		Button* m_ButtonTools_06 = nullptr;
		Button* m_ButtonTools_07 = nullptr;
		Button* m_ButtonTools_08 = nullptr;
		Button* m_ButtonTools_09 = nullptr;
		Button* m_ButtonTools_10 = nullptr;
		Button* m_ButtonTools_11 = nullptr;
		Button* m_ButtonTools_12 = nullptr;
		Button* m_ButtonTools_13 = nullptr;
		Button* m_ButtonTools_14 = nullptr;

		Button* m_ButtonNeeds_01 = nullptr;
		Button* m_ButtonNeeds_02 = nullptr;
		Button* m_ButtonNeeds_03 = nullptr;
		Button* m_ButtonNeeds_04 = nullptr;
		Button* m_ButtonNeeds_05 = nullptr;
		Button* m_ButtonNeeds_06 = nullptr;
		Button* m_ButtonNeeds_07 = nullptr;
		Button* m_ButtonNeeds_08 = nullptr;
		Button* m_ButtonNeeds_09 = nullptr;
		Button* m_ButtonNeeds_10 = nullptr;
		Button* m_ButtonNeeds_11 = nullptr;
		Button* m_ButtonNeeds_12 = nullptr;
		Button* m_ButtonNeeds_13 = nullptr;
		Button* m_ButtonNeeds_14 = nullptr;

		std::vector<Button*> m_RoadPanelButtonList;
		std::vector<Button*> m_BuildingPanelButtonList;
		std::vector<Button*> m_TreePanelButtonList;

		GameApp* m_Parent;
	private:
		float m_ZoomLevel;
		float m_AspectRatio;

		Camera::OrthographicCameraController m_CameraController;
	};
}