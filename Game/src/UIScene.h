#pragma once
#include "Can.h"

namespace Can
{
	bool CheckCollision(entt::entity id, entt::registry* registry, const glm::vec2& clickPosition);
	void Draw(entt::entity id, entt::registry* registry, const glm::vec2& offset);
	
	class GameApp;
	class UIScene : public Can::Layer::Layer
	{
	public:
		UIScene(GameApp* parent);
		virtual ~UIScene();

		virtual void OnAttach() override {}
		virtual void OnDetach() override {}

		virtual void OnUpdate(Can::TimeStep ts) override;
		virtual void OnEvent(Can::Event::Event& event) override;

		bool OnMousePressed(Can::Event::MouseButtonPressedEvent& event);

	public:
		Scene* m_Scene;

		Button* m_ButtonRoads = nullptr;
		Button* m_ButtonBuildings = nullptr;
		Button* m_ButtonDebug = nullptr;
		Button* m_ButtonNeeds = nullptr;
		Button* m_ButtonTools = nullptr;

		Panel* m_PanelRoads = nullptr;
		Panel* m_PanelBuildings = nullptr;
		Panel* m_PanelDebug = nullptr;
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

	private:
		GameApp* m_Parent;
		float m_ZoomLevel;
		float m_AspectRatio;
		Can::Camera::OrthographicCameraController m_CameraController;
	};
}