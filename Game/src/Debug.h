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

		virtual bool OnUpdate(TimeStep ts) override;
		virtual void OnEvent(Event* event) override;
		virtual void OnImGuiRender() override;

		bool OnMousePressed(MouseButtonPressedEvent* event);
		bool OnKeyPressed(KeyPressedEvent* event);
		
	private:
		GameApp* m_Parent;
		bool is_open = true;
	};
}
