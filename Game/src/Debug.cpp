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
		static TestScene* testScene = m_Parent->testScene;

		ImGui::Begin("Construction Mode");
		if (ImGui::RadioButton("Construct", testScene->m_ConstructionMode == Construct))
			testScene->m_ConstructionMode = Construct;
		if (ImGui::RadioButton("Upgrade", testScene->m_ConstructionMode == Upgrade))
			testScene->m_ConstructionMode = Upgrade;
		if (ImGui::RadioButton("Destruction", testScene->m_ConstructionMode == Destruct))
			testScene->m_ConstructionMode = Destruct;
		if (ImGui::RadioButton("None", testScene->m_ConstructionMode == None))
			testScene->m_ConstructionMode = None;
		ImGui::End();

		ImGui::Begin("Snap Options");
		ImGui::Checkbox("Road Snap", &testScene->snapOptions[0]);
		ImGui::Checkbox("Length Snap", &testScene->snapOptions[1]);
		ImGui::Checkbox("Angle Snap", &testScene->snapOptions[2]);
		ImGui::Checkbox("Grid Snap", &testScene->snapOptions[3]);
		ImGui::End();
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
