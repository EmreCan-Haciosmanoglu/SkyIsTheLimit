#include "canpch.h"
#include "Debug.h"
#include "GameApp.h"

namespace Can
{
	Debug::Debug(GameApp* parent)
		:m_Parent(parent)
	{
	}

	bool Debug::OnUpdate(Can::TimeStep ts)
	{
		return false;
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
		BuildingManager* buildingManager = &(gameScene->m_BuildingManager);
		if (!is_open)
			return;

		ImGui::Begin("DeleteMe");
		ImGui::Text("Elevation: %f", roadManager->m_CurrentElevation);
		ImGui::End();

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

		if (gameScene->e_ConstructionMode == ConstructionMode::Road)
		{
			bool r = roadManager->snapFlags & (u8)RoadSnapOptions::SNAP_TO_ROAD;
			bool l = roadManager->snapFlags & (u8)RoadSnapOptions::SNAP_TO_LENGTH;
			bool h = roadManager->snapFlags & (u8)RoadSnapOptions::SNAP_TO_HEIGHT;
			bool a = roadManager->snapFlags & (u8)RoadSnapOptions::SNAP_TO_ANGLE;
			bool g = roadManager->snapFlags & (u8)RoadSnapOptions::SNAP_TO_GRID;
			ImGui::Text("Road Construction Snap Options");
			ImGui::BeginChild("Road Construction Snap Options", ImVec2(0, 135), true);
			if (ImGui::Checkbox("Road", &r))   roadManager->snapFlags ^= (u8)RoadSnapOptions::SNAP_TO_ROAD;
			if (ImGui::Checkbox("Length", &l)) roadManager->snapFlags ^= (u8)RoadSnapOptions::SNAP_TO_LENGTH;
			if (ImGui::Checkbox("Height", &h)) roadManager->snapFlags ^= (u8)RoadSnapOptions::SNAP_TO_HEIGHT;
			if (ImGui::Checkbox("Angle", &a))  roadManager->snapFlags ^= (u8)RoadSnapOptions::SNAP_TO_ANGLE;
			if (ImGui::Checkbox("Grid", &g))   roadManager->snapFlags ^= (u8)RoadSnapOptions::SNAP_TO_GRID;
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
			bool sm = roadManager->restrictionFlags & (u8)RoadRestrictions::RESTRICT_SMALL_ANGLES;
			bool sh = roadManager->restrictionFlags & (u8)RoadRestrictions::RESTRICT_SHORT_LENGTH;
			bool co = roadManager->restrictionFlags & (u8)RoadRestrictions::RESTRICT_COLLISIONS;
			ImGui::Text("Road Construction Restriction Options");
			ImGui::BeginChild("Road Construction Restriction Options", ImVec2(0, 85), true);
			if (ImGui::Checkbox("Small angles", &sm))  roadManager->restrictionFlags ^= (u8)RoadRestrictions::RESTRICT_SMALL_ANGLES;
			if (ImGui::Checkbox("Short lengths", &sh)) roadManager->restrictionFlags ^= (u8)RoadRestrictions::RESTRICT_SHORT_LENGTH;
			if (ImGui::Checkbox("Collisions", &co))    roadManager->restrictionFlags ^= (u8)RoadRestrictions::RESTRICT_COLLISIONS;
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
		if (event.GetKeyCode() == KeyCode::GraveAccent)
			is_open = !is_open;
		return ImGui::GetIO().WantCaptureKeyboard;
	}
}
