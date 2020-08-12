#include "canpch.h"
#include "Debug.h"
#include "GameApp.h"

namespace Can
{
	Debug::Debug(GameApp* parent)
		:m_Parent(parent)
	{
	}

	void Debug::OnUpdate(Can::TimeStep ts)
	{
	}

	void Debug::OnEvent(Can::Event::Event& event)
	{
		Can::Event::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Event::MouseButtonPressedEvent>(CAN_BIND_EVENT_FN(Debug::OnMousePressed));
		dispatcher.Dispatch<Event::KeyPressedEvent>(CAN_BIND_EVENT_FN(Debug::OnKeyPressed));
	}

	void Debug::OnImGuiRender()
	{

		ImGui::Begin("Construction Mode");
		static bool construct = true;
		static bool upgrade = false;
		static bool destruction = false;
		static bool none = false;
		if (ImGui::RadioButton("Construct", construct))
		{
			construct = true;
			upgrade = false;
			destruction = false;
			none = false;
		}
		if (ImGui::RadioButton("Upgrade", upgrade))
		{
			construct = false;
			upgrade = true;
			destruction = false;
			none = false;
		}
		if (ImGui::RadioButton("Destruction", destruction))
		{
			construct = false;
			upgrade = false;
			destruction = true;
			none = false;
		}
		if (ImGui::RadioButton("None", none))
		{
			construct = false;
			upgrade = false;
			destruction = false;
			none = true;
		}
	}
	bool Debug::OnMousePressed(Event::MouseButtonPressedEvent& event)
	{
		return ImGui::GetIO().WantCaptureMouse;
	}
	bool Debug::OnKeyPressed(Event::KeyPressedEvent& event)
	{
		return ImGui::GetIO().WantCaptureKeyboard;
	}
}
