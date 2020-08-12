#pragma once
#include "Can.h"
#include "imgui.h"

namespace Can
{
	class GameApp;
	class Debug : public Can::Layer::Layer
	{
	public:
		Debug(GameApp* parent);
		virtual ~Debug() = default;

		virtual void OnAttach() override {};
		virtual void OnDetach() override {};

		virtual void OnUpdate(TimeStep ts) override;
		virtual void OnEvent(Event::Event& event) override;
		virtual void OnImGuiRender() override;

		bool OnMousePressed(Event::MouseButtonPressedEvent& event);
		bool OnKeyPressed(Event::KeyPressedEvent& event);
		
	private:
		GameApp* m_Parent;
	};
}
