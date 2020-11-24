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
		if (!is_open)
			return;

		ImGui::Begin("Debug");

		ImGui::Text("Construction Type");
		ImGui::BeginChild("Construction Type", ImVec2(0, 85), true);
		if (ImGui::RadioButton("Road", testScene->m_ConstructionMode == ConstructionMode::Road))
			testScene->SetConstructionMode(ConstructionMode::Road);
		if (ImGui::RadioButton("Building", testScene->m_ConstructionMode == ConstructionMode::Building))
			testScene->SetConstructionMode(ConstructionMode::Building);
		if (ImGui::RadioButton("Tree", testScene->m_ConstructionMode == ConstructionMode::Tree))
			testScene->SetConstructionMode(ConstructionMode::Tree);
		ImGui::EndChild();

		if (testScene->m_ConstructionMode == ConstructionMode::Road)
		{
			ImGui::Text("Road Construction Mode");
			ImGui::BeginChild("Road Construction Mode", ImVec2(0, 110), true);
			if (ImGui::RadioButton("Construct", testScene->m_RoadConstructionMode == RoadConstructionMode::Construct))
				testScene->SetRoadConstructionMode(RoadConstructionMode::Construct);
			if (ImGui::RadioButton("Upgrade", testScene->m_RoadConstructionMode == RoadConstructionMode::Upgrade))
				testScene->SetRoadConstructionMode(RoadConstructionMode::Upgrade);
			if (ImGui::RadioButton("Destruction", testScene->m_RoadConstructionMode == RoadConstructionMode::Destruct))
				testScene->SetRoadConstructionMode(RoadConstructionMode::Destruct);
			if (ImGui::RadioButton("None", testScene->m_RoadConstructionMode == RoadConstructionMode::None))
				testScene->SetRoadConstructionMode(RoadConstructionMode::None);
			ImGui::EndChild();
		}
		else if (testScene->m_ConstructionMode == ConstructionMode::Building)
		{
			ImGui::Text("Building Construction Mode");
			ImGui::BeginChild("Building Construction Mode", ImVec2(0, 110), true);
			if (ImGui::RadioButton("Construct", testScene->m_BuildingConstructionMode == BuildingConstructionMode::Construct))
				testScene->SetBuildingConstructionMode(BuildingConstructionMode::Construct);
			if (ImGui::RadioButton("Upgrade(?)", testScene->m_BuildingConstructionMode == BuildingConstructionMode::Upgrade))
				testScene->SetBuildingConstructionMode(BuildingConstructionMode::Upgrade);
			if (ImGui::RadioButton("Destruction", testScene->m_BuildingConstructionMode == BuildingConstructionMode::Destruct))
				testScene->SetBuildingConstructionMode(BuildingConstructionMode::Destruct);
			if (ImGui::RadioButton("None", testScene->m_BuildingConstructionMode == BuildingConstructionMode::None))
				testScene->SetBuildingConstructionMode(BuildingConstructionMode::None);
			ImGui::EndChild();
		}
		else if (testScene->m_ConstructionMode == ConstructionMode::Tree)
		{
			ImGui::Text("Tree Construction Mode");
			ImGui::BeginChild("Tree Construction Mode", ImVec2(0, 85), true);
			if (ImGui::RadioButton("Add", testScene->m_TreeConstructionMode == TreeConstructionMode::Adding))
				testScene->SetTreeConstructionMode(TreeConstructionMode::Adding);
			if (ImGui::RadioButton("Remove", testScene->m_TreeConstructionMode == TreeConstructionMode::Removing))
				testScene->SetTreeConstructionMode(TreeConstructionMode::Removing);
			if (ImGui::RadioButton("None", testScene->m_TreeConstructionMode == TreeConstructionMode::None))
				testScene->SetTreeConstructionMode(TreeConstructionMode::None);
			ImGui::EndChild();
		}

		ImGui::Text("Selected Objects");
		ImGui::BeginChild("Selected Objects", ImVec2(0, 85), true);
		{
			static std::string current_road_item = "";
			if (ImGui::BeginCombo("Selected Road", current_road_item.c_str()))
			{
				for (size_t i = 0; i < m_Parent->roads.size(); i++)
				{
					bool is_selected = testScene->m_RoadConstructionType == i;
					std::string text = "Road-";
					text += std::to_string(i);
					if (ImGui::Selectable(text.c_str(), is_selected, 0, ImVec2(0, 25)))
					{
						text.copy(current_road_item.data(), text.size(), 0);
						testScene->SetSelectedConstructionRoad(i);
					}
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			static std::string current_building_item = "";
			if (ImGui::BeginCombo("Selected Building", current_building_item.c_str()))
			{
				for (size_t i = 0; i < m_Parent->buildings.size(); i++)
				{
					bool is_selected = testScene->m_BuildingType == i;
					std::string text = "Building-";
					text += std::to_string(i);
					if (ImGui::Selectable(text.c_str(), is_selected, 0, ImVec2(0, 25)))
					{
						text.copy(current_building_item.data(), text.size(), 0);
						testScene->SetSelectedConstructionBuilding(i);
					}
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			static std::string current_tree_item = "";
			if (ImGui::BeginCombo("Selected Tree", current_tree_item.c_str()))
			{
				for (size_t i = 0; i < m_Parent->trees.size(); i++)
				{
					bool is_selected = testScene->m_TreeType == i;
					std::string text = "Tree-";
					text += std::to_string(i);
					if (ImGui::Selectable(text.c_str(), is_selected, 0, ImVec2(0, 25)))
					{
						text.copy(current_tree_item.data(), text.size(), 0);
						testScene->SetSelectedTree(i);
					}
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		}
		ImGui::EndChild();

		if (testScene->m_ConstructionMode == ConstructionMode::Road)
		{
			ImGui::Text("Road Construction Snap Options");
			ImGui::BeginChild("Road Construction Snap Options", ImVec2(0, 135), true);
			ImGui::Checkbox("Road", &testScene->roadSnapOptions[0]);
			ImGui::Checkbox("Length", &testScene->roadSnapOptions[1]);
			ImGui::Checkbox("Height", &testScene->roadSnapOptions[2]);
			ImGui::Checkbox("Angle", &testScene->roadSnapOptions[3]);
			ImGui::Checkbox("Grid", &testScene->roadSnapOptions[4]);
			ImGui::EndChild();
		}
		else if (testScene->m_ConstructionMode == ConstructionMode::Building)
		{
			ImGui::Text("Building Construction Snap Options");
			ImGui::BeginChild("Building Construction Snap Options", ImVec2(0, 60), true);
			ImGui::Checkbox("Road", &testScene->buildingSnapOptions[0]);
			ImGui::Checkbox("Building", &testScene->buildingSnapOptions[1]);
			ImGui::EndChild();
		}

		if (testScene->m_ConstructionMode == ConstructionMode::Road)
		{
			ImGui::Text("Road Construction Restriction Options");
			ImGui::BeginChild("Road Construction Restriction Options", ImVec2(0, 85), true);
			ImGui::Checkbox("Small angles", &testScene->roadRestrictionOptions[0]);
			ImGui::Checkbox("Short lengths", &testScene->roadRestrictionOptions[1]);
			ImGui::Checkbox("Collisions", &testScene->roadRestrictionOptions[2]);
			ImGui::EndChild();
		}
		else if (testScene->m_ConstructionMode == ConstructionMode::Building)
		{
			ImGui::Text("Building Construction Restriction Options");
			ImGui::BeginChild("Building Construction Restriction Options", ImVec2(0, 60), true);
			ImGui::Checkbox("Collisions", &testScene->buildingRestrictionOptions[0]);
			ImGui::Checkbox("Snapping to a road", &testScene->buildingRestrictionOptions[1]);
			ImGui::EndChild();
		}
		else if (testScene->m_ConstructionMode == ConstructionMode::Tree)
		{
			ImGui::Text("Tree Construction Restriction Options");
			ImGui::BeginChild("Tree Construction Restriction Options", ImVec2(0, 35), true);
			ImGui::Checkbox("Collisions", &testScene->treeRestrictionOptions[0]);
			ImGui::EndChild();
		}
		ImGui::End();
	}
	bool Debug::OnMousePressed(Event::MouseButtonPressedEvent& event)
	{
		return ImGui::GetIO().WantCaptureMouse;
	}
	bool Debug::OnKeyPressed(Event::KeyPressedEvent& event)
	{
		if (event.GetKeyCode() == CAN_KEY_GRAVE_ACCENT)
			is_open = !is_open;
		return ImGui::GetIO().WantCaptureKeyboard;
	}
}
