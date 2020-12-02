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
		GameScene* gameScene = m_Parent->gameScene;
		RoadManager* roadManager = &(gameScene->m_RoadManager);
		TreeManager* treeManager = &(gameScene->m_TreeManager);
		BuildingManager* buildingManager= &(gameScene->m_BuildingManager);
		if (!is_open)
			return;

		ImGui::Begin("Debug");

		ImGui::Text("Construction Type");
		ImGui::BeginChild("Construction Type", ImVec2(0, 110), true);
		if (ImGui::RadioButton("Road", gameScene->e_ConstructionMode == ConstructionMode::Road))
			gameScene->SetConstructionMode(ConstructionMode::Road);
		if (ImGui::RadioButton("Building", gameScene->e_ConstructionMode == ConstructionMode::Building))
			gameScene->SetConstructionMode(ConstructionMode::Building);
		if (ImGui::RadioButton("Tree", gameScene->e_ConstructionMode == ConstructionMode::Tree))
			gameScene->SetConstructionMode(ConstructionMode::Tree);
		if (ImGui::RadioButton("None", gameScene->e_ConstructionMode == ConstructionMode::None))
			gameScene->SetConstructionMode(ConstructionMode::None);
		ImGui::EndChild();

		if (gameScene->e_ConstructionMode == ConstructionMode::Road)
		{
			ImGui::Text("Road Construction Mode");
			ImGui::BeginChild("Road Construction Mode", ImVec2(0, 135), true);
			if (ImGui::RadioButton("Straight", roadManager->GetConstructionMode() == RoadConstructionMode::Straight))
				roadManager->SetConstructionMode(RoadConstructionMode::Straight);
			if (ImGui::RadioButton("CubicCurve", roadManager->GetConstructionMode() == RoadConstructionMode::CubicCurve))
				roadManager->SetConstructionMode(RoadConstructionMode::CubicCurve);
			if (ImGui::RadioButton("Upgrade", roadManager->GetConstructionMode() == RoadConstructionMode::Upgrade))
				roadManager->SetConstructionMode(RoadConstructionMode::Upgrade);
			if (ImGui::RadioButton("Destruction", roadManager->GetConstructionMode() == RoadConstructionMode::Destruct))
				roadManager->SetConstructionMode(RoadConstructionMode::Destruct);
			if (ImGui::RadioButton("None", roadManager->GetConstructionMode() == RoadConstructionMode::None))
				roadManager->SetConstructionMode(RoadConstructionMode::None);
			ImGui::EndChild();
		}
		else if (gameScene->e_ConstructionMode == ConstructionMode::Building)
		{
			ImGui::Text("Building Construction Mode");
			ImGui::BeginChild("Building Construction Mode", ImVec2(0, 110), true);
			if (ImGui::RadioButton("Construct", buildingManager->GetConstructionMode() == BuildingConstructionMode::Construct))
				buildingManager->SetConstructionMode(BuildingConstructionMode::Construct);
			if (ImGui::RadioButton("Upgrade(?)", buildingManager->GetConstructionMode() == BuildingConstructionMode::Upgrade))
				buildingManager->SetConstructionMode(BuildingConstructionMode::Upgrade);
			if (ImGui::RadioButton("Destruction", buildingManager->GetConstructionMode() == BuildingConstructionMode::Destruct))
				buildingManager->SetConstructionMode(BuildingConstructionMode::Destruct);
			if (ImGui::RadioButton("None", buildingManager->GetConstructionMode() == BuildingConstructionMode::None))
				buildingManager->SetConstructionMode(BuildingConstructionMode::None);
			ImGui::EndChild();
		}
		else if (gameScene->e_ConstructionMode == ConstructionMode::Tree)
		{
			ImGui::Text("Tree Construction Mode");
			ImGui::BeginChild("Tree Construction Mode", ImVec2(0, 85), true);
			if (ImGui::RadioButton("Add", treeManager->GetConstructionMode() == TreeConstructionMode::Adding))
				treeManager->SetConstructionMode(TreeConstructionMode::Adding);
			if (ImGui::RadioButton("Remove", treeManager->GetConstructionMode() == TreeConstructionMode::Removing))
				treeManager->SetConstructionMode(TreeConstructionMode::Removing);
			if (ImGui::RadioButton("None", treeManager->GetConstructionMode() == TreeConstructionMode::None))
				treeManager->SetConstructionMode(TreeConstructionMode::None);
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
					bool is_selected = treeManager->GetType() == i;
					std::string text = "Road-";
					text += std::to_string(i);
					if (ImGui::Selectable(text.c_str(), is_selected, 0, ImVec2(0, 25)))
					{
						text.copy(current_road_item.data(), text.size(), 0);
						treeManager->SetType(i);
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
					bool is_selected = buildingManager->GetType() == i;
					std::string text = "Building-";
					text += std::to_string(i);
					if (ImGui::Selectable(text.c_str(), is_selected, 0, ImVec2(0, 25)))
					{
						text.copy(current_building_item.data(), text.size(), 0);
						buildingManager->SetType(i);
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
					bool is_selected = treeManager->GetType()== i;
					std::string text = "Tree-";
					text += std::to_string(i);
					if (ImGui::Selectable(text.c_str(), is_selected, 0, ImVec2(0, 25)))
					{
						text.copy(current_tree_item.data(), text.size(), 0);
						treeManager->SetType(i);
					}
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		}
		ImGui::EndChild();

		if (gameScene->e_ConstructionMode == ConstructionMode::Road)
		{
			ImGui::Text("Road Construction Snap Options");
			ImGui::BeginChild("Road Construction Snap Options", ImVec2(0, 135), true);
			ImGui::Checkbox("Road", &roadManager->snapOptions[0]);
			ImGui::Checkbox("Length", &roadManager->snapOptions[1]);
			ImGui::Checkbox("Height", &roadManager->snapOptions[2]);
			ImGui::Checkbox("Angle", &roadManager->snapOptions[3]);
			ImGui::Checkbox("Grid", &roadManager->snapOptions[4]);
			ImGui::EndChild();
		}
		else if (gameScene->e_ConstructionMode == ConstructionMode::Building)
		{
			ImGui::Text("Building Construction Snap Options");
			ImGui::BeginChild("Building Construction Snap Options", ImVec2(0, 60), true);
			ImGui::Checkbox("Road", &buildingManager->snapOptions[0]);
			ImGui::Checkbox("Building", &buildingManager->snapOptions[1]);
			ImGui::EndChild();
		}

		if (gameScene->e_ConstructionMode == ConstructionMode::Road)
		{
			ImGui::Text("Road Construction Restriction Options");
			ImGui::BeginChild("Road Construction Restriction Options", ImVec2(0, 85), true);
			ImGui::Checkbox("Small angles", &roadManager->restrictions[0]);
			ImGui::Checkbox("Short lengths", &roadManager->restrictions[1]);
			ImGui::Checkbox("Collisions", &roadManager->restrictions[2]);
			ImGui::EndChild();
		}
		else if (gameScene->e_ConstructionMode == ConstructionMode::Building)
		{
			ImGui::Text("Building Construction Restriction Options");
			ImGui::BeginChild("Building Construction Restriction Options", ImVec2(0, 60), true);
			ImGui::Checkbox("Collisions", &buildingManager->restrictions[0]);
			ImGui::Checkbox("Snapping to a road", &buildingManager->restrictions[1]);
			ImGui::EndChild();
		}
		else if (gameScene->e_ConstructionMode == ConstructionMode::Tree)
		{
			ImGui::Text("Tree Construction Restriction Options");
			ImGui::BeginChild("Tree Construction Restriction Options", ImVec2(0, 35), true);
			ImGui::Checkbox("Collisions", &treeManager->restrictions[0]);
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
