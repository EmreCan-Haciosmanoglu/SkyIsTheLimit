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

		ImGui::Begin("Debug");

		ImGui::Text("Construction Type");
		ImGui::BeginChild("Construction Type", ImVec2(0, 60), true);
		if (ImGui::RadioButton("Road", testScene->m_ConstructionMode == ConstructionMode::Road))
			testScene->m_ConstructionMode = ConstructionMode::Road;
		if (ImGui::RadioButton("Building", testScene->m_ConstructionMode == ConstructionMode::Building))
			testScene->m_ConstructionMode = ConstructionMode::Building;
		ImGui::EndChild();

		ImGui::Text("Road Construction Mode");
		ImGui::BeginChild("Road Construction Mode", ImVec2(0, 110), true);
		if (ImGui::RadioButton("Construct", testScene->m_RoadConstructionMode == RoadConstructionMode::Construct))
			testScene->m_RoadConstructionMode = RoadConstructionMode::Construct;
		if (ImGui::RadioButton("Upgrade", testScene->m_RoadConstructionMode == RoadConstructionMode::Upgrade))
			testScene->m_RoadConstructionMode = RoadConstructionMode::Upgrade;
		if (ImGui::RadioButton("Destruction", testScene->m_RoadConstructionMode == RoadConstructionMode::Destruct))
			testScene->m_RoadConstructionMode = RoadConstructionMode::Destruct;
		if (ImGui::RadioButton("None", testScene->m_RoadConstructionMode == RoadConstructionMode::None))
			testScene->m_RoadConstructionMode = RoadConstructionMode::None;
		ImGui::EndChild();

		ImGui::Text("Road Construction Snap Options");
		ImGui::BeginChild("Road Construction Snap Options", ImVec2(0, 110), true);
		ImGui::Checkbox("Road", &testScene->snapOptions[0]);
		ImGui::Checkbox("Length", &testScene->snapOptions[1]);
		ImGui::Checkbox("Angle", &testScene->snapOptions[2]);
		ImGui::Checkbox("Grid", &testScene->snapOptions[3]);
		ImGui::EndChild();

		ImGui::Text("Road Building Restriction Options");
		ImGui::BeginChild("Road Building Restriction Options", ImVec2(0, 135), true);
		ImGui::Checkbox("Small angles", &testScene->roadRestrictionOptions[0]);
		ImGui::Checkbox("Short lengths", &testScene->roadRestrictionOptions[1]);
		ImGui::Checkbox("Collisions with other rads", &testScene->roadRestrictionOptions[2]);
		ImGui::Checkbox("Collisions with buildings", &testScene->roadRestrictionOptions[3]);
		ImGui::Checkbox("Collisions with other objects", &testScene->roadRestrictionOptions[4]);
		ImGui::EndChild();
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
