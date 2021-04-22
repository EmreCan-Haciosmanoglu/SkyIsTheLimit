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
		CarManager* carManager = &(gameScene->m_CarManager);
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
		if (ImGui::RadioButton("Car", gameScene->e_ConstructionMode == ConstructionMode::Car))
			gameScene->SetConstructionMode(ConstructionMode::Car);
		if (ImGui::RadioButton("None", gameScene->e_ConstructionMode == ConstructionMode::None))
			gameScene->SetConstructionMode(ConstructionMode::None);
		ImGui::EndChild();

		if (gameScene->e_ConstructionMode == ConstructionMode::Road)
		{
			ImGui::Text("Road Construction Mode");
			ImGui::BeginChild("Road Construction Mode", ImVec2(0, 160), true);
			if (ImGui::RadioButton("Straight", roadManager->GetConstructionMode() == RoadConstructionMode::Straight))
				roadManager->SetConstructionMode(RoadConstructionMode::Straight);
			if (ImGui::RadioButton("QuadraticCurve", roadManager->GetConstructionMode() == RoadConstructionMode::QuadraticCurve))
				roadManager->SetConstructionMode(RoadConstructionMode::QuadraticCurve);
			if (ImGui::RadioButton("CubicCurve", roadManager->GetConstructionMode() == RoadConstructionMode::CubicCurve))
				roadManager->SetConstructionMode(RoadConstructionMode::CubicCurve);
			if (ImGui::RadioButton("Change", roadManager->GetConstructionMode() == RoadConstructionMode::Change))
				roadManager->SetConstructionMode(RoadConstructionMode::Change);
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
		else if (gameScene->e_ConstructionMode == ConstructionMode::Car)
		{
			ImGui::Text("Car Construction Mode");
			ImGui::BeginChild("Car Construction Mode", ImVec2(0, 85), true);
			if (ImGui::RadioButton("Add", carManager->GetConstructionMode() == CarConstructionMode::Adding))
				carManager->SetConstructionMode(CarConstructionMode::Adding);
			if (ImGui::RadioButton("Remove", carManager->GetConstructionMode() == CarConstructionMode::Removing))
				carManager->SetConstructionMode(CarConstructionMode::Removing);
			if (ImGui::RadioButton("None", carManager->GetConstructionMode() == CarConstructionMode::None))
				carManager->SetConstructionMode(CarConstructionMode::None);
			ImGui::EndChild();
		}

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
