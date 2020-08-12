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

		virtual void OnUpdate(Can::TimeStep ts) override;
		virtual void OnEvent(Can::Event::Event& event) override;
		virtual void OnImGuiRender() override;
	private:
		GameApp* m_Parent;
	};
}
