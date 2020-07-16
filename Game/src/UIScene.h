#pragma once
#include "Can.h"
#include "Can/ECS/Entity.h"

namespace Can
{
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

	private:
		GameApp* m_Parent;
		float m_ZoomLevel;
		float m_AspectRatio;
		Can::Camera::OrthographicCameraController m_CameraController;

		Entity* m_Scene;
	};
}