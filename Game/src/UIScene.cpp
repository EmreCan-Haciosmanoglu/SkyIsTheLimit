#include "canpch.h"
#include "UIScene.h"
#include "GameApp.h"
#include "Helper.h"

namespace Can
{
	UIScene::UIScene(GameApp* parent)
		: m_Parent(parent)
		, m_ZoomLevel(10.0f)
		, m_AspectRatio(16.0f / 9.0f)
		, m_Scene(new Scene())
	{
		init_orthographic_camera_controller(m_CameraController, m_AspectRatio, m_ZoomLevel, false);
		auto& ui = parent->gameScene->ui_layer;

		float width = m_AspectRatio * m_ZoomLevel * 2.0f;
		float height = m_ZoomLevel * 2.0f;
		Application& app = Application::Get();
		unsigned int w = app.GetWindow().GetWidth();
		unsigned int h = app.GetWindow().GetHeight();

		ButtonConstructorParameters buttonPauseParams = ButtonConstructorParameters{
			m_Scene->m_Registry,
			m_Scene->entityID,
			v3{width - 4.0f, 0.5f, 0.2f},
			v2(0.8f),
			v4(1.0f),
			m_Parent->pauseTexture,
			[this]() {
				entt::registry& mainRegistry = m_Scene->m_Registry;
				GameScene* gameScene = m_Parent->gameScene;

				mainRegistry.get<SpriteRendererComponent>(m_ButtonPause->entityID).border = true;
				mainRegistry.get<SpriteRendererComponent>(m_ButtonNormal->entityID).border = false;
				mainRegistry.get<SpriteRendererComponent>(m_Button2Times->entityID).border = false;
				mainRegistry.get<SpriteRendererComponent>(m_Button4Times->entityID).border = false;
				gameScene->SetSpeedMode(SpeedMode::Pause);
			},
			0.1f,
			false,
			v4{ 255.0f / 255.0f, 166.0f / 255.0f, 158.0f / 255.0f, 1.0f },
		};
		m_ButtonPause = new Button(buttonPauseParams);
		ButtonConstructorParameters buttonNormalParams = ButtonConstructorParameters{
			m_Scene->m_Registry,
			m_Scene->entityID,
			v3{width - 3.0f, 0.5f, 0.2f},
			v2(0.8f),
			v4(1.0f),
			m_Parent->normalSpeedTexture,
			[this]() {
				entt::registry& mainRegistry = m_Scene->m_Registry;
				GameScene* gameScene = m_Parent->gameScene;

				mainRegistry.get<SpriteRendererComponent>(m_ButtonPause->entityID).border = false;
				mainRegistry.get<SpriteRendererComponent>(m_ButtonNormal->entityID).border = true;
				mainRegistry.get<SpriteRendererComponent>(m_Button2Times->entityID).border = false;
				mainRegistry.get<SpriteRendererComponent>(m_Button4Times->entityID).border = false;
				gameScene->SetSpeedMode(SpeedMode::Normal);
			},
			0.1f,
			true,
			v4{ 255.0f / 255.0f, 166.0f / 255.0f, 158.0f / 255.0f, 1.0f },
		};
		m_ButtonNormal = new Button(buttonNormalParams);
		ButtonConstructorParameters button2TimesParams = ButtonConstructorParameters{
			m_Scene->m_Registry,
			m_Scene->entityID,
			v3{width - 2.0f, 0.5f, 0.2f},
			v2(0.8f),
			v4(1.0f),
			m_Parent->twoTimesSpeedTexture,
			[this]() {
				entt::registry& mainRegistry = m_Scene->m_Registry;
				GameScene* gameScene = m_Parent->gameScene;

				mainRegistry.get<SpriteRendererComponent>(m_ButtonPause->entityID).border = false;
				mainRegistry.get<SpriteRendererComponent>(m_ButtonNormal->entityID).border = false;
				mainRegistry.get<SpriteRendererComponent>(m_Button2Times->entityID).border = true;
				mainRegistry.get<SpriteRendererComponent>(m_Button4Times->entityID).border = false;
				gameScene->SetSpeedMode(SpeedMode::TwoX);
			},
			0.1f,
			false,
			v4{ 255.0f / 255.0f, 166.0f / 255.0f, 158.0f / 255.0f, 1.0f },
		};
		m_Button2Times = new Button(button2TimesParams);
		ButtonConstructorParameters button4TimesParams = ButtonConstructorParameters{
			m_Scene->m_Registry,
			m_Scene->entityID,
			v3{width - 1.0f, 0.5f, 0.2f},
			v2(0.8f),
			v4(1.0f),
			m_Parent->fourTimesSpeedTexture,
			[this]() {
				entt::registry& mainRegistry = m_Scene->m_Registry;
				GameScene* gameScene = m_Parent->gameScene;

				mainRegistry.get<SpriteRendererComponent>(m_ButtonPause->entityID).border = false;
				mainRegistry.get<SpriteRendererComponent>(m_ButtonNormal->entityID).border = false;
				mainRegistry.get<SpriteRendererComponent>(m_Button2Times->entityID).border = false;
				mainRegistry.get<SpriteRendererComponent>(m_Button4Times->entityID).border = true;
				gameScene->SetSpeedMode(SpeedMode::FourX);
			},
			0.1f,
			false,
			v4{ 255.0f / 255.0f, 166.0f / 255.0f, 158.0f / 255.0f, 1.0f },
		};
		m_Button4Times = new Button(button4TimesParams);
		ButtonConstructorParameters buttonSaveParams = ButtonConstructorParameters{
			m_Scene->m_Registry,
			m_Scene->entityID,
			v3{width - 5.0f, 0.5f,0.2f },
			v2(0.8f),
			v4(1.0f),
			m_Parent->saveTexture,
			[this]() {
				GameScene* gameScene = m_Parent->gameScene;

				gameScene->save_the_game();
			},
			0.1f,
			false,
			v4{ 255.0f / 255.0f, 166.0f / 255.0f, 158.0f / 255.0f, 1.0f },
		};
		m_ButtonSave = new Button(buttonSaveParams);

		ButtonConstructorParameters buttonRoadsParams = ButtonConstructorParameters{
			m_Scene->m_Registry,
			m_Scene->entityID,
			v3{ 1.0f, height - 6.8f, 0.0011f },
			v2{ 3.0f, 1.0f},
			v4{ 221.0f / 255.0f, 255.0f / 255.0f, 247.0f / 255.0f, 1.0f },
			nullptr,
			[this, height, &ui]() {
				if (!Input::IsMouseButtonPressed(MouseCode::Button0))
					return;
				entt::registry& mainRegistry = this->m_Scene->m_Registry;

				entt::entity panelRoadsID = this->m_PanelRoads->entityID;
				entt::entity panelTreesID = this->m_PanelTrees->entityID;

				if (!ui.draw_building_panel && mainRegistry.has<HiddenComponent>(panelTreesID))
				{
					entt::entity buttonRoadsID = this->m_ButtonRoads->entityID;
					entt::entity buttonBuildingsID = this->m_ButtonBuildings->entityID;
					entt::entity buttonTreesID = this->m_ButtonTrees->entityID;

					TransformComponent& transformR = mainRegistry.get<TransformComponent>(buttonRoadsID);
					TransformComponent& transformB = mainRegistry.get<TransformComponent>(buttonBuildingsID);
					TransformComponent& transformT = mainRegistry.get<TransformComponent>(buttonTreesID);

					v3 movement = { 0.0f, 5.2f, 0.0f };

					if (mainRegistry.has<HiddenComponent>(panelRoadsID))
					{
						mainRegistry.remove<HiddenComponent>(panelRoadsID);
						transformR.Position -= movement;
						transformB.Position -= movement;
						transformT.Position -= movement;
					}
					else
					{
						mainRegistry.emplace<HiddenComponent>(panelRoadsID);
						transformR.Position += movement;
						transformB.Position += movement;
						transformT.Position += movement;
					}
				}
				else
				{
					mainRegistry.remove<HiddenComponent>(panelRoadsID);
					ui.draw_building_panel = false;
					mainRegistry.emplace_or_replace<HiddenComponent>(panelTreesID);
				}
			},
			0.3f
		};
		m_ButtonRoads = new Button(buttonRoadsParams);
		ButtonConstructorParameters buttonBuildingsParams = ButtonConstructorParameters{
			m_Scene->m_Registry,
			m_Scene->entityID,
			v3{ 5.0f, height - 6.8f, 0.0011f },
			v2{ 3.0f, 1.0f },
			v4{ 255.0f / 255.0f, 166.0f / 255.0f, 158.0f / 255.0f, 1.0f },
			nullptr,
			[this, height, &ui]() {
				if (!Input::IsMouseButtonPressed(MouseCode::Button0))
					return;
				entt::registry& mainRegistry = this->m_Scene->m_Registry;

				entt::entity panelRoadsID = this->m_PanelRoads->entityID;
				entt::entity panelTreesID = this->m_PanelTrees->entityID;

				if (
					mainRegistry.has<HiddenComponent>(panelRoadsID) &&
					mainRegistry.has<HiddenComponent>(panelTreesID)
					)
				{
					entt::entity buttonRoadsID = this->m_ButtonRoads->entityID;
					entt::entity buttonBuildingsID = this->m_ButtonBuildings->entityID;
					entt::entity buttonTreesID = this->m_ButtonTrees->entityID;

					TransformComponent& transformR = mainRegistry.get<TransformComponent>(buttonRoadsID);
					TransformComponent& transformB = mainRegistry.get<TransformComponent>(buttonBuildingsID);
					TransformComponent& transformT = mainRegistry.get<TransformComponent>(buttonTreesID);

					v3 movement = { 0.0f, 5.2f, 0.0f };

					if (ui.draw_building_panel)
					{
						ui.draw_building_panel = false;
						transformR.Position += movement;
						transformB.Position += movement;
						transformT.Position += movement;
					}
					else
					{
						ui.draw_building_panel = true;
						transformR.Position -= movement;
						transformB.Position -= movement;
						transformT.Position -= movement;
					}
				}
				else
				{
					ui.draw_building_panel = true;
					mainRegistry.emplace_or_replace<HiddenComponent>(panelRoadsID);
					mainRegistry.emplace_or_replace<HiddenComponent>(panelTreesID);
				}
			},
			0.3f
		};
		m_ButtonBuildings = new Button(buttonBuildingsParams);
		ButtonConstructorParameters buttonTreesParams = ButtonConstructorParameters{
			m_Scene->m_Registry,
			m_Scene->entityID,
			v3{ 9.0f, height - 6.8f, 0.0011f },
			v2{ 3.0f, 1.0f },
			v4{ 69.0f / 255.0f, 123.0f / 255.0f, 157.0f / 255.0f, 1.0f },
			nullptr,
			[this, height, &ui]() {
				if (!Input::IsMouseButtonPressed(MouseCode::Button0))
					return;
				entt::registry& mainRegistry = this->m_Scene->m_Registry;

				entt::entity panelRoadsID = this->m_PanelRoads->entityID;
				entt::entity panelTreesID = this->m_PanelTrees->entityID;

				if (mainRegistry.has<HiddenComponent>(panelRoadsID) && !ui.draw_building_panel )
				{
					entt::entity buttonRoadsID = this->m_ButtonRoads->entityID;
					entt::entity buttonBuildingsID = this->m_ButtonBuildings->entityID;
					entt::entity buttonTreesID = this->m_ButtonTrees->entityID;

					TransformComponent& transformR = mainRegistry.get<TransformComponent>(buttonRoadsID);
					TransformComponent& transformB = mainRegistry.get<TransformComponent>(buttonBuildingsID);
					TransformComponent& transformT = mainRegistry.get<TransformComponent>(buttonTreesID);

					v3 movement = { 0.0f, 5.2f, 0.0f };

					if (mainRegistry.has<HiddenComponent>(panelTreesID))
					{
						mainRegistry.remove<HiddenComponent>(panelTreesID);
						transformR.Position -= movement;
						transformB.Position -= movement;
						transformT.Position -= movement;
					}
					else
					{
						mainRegistry.emplace<HiddenComponent>(panelTreesID);
						transformR.Position += movement;
						transformB.Position += movement;
						transformT.Position += movement;
					}
				}
				else
				{
					mainRegistry.remove<HiddenComponent>(panelTreesID);
					mainRegistry.emplace_or_replace<HiddenComponent>(panelRoadsID);
					ui.draw_building_panel = false;
				}
			},
			0.3f
		};
		m_ButtonTrees = new Button(buttonTreesParams);

		ButtonConstructorParameters buttonNeedsParams = ButtonConstructorParameters{
			m_Scene->m_Registry,
			m_Scene->entityID,
			v3{ 0.75f, 0.5f, 0.0011f },
			v2{ 1.0f, 1.0f },
			v4{ 170.0f / 255.0f, 68.0f / 255.0f, 101.0f / 255.0f, 1.0f },
			nullptr,
			[this]() {
				if (!Input::IsMouseButtonPressed(MouseCode::Button0))
					return;
				entt::registry& mainRegistry = this->m_Scene->m_Registry;

				entt::entity panelNeedsID = this->m_PanelNeeds->entityID;
				entt::entity panelToolsID = this->m_PanelTools->entityID;

				if (mainRegistry.has<HiddenComponent>(panelNeedsID))
				{
					mainRegistry.remove<HiddenComponent>(panelNeedsID);
					mainRegistry.emplace_or_replace<HiddenComponent>(panelToolsID);
				}
				else
				{
					mainRegistry.emplace<HiddenComponent>(panelNeedsID);
				}
			},
			0.2f
		};
		m_ButtonNeeds = new Button(buttonNeedsParams);
		ButtonConstructorParameters buttonToolsParams = ButtonConstructorParameters{
			m_Scene->m_Registry,
			m_Scene->entityID,
			v3{ 2.0f, 0.5f, 0.0011f },
			v2{ 1.0f, 1.0f },
			v4{ 147.0f / 255.0f, 225.0f / 255.0f, 216.0f / 255.0f, 1.0f },
			nullptr,
			[this]() {
				if (!Input::IsMouseButtonPressed(MouseCode::Button0))
					return;
				entt::registry& mainRegistry = this->m_Scene->m_Registry;

				entt::entity panelNeedsID = this->m_PanelNeeds->entityID;
				entt::entity panelToolsID = this->m_PanelTools->entityID;

				if (mainRegistry.has<HiddenComponent>(panelToolsID))
				{
					mainRegistry.remove<HiddenComponent>(panelToolsID);
					mainRegistry.emplace_or_replace<HiddenComponent>(panelNeedsID);
				}
				else
				{
					mainRegistry.emplace<HiddenComponent>(panelToolsID);
				}
			},
			0.2f
		};
		m_ButtonTools = new Button(buttonToolsParams);

		std::vector<entt::entity> buttonList = {
			m_ButtonPause->entityID,
			m_ButtonNormal->entityID,
			m_Button2Times->entityID,
			m_Button4Times->entityID,
			m_ButtonSave->entityID,
			m_ButtonRoads->entityID,
			m_ButtonBuildings->entityID,
			m_ButtonTrees->entityID,
			m_ButtonNeeds->entityID,
			m_ButtonTools->entityID
		};
		m_Scene->m_Registry.emplace<ChildrenComponent>(m_Scene->entityID, buttonList);

		PanelConstructorParameters panelRoadsParams = PanelConstructorParameters{
			m_Scene->m_Registry,
			m_ButtonRoads->entityID,
			v3{ 0.5f, height - 5.4f, 0.001f },
			v2{ width - 1.0f, 5.2f },
			v4{ 221.0f / 255.0f, 255.0f / 255.0f, 247.0f / 255.0f, 1.0f },
			nullptr,
			[]() {std::cout << "You clicked the panel that is for Roads!" << std::endl; },
			0.1f
		};
		m_PanelRoads = new Panel(panelRoadsParams);
		//m_Scene->m_Registry.emplace<HiddenComponent>(m_PanelRoads->entityID);
		m_Scene->m_Registry.emplace<ChildrenComponent>(m_ButtonRoads->entityID, std::vector<entt::entity>{ m_PanelRoads->entityID });

		PanelConstructorParameters panelTreesParams = PanelConstructorParameters{
					m_Scene->m_Registry,
					m_ButtonTrees->entityID,
					v3{ 0.5f, height - 5.4f, 0.001f },
					v2{ width - 1.0f, 5.2f },
					v4{ 69.0f / 255.0f, 123.0f / 255.0f, 157.0f / 255.0f, 1.0f },
					nullptr,
					[]() {std::cout << "You clicked the panel that is for Trees!" << std::endl; },
					0.1f
		};
		m_PanelTrees = new Panel(panelTreesParams);
		m_Scene->m_Registry.emplace<HiddenComponent>(m_PanelTrees->entityID);
		m_Scene->m_Registry.emplace<ChildrenComponent>(m_ButtonTrees->entityID, std::vector<entt::entity>{ m_PanelTrees->entityID });

		PanelConstructorParameters panelNeedsParams = PanelConstructorParameters{
					m_Scene->m_Registry,
					m_ButtonTools->entityID,
					v3{ 0.5f, 1.75f, 0.001f },
					v2{ 2.75f, 9.0f },
					v4{ 170.0f / 255.0f, 68.0f / 255.0f, 101.0f / 255.0f, 1.0f },
					nullptr,
					[]() {std::cout << "You clicked the panel that is for Needs!" << std::endl; },
					0.1f
		};
		m_PanelNeeds = new Panel(panelNeedsParams);
		m_Scene->m_Registry.emplace<HiddenComponent>(m_PanelNeeds->entityID);
		m_Scene->m_Registry.emplace<ChildrenComponent>(m_ButtonNeeds->entityID, std::vector<entt::entity>{ m_PanelNeeds->entityID });

		PanelConstructorParameters panelToolsParams = PanelConstructorParameters{
					m_Scene->m_Registry,
					m_ButtonTools->entityID,
					v3{ 0.5f, 1.75f, 0.001f },
					v2{ 2.75f, 9.0f },
					v4{ 147.0f / 255.0f, 225.0f / 255.0f, 216.0f / 255.0f, 1.0f },
					nullptr,
					[]() {std::cout << "You clicked the panel that is for Tools!" << std::endl; },
					0.1f
		};
		m_PanelTools = new Panel(panelToolsParams);
		m_Scene->m_Registry.emplace<HiddenComponent>(m_PanelTools->entityID);
		m_Scene->m_Registry.emplace<ChildrenComponent>(m_ButtonTools->entityID, std::vector<entt::entity>{ m_PanelTools->entityID });

		ScrollViewConstructorParameters scrollViewRoadsParams = ScrollViewConstructorParameters{
				m_Scene->m_Registry,
				m_PanelRoads->entityID,
				v3{ width * 0.25f - 0.6f, height - 5.3f, 0.01f },
				v2{ width * 0.75f, 5.0f},
				v4{ 201.0f / 255.0f, 235.0f / 255.0f, 227.0f / 255.0f, 1.0f },
				2,
				[]() {std::cout << "You clicked to the ScrollView For roads!" << std::endl; }
		};
		ScrollBarConstructorParameters scrollBarRoadsParams = ScrollBarConstructorParameters{
				m_Scene->m_Registry,
				entt::null,
				v3(0.0f),
				v2{ width * 0.75f - 0.5f, 0.5f},
				v4{ 221.0f / 255.0f, 155.0f / 255.0f, 247.0f / 255.0f, 1.0f },
				v4{ 255.0f / 255.0f, 166.0f / 255.0f, 158.0f / 255.0f, 1.0f },
				false,
				false,
				0.0f,
				4.0f,
				0.0f,
				[this, width, w, height, h]() {
					if (!Input::IsMouseButtonPressed(MouseCode::Button0))
						return;
					entt::registry& mainRegistry = this->m_Scene->m_Registry;
					entt::entity scrollbarID = this->m_ScrollViewRoads->scrollbar->entityID;

					auto [mouseX, mouseY] = Input::get_mouse_pos_float();
					bool changed = this->m_ScrollViewRoads->scrollbar->Update(v2{
							(mouseX * width) / w,
							(mouseY * height) / h
						});
					if (changed)
						this->m_ScrollViewRoads->Update();
					mainRegistry.emplace_or_replace<OnDragCallbackComponent>(scrollbarID,this->m_ScrollViewRoads->scrollbar->OnDragCallback);
				},
				[this, width, w, height, h]() {
					entt::registry& mainRegistry = this->m_Scene->m_Registry;
					entt::entity scrollbarID = this->m_ScrollViewRoads->scrollbar->entityID;

					auto [mouseX, mouseY] = Input::get_mouse_pos_float();
					bool changed = this->m_ScrollViewRoads->scrollbar->Update(v2{
							(mouseX * width) / w,
							(mouseY * height) / h
						});
					if (changed)
						this->m_ScrollViewRoads->Update();

					if (!Input::IsMouseButtonPressed(MouseCode::Button0))
						mainRegistry.remove<OnDragCallbackComponent>(scrollbarID);
				}
		};
		m_ScrollViewRoads = new ScrollView(scrollViewRoadsParams, scrollBarRoadsParams);

		ScrollViewConstructorParameters scrollViewTreesParams = ScrollViewConstructorParameters{
				m_Scene->m_Registry,
				m_PanelTrees->entityID,
				v3{ width * 0.25f - 0.6f, height - 5.3f, 0.01f },
				v2{ width * 0.75f, 5.0f},
				v4{ 201.0f / 255.0f, 235.0f / 255.0f, 227.0f / 255.0f, 1.0f },
				2,
				[]() {std::cout << "You clicked to the ScrollView for trees!" << std::endl; }
		};
		ScrollBarConstructorParameters scrollBarTreesParams = ScrollBarConstructorParameters{
				m_Scene->m_Registry,
				entt::null,
				v3(0.0f),
				v2{ width * 0.75f - 0.5f, 0.5f},
				v4{ 221.0f / 255.0f, 155.0f / 255.0f, 247.0f / 255.0f, 1.0f },
				v4{ 255.0f / 255.0f, 166.0f / 255.0f, 158.0f / 255.0f, 1.0f },
				false,
				false,
				0.0f,
				4.0f,
				0.0f,
				[this, width, w, height, h]() {
					if (!Input::IsMouseButtonPressed(MouseCode::Button0))
						return;
					entt::registry& mainRegistry = this->m_Scene->m_Registry;
					entt::entity scrollbarID = this->m_ScrollViewTrees->scrollbar->entityID;

					auto [mouseX, mouseY] = Input::get_mouse_pos_float();
					bool changed = this->m_ScrollViewTrees->scrollbar->Update(v2{
							(mouseX * width) / w,
							(mouseY * height) / h
						});
					if (changed)
						this->m_ScrollViewTrees->Update();
					mainRegistry.emplace_or_replace<OnDragCallbackComponent>(scrollbarID,this->m_ScrollViewTrees->scrollbar->OnDragCallback);
				},
				[this, width, w, height, h]() {
					entt::registry& mainRegistry = this->m_Scene->m_Registry;
					entt::entity scrollbarID = this->m_ScrollViewTrees->scrollbar->entityID;

					auto [mouseX, mouseY] = Input::get_mouse_pos_float();
					bool changed = this->m_ScrollViewTrees->scrollbar->Update(v2{
							(mouseX * width) / w,
							(mouseY * height) / h
						});
					if (changed)
						this->m_ScrollViewTrees->Update();

					if (!Input::IsMouseButtonPressed(MouseCode::Button0))
						mainRegistry.remove<OnDragCallbackComponent>(scrollbarID);
				}
		};
		m_ScrollViewTrees = new ScrollView(scrollViewTreesParams, scrollBarTreesParams);

		v2 roadConstructionModeButtonsSize(0.8f);
		f32 padding = 0.2f;
		ButtonConstructorParameters straightRoadButtonParams = ButtonConstructorParameters{
					m_Scene->m_Registry,
					m_PanelRoads->entityID,
					v3{1.0f, height - 5.0f, 0.01f},
					roadConstructionModeButtonsSize,
					v4(1.0f),
					m_Parent->straightTexture,
					[this]() {
						entt::registry& mainRegistry = m_Scene->m_Registry;
						GameScene* gameScene = m_Parent->gameScene;

						mainRegistry.get<SpriteRendererComponent>(m_StraightRoadButton->entityID).border = true;
						mainRegistry.get<SpriteRendererComponent>(m_QuadraticRoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_Cubic1234RoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_Cubic1243RoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_Cubic1342RoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_Cubic1432RoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_DestructRoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_ChangeRoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_CancelRoadButton->entityID).border = false;

						gameScene->SetConstructionMode(ConstructionMode::Road);
						gameScene->m_RoadManager.SetConstructionMode(RoadConstructionMode::Straight);
					},
					0.1f,
					false,
					v4{ 255.0f / 255.0f, 166.0f / 255.0f, 158.0f / 255.0f, 1.0f },
		};
		m_StraightRoadButton = new Button(straightRoadButtonParams);
		ButtonConstructorParameters quadraticRoadButtonParams = ButtonConstructorParameters{
					m_Scene->m_Registry,
					m_PanelRoads->entityID,
					v3{1.0f + (roadConstructionModeButtonsSize.x + padding) * 1.0f, height - 5.0f, 0.01f},
					roadConstructionModeButtonsSize,
					v4(1.0f),
					m_Parent->quadraticTexture,
					[this]() {
						entt::registry& mainRegistry = m_Scene->m_Registry;
						GameScene* gameScene = m_Parent->gameScene;

						mainRegistry.get<SpriteRendererComponent>(m_StraightRoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_QuadraticRoadButton->entityID).border = true;
						mainRegistry.get<SpriteRendererComponent>(m_Cubic1234RoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_Cubic1243RoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_Cubic1342RoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_Cubic1432RoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_DestructRoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_ChangeRoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_CancelRoadButton->entityID).border = false;

						gameScene->SetConstructionMode(ConstructionMode::Road);
						gameScene->m_RoadManager.SetConstructionMode(RoadConstructionMode::QuadraticCurve);
					},
					0.1f,
					false,
					v4{ 255.0f / 255.0f, 166.0f / 255.0f, 158.0f / 255.0f, 1.0f },
		};
		m_QuadraticRoadButton = new Button(quadraticRoadButtonParams);
		ButtonConstructorParameters cubic1234RoadButtonParams = ButtonConstructorParameters{
					m_Scene->m_Registry,
					m_PanelRoads->entityID,
					v3{1.0f + (roadConstructionModeButtonsSize.x + padding) * 2.0f, (height - 5.0f), 0.01f},
					roadConstructionModeButtonsSize,
					v4(1.0f),
					m_Parent->cubic1234Texture,
					[this]() {
						entt::registry& mainRegistry = m_Scene->m_Registry;
						GameScene* gameScene = m_Parent->gameScene;

						mainRegistry.get<SpriteRendererComponent>(m_StraightRoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_QuadraticRoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_Cubic1234RoadButton->entityID).border = true;
						mainRegistry.get<SpriteRendererComponent>(m_Cubic1243RoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_Cubic1342RoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_Cubic1432RoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_DestructRoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_ChangeRoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_CancelRoadButton->entityID).border = false;

						gameScene->SetConstructionMode(ConstructionMode::Road);
						gameScene->m_RoadManager.SetConstructionMode(RoadConstructionMode::CubicCurve);
						gameScene->m_RoadManager.cubicCurveOrder = std::array<uint8_t, 4>{ 0, 1, 2, 3};
					},
					0.1f,
					false,
					v4{ 255.0f / 255.0f, 166.0f / 255.0f, 158.0f / 255.0f, 1.0f },
		};
		m_Cubic1234RoadButton = new Button(cubic1234RoadButtonParams);
		ButtonConstructorParameters cubic1243RoadButtonParams = ButtonConstructorParameters{
					m_Scene->m_Registry,
					m_PanelRoads->entityID,
					v3{1.0f + (roadConstructionModeButtonsSize.x + padding) * 2.0f, (height - 5.0f) + (roadConstructionModeButtonsSize.y + padding) * 1.0f, 0.01f},
					roadConstructionModeButtonsSize,
					v4(1.0f),
					m_Parent->cubic1243Texture,
					[this]() {
						entt::registry& mainRegistry = m_Scene->m_Registry;
						GameScene* gameScene = m_Parent->gameScene;

						mainRegistry.get<SpriteRendererComponent>(m_StraightRoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_QuadraticRoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_Cubic1234RoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_Cubic1243RoadButton->entityID).border = true;
						mainRegistry.get<SpriteRendererComponent>(m_Cubic1342RoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_Cubic1432RoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_DestructRoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_ChangeRoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_CancelRoadButton->entityID).border = false;

						gameScene->SetConstructionMode(ConstructionMode::Road);
						gameScene->m_RoadManager.SetConstructionMode(RoadConstructionMode::CubicCurve);
						gameScene->m_RoadManager.cubicCurveOrder = std::array<uint8_t, 4>{0, 1, 3, 2};
					},
					0.1f,
					false,
					v4{ 255.0f / 255.0f, 166.0f / 255.0f, 158.0f / 255.0f, 1.0f },
		};
		m_Cubic1243RoadButton = new Button(cubic1243RoadButtonParams);
		ButtonConstructorParameters cubic1342RoadButtonParams = ButtonConstructorParameters{
					m_Scene->m_Registry,
					m_PanelRoads->entityID,
					v3{1.0f + (roadConstructionModeButtonsSize.x + padding) * 2.0f, (height - 5.0f) + (roadConstructionModeButtonsSize.y + padding) * 2.0f, 0.01f},
					roadConstructionModeButtonsSize,
					v4(1.0f),
					m_Parent->cubic1342Texture,
					[this]() {
						entt::registry& mainRegistry = m_Scene->m_Registry;
						GameScene* gameScene = m_Parent->gameScene;

						mainRegistry.get<SpriteRendererComponent>(m_StraightRoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_QuadraticRoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_Cubic1234RoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_Cubic1243RoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_Cubic1342RoadButton->entityID).border = true;
						mainRegistry.get<SpriteRendererComponent>(m_Cubic1432RoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_DestructRoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_ChangeRoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_CancelRoadButton->entityID).border = false;

						gameScene->SetConstructionMode(ConstructionMode::Road);
						gameScene->m_RoadManager.SetConstructionMode(RoadConstructionMode::CubicCurve);
						gameScene->m_RoadManager.cubicCurveOrder = std::array<uint8_t, 4>{0, 3, 1, 2};
					},
					0.1f,
					false,
					v4{ 255.0f / 255.0f, 166.0f / 255.0f, 158.0f / 255.0f, 1.0f },
		};
		m_Cubic1342RoadButton = new Button(cubic1342RoadButtonParams);
		ButtonConstructorParameters cubic1432RoadButtonParams = ButtonConstructorParameters{
					m_Scene->m_Registry,
					m_PanelRoads->entityID,
					v3{1.0f + (roadConstructionModeButtonsSize.x + padding) * 2.0f, (height - 5.0f) + (roadConstructionModeButtonsSize.y + padding) * 3.0f, 0.01f},
					roadConstructionModeButtonsSize,
					v4(1.0f),
					m_Parent->cubic1432Texture,
					[this]() {
						entt::registry& mainRegistry = m_Scene->m_Registry;
						GameScene* gameScene = m_Parent->gameScene;

						mainRegistry.get<SpriteRendererComponent>(m_StraightRoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_QuadraticRoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_Cubic1234RoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_Cubic1243RoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_Cubic1342RoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_Cubic1432RoadButton->entityID).border = true;
						mainRegistry.get<SpriteRendererComponent>(m_DestructRoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_ChangeRoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_CancelRoadButton->entityID).border = false;

						gameScene->SetConstructionMode(ConstructionMode::Road);
						gameScene->m_RoadManager.SetConstructionMode(RoadConstructionMode::CubicCurve);
						gameScene->m_RoadManager.cubicCurveOrder = std::array<uint8_t, 4>{0, 3, 2, 1};
					},
					0.1f,
					false,
					v4{ 255.0f / 255.0f, 166.0f / 255.0f, 158.0f / 255.0f, 1.0f },
		};
		m_Cubic1432RoadButton = new Button(cubic1432RoadButtonParams);
		ButtonConstructorParameters destructRoadButtonParams = ButtonConstructorParameters{
					m_Scene->m_Registry,
					m_PanelRoads->entityID,
					v3{1.0f + (roadConstructionModeButtonsSize.x + padding) * 3.0f, height - 5.0f, 0.01f},
					roadConstructionModeButtonsSize,
					v4(1.0f),
					m_Parent->removeTexture,
					[this]() {
						entt::registry& mainRegistry = m_Scene->m_Registry;
						GameScene* gameScene = m_Parent->gameScene;

						mainRegistry.get<SpriteRendererComponent>(m_StraightRoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_QuadraticRoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_Cubic1234RoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_Cubic1243RoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_Cubic1342RoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_Cubic1432RoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_DestructRoadButton->entityID).border = true;
						mainRegistry.get<SpriteRendererComponent>(m_ChangeRoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_CancelRoadButton->entityID).border = false;

						gameScene->SetConstructionMode(ConstructionMode::Road);
						gameScene->m_RoadManager.SetConstructionMode(RoadConstructionMode::Destruct);
					},
					0.1f,
					false,
					v4{ 255.0f / 255.0f, 166.0f / 255.0f, 158.0f / 255.0f, 1.0f },
		};
		m_DestructRoadButton = new Button(destructRoadButtonParams);
		ButtonConstructorParameters changeRoadButtonParams = ButtonConstructorParameters{
					m_Scene->m_Registry,
					m_PanelRoads->entityID,
					v3{1.0f + (roadConstructionModeButtonsSize.x + padding) * 4.0f, height - 5.0f, 0.01f},
					roadConstructionModeButtonsSize,
					v4(1.0f),
					m_Parent->changeTexture,
					[this]() {
						entt::registry& mainRegistry = m_Scene->m_Registry;
						GameScene* gameScene = m_Parent->gameScene;

						mainRegistry.get<SpriteRendererComponent>(m_StraightRoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_QuadraticRoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_Cubic1234RoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_Cubic1243RoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_Cubic1342RoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_Cubic1432RoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_DestructRoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_ChangeRoadButton->entityID).border = true;
						mainRegistry.get<SpriteRendererComponent>(m_CancelRoadButton->entityID).border = false;

						gameScene->SetConstructionMode(ConstructionMode::Road);
						gameScene->m_RoadManager.SetConstructionMode(RoadConstructionMode::Change);
					},
					0.1f,
					false,
					v4{ 255.0f / 255.0f, 166.0f / 255.0f, 158.0f / 255.0f, 1.0f },
		};
		m_ChangeRoadButton = new Button(changeRoadButtonParams);
		ButtonConstructorParameters cancelRoadButtonParams = ButtonConstructorParameters{
					m_Scene->m_Registry,
					m_PanelRoads->entityID,
					v3{1.0f + (roadConstructionModeButtonsSize.x + padding) * 5.0f, height - 5.0f, 0.01f},
					roadConstructionModeButtonsSize,
					v4(1.0f),
					m_Parent->cancelTexture,
					[this]() {
						entt::registry& mainRegistry = m_Scene->m_Registry;
						GameScene* gameScene = m_Parent->gameScene;

						mainRegistry.get<SpriteRendererComponent>(m_StraightRoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_QuadraticRoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_Cubic1234RoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_Cubic1243RoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_Cubic1342RoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_Cubic1432RoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_DestructRoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_ChangeRoadButton->entityID).border = false;
						mainRegistry.get<SpriteRendererComponent>(m_CancelRoadButton->entityID).border = true;

						gameScene->SetConstructionMode(ConstructionMode::Road);
						gameScene->m_RoadManager.SetConstructionMode(RoadConstructionMode::None);
					},
					0.1f,
					true,
					v4{ 255.0f / 255.0f, 166.0f / 255.0f, 158.0f / 255.0f, 1.0f },
		};
		m_CancelRoadButton = new Button(cancelRoadButtonParams);
		m_Scene->m_Registry.emplace<ChildrenComponent>(m_PanelRoads->entityID, std::vector<entt::entity>{
			m_ScrollViewRoads->entityID,
			m_StraightRoadButton->entityID,
			m_QuadraticRoadButton->entityID,
			m_Cubic1234RoadButton->entityID,
			m_Cubic1243RoadButton->entityID,
			m_Cubic1342RoadButton->entityID,
			m_Cubic1432RoadButton->entityID,
			m_DestructRoadButton->entityID,
			m_ChangeRoadButton->entityID,
			m_CancelRoadButton->entityID
			});

		ButtonConstructorParameters addTreeButtonParams = ButtonConstructorParameters{
			m_Scene->m_Registry,
			m_PanelTrees->entityID,
			v3{2.0f, height - 5.0f, 0.01f},
			v2(0.8f),
			v4(1.0f),
			m_Parent->addTexture,
			[this]() {
				entt::registry& mainRegistry = m_Scene->m_Registry;
				GameScene* gameScene = m_Parent->gameScene;

				mainRegistry.get<SpriteRendererComponent>(m_AddTreeButton->entityID).border = true;
				mainRegistry.get<SpriteRendererComponent>(m_RemoveTreeButton->entityID).border = false;
				mainRegistry.get<SpriteRendererComponent>(m_CancelTreeButton->entityID).border = false;

				gameScene->SetConstructionMode(ConstructionMode::Tree);
				gameScene->m_TreeManager.SetConstructionMode(TreeConstructionMode::Adding);
			},
			0.1f,
			false,
			v4{ 221.0f / 255.0f, 255.0f / 255.0f, 247.0f / 255.0f, 1.0f },
		};
		m_AddTreeButton = new Button(addTreeButtonParams);
		ButtonConstructorParameters removeTreeButtonParams = ButtonConstructorParameters{
			m_Scene->m_Registry,
			m_PanelTrees->entityID,
			v3{3.0f, height - 5.0f, 0.01f},
			v2(0.8f),
			v4(1.0f),
			m_Parent->removeTexture,
			[this]() {
				entt::registry& mainRegistry = m_Scene->m_Registry;
				GameScene* gameScene = m_Parent->gameScene;

				mainRegistry.get<SpriteRendererComponent>(m_AddTreeButton->entityID).border = false;
				mainRegistry.get<SpriteRendererComponent>(m_RemoveTreeButton->entityID).border = true;
				mainRegistry.get<SpriteRendererComponent>(m_CancelTreeButton->entityID).border = false;

				gameScene->SetConstructionMode(ConstructionMode::Tree);
				gameScene->m_TreeManager.SetConstructionMode(TreeConstructionMode::Removing);
			},
			0.1f,
			false,
			v4{ 221.0f / 255.0f, 255.0f / 255.0f, 247.0f / 255.0f, 1.0f },
		};
		m_RemoveTreeButton = new Button(removeTreeButtonParams);
		ButtonConstructorParameters cancelTreeButtonParams = ButtonConstructorParameters{
			m_Scene->m_Registry,
			m_PanelTrees->entityID,
			v3{4.0f, height - 5.0f, 0.01f},
			v2(0.8f),
			v4(1.0f),
			m_Parent->cancelTexture,
			[this]() {
				entt::registry& mainRegistry = m_Scene->m_Registry;
				GameScene* gameScene = m_Parent->gameScene;

				mainRegistry.get<SpriteRendererComponent>(m_AddTreeButton->entityID).border = false;
				mainRegistry.get<SpriteRendererComponent>(m_RemoveTreeButton->entityID).border = false;
				mainRegistry.get<SpriteRendererComponent>(m_CancelTreeButton->entityID).border = true;

				gameScene->SetConstructionMode(ConstructionMode::Tree);
				gameScene->m_TreeManager.SetConstructionMode(TreeConstructionMode::None);
			},
			0.1f,
			true,
			v4{ 221.0f / 255.0f, 255.0f / 255.0f, 247.0f / 255.0f, 1.0f },
		};
		m_CancelTreeButton = new Button(cancelTreeButtonParams);
		m_Scene->m_Registry.emplace<ChildrenComponent>(m_PanelTrees->entityID, std::vector<entt::entity>{
			m_ScrollViewTrees->entityID,
			m_AddTreeButton->entityID,
			m_RemoveTreeButton->entityID,
			m_CancelTreeButton->entityID
			});


		/*Buttons in the tools panel*/ {
			v2 buttonSize{ 1.0f, 1.0f };
			v4 buttonColor{ 1.0f, 1.0f, 1.0f, 1.0f };
			ButtonConstructorParameters buttonTools_01Params = ButtonConstructorParameters{
				m_Scene->m_Registry,
				m_PanelTools->entityID,
				v3{ 0.75f, 2.0f, 0.0011f },
				buttonSize,
				buttonColor,
				nullptr,
				[]() {std::cout << "You clicked the 1st Button inside the tools panel!" << std::endl; },
				0.2f
			};
			m_ButtonTools_01 = new Button(buttonTools_01Params);
			ButtonConstructorParameters buttonTools_02Params = ButtonConstructorParameters{
				m_Scene->m_Registry,
				m_PanelTools->entityID,
				v3{ 2.0f, 2.0f, 0.0011f },
				buttonSize,
				buttonColor,
				nullptr,
				[]() {std::cout << "You clicked the 2nd Button inside the tools panel!" << std::endl; },
				0.2f
			};
			m_ButtonTools_02 = new Button(buttonTools_02Params);
			ButtonConstructorParameters buttonTools_03Params = ButtonConstructorParameters{
				m_Scene->m_Registry,
				m_PanelTools->entityID,
				v3{ 0.75f, 3.25f, 0.0011f },
				buttonSize,
				buttonColor,
				nullptr,
				[]() {std::cout << "You clicked the 3rd Button inside the tools panel!" << std::endl; },
				0.2f
			};
			m_ButtonTools_03 = new Button(buttonTools_03Params);
			ButtonConstructorParameters buttonTools_04Params = ButtonConstructorParameters{
				m_Scene->m_Registry,
				m_PanelTools->entityID,
				v3{ 2.0f, 3.25f, 0.0011f },
				buttonSize,
				buttonColor,
				nullptr,
				[]() {std::cout << "You clicked the 4th Button inside the tools panel!" << std::endl; },
				0.2f
			};
			m_ButtonTools_04 = new Button(buttonTools_04Params);
			ButtonConstructorParameters buttonTools_05Params = ButtonConstructorParameters{
				m_Scene->m_Registry,
				m_PanelTools->entityID,
				v3{ 0.75f, 4.5f, 0.0011f },
				buttonSize,
				buttonColor,
				nullptr,
				[]() {std::cout << "You clicked the 5th Button inside the tools panel!" << std::endl; },
				0.2f
			};
			m_ButtonTools_05 = new Button(buttonTools_05Params);
			ButtonConstructorParameters buttonTools_06Params = ButtonConstructorParameters{
				m_Scene->m_Registry,
				m_PanelTools->entityID,
				v3{ 2.0f, 4.5, 0.0011f },
				buttonSize,
				buttonColor,
				nullptr,
				[]() {std::cout << "You clicked the 6th Button inside the tools panel!" << std::endl; },
				0.2f
			};
			m_ButtonTools_06 = new Button(buttonTools_06Params);
			ButtonConstructorParameters buttonTools_07Params = ButtonConstructorParameters{
				m_Scene->m_Registry,
				m_PanelTools->entityID,
				v3{ 0.75f, 5.75, 0.0011f },
				buttonSize,
				buttonColor,
				nullptr,
				[]() {std::cout << "You clicked the 7th Button inside the tools panel!" << std::endl; },
				0.2f
			};
			m_ButtonTools_07 = new Button(buttonTools_07Params);
			ButtonConstructorParameters buttonTools_08Params = ButtonConstructorParameters{
				m_Scene->m_Registry,
				m_PanelTools->entityID,
				v3{ 2.0f, 5.75, 0.0011f },
				buttonSize,
				buttonColor,
				nullptr,
				[]() {std::cout << "You clicked the 8th Button inside the tools panel!" << std::endl; },
				0.2f
			};
			m_ButtonTools_08 = new Button(buttonTools_08Params);
			ButtonConstructorParameters buttonTools_09Params = ButtonConstructorParameters{
				m_Scene->m_Registry,
				m_PanelTools->entityID,
				v3{ 0.75f, 7.0f, 0.0011f },
				buttonSize,
				buttonColor,
				nullptr,
				[]() {std::cout << "You clicked the 9th Button inside the tools panel!" << std::endl; },
				0.2f
			};
			m_ButtonTools_09 = new Button(buttonTools_09Params);
			ButtonConstructorParameters buttonTools_10Params = ButtonConstructorParameters{
				m_Scene->m_Registry,
				m_PanelTools->entityID,
				v3{ 2.0f, 7.0f, 0.0011f },
				buttonSize,
				buttonColor,
				nullptr,
				[]() {std::cout << "You clicked the 10th Button inside the tools panel!" << std::endl; },
				0.2f
			};
			m_ButtonTools_10 = new Button(buttonTools_10Params);
			ButtonConstructorParameters buttonTools_11Params = ButtonConstructorParameters{
				m_Scene->m_Registry,
				m_PanelTools->entityID,
				v3{ 0.75f, 8.25, 0.0011f },
				buttonSize,
				buttonColor,
				nullptr,
				[]() {std::cout << "You clicked the 11th Button inside the tools panel!" << std::endl; },
				0.2f
			};
			m_ButtonTools_11 = new Button(buttonTools_11Params);
			ButtonConstructorParameters buttonTools_12Params = ButtonConstructorParameters{
				m_Scene->m_Registry,
				m_PanelTools->entityID,
				v3{ 2.0f, 8.25, 0.0011f },
				buttonSize,
				buttonColor,
				nullptr,
				[]() {std::cout << "You clicked the 12th Button inside the tools panel!" << std::endl; },
				0.2f
			};
			m_ButtonTools_12 = new Button(buttonTools_12Params);
			ButtonConstructorParameters buttonTools_13Params = ButtonConstructorParameters{
				m_Scene->m_Registry,
				m_PanelTools->entityID,
				v3{ 0.75f, 9.5, 0.0011f },
				buttonSize,
				buttonColor,
				nullptr,
				[]() {std::cout << "You clicked the 13th Button inside the tools panel!" << std::endl; },
				0.2f
			};
			m_ButtonTools_13 = new Button(buttonTools_13Params);
			ButtonConstructorParameters buttonTools_14Params = ButtonConstructorParameters{
				m_Scene->m_Registry,
				m_PanelTools->entityID,
				v3{ 2.0f, 9.5, 0.0011f },
				buttonSize,
				buttonColor,
				nullptr,
				[]() {std::cout << "You clicked the 14th Button inside the tools panel!" << std::endl; },
				0.2f
			};
			m_ButtonTools_14 = new Button(buttonTools_14Params);
			std::vector<entt::entity> toolsButtonList = {
			 m_ButtonTools_01->entityID,
			 m_ButtonTools_02->entityID,
			 m_ButtonTools_03->entityID,
			 m_ButtonTools_04->entityID,
			 m_ButtonTools_05->entityID,
			 m_ButtonTools_06->entityID,
			 m_ButtonTools_07->entityID,
			 m_ButtonTools_08->entityID,
			 m_ButtonTools_09->entityID,
			 m_ButtonTools_10->entityID,
			 m_ButtonTools_11->entityID,
			 m_ButtonTools_12->entityID,
			 m_ButtonTools_13->entityID,
			 m_ButtonTools_14->entityID
			};
			m_Scene->m_Registry.emplace<ChildrenComponent>(m_PanelTools->entityID, toolsButtonList);
		}
		/*Buttons in the needs panel*/ {
			v2 buttonSize{ 1.0f, 1.0f };
			v4 buttonColor{ 1.0f, 1.0f, 1.0f, 1.0f };
			ButtonConstructorParameters buttonNeeds_01Params = ButtonConstructorParameters{
				m_Scene->m_Registry,
				m_PanelNeeds->entityID,
				v3{ 0.75f, 2.0f, 0.0011f },
				buttonSize,
				buttonColor,
				nullptr,
				[]() {std::cout << "You clicked the 1st Button inside the needs panel!" << std::endl; },
				0.2f
			};
			m_ButtonNeeds_01 = new Button(buttonNeeds_01Params);
			ButtonConstructorParameters buttonNeeds_02Params = ButtonConstructorParameters{
				m_Scene->m_Registry,
				m_PanelNeeds->entityID,
				v3{ 2.0f, 2.0f, 0.0011f },
				buttonSize,
				buttonColor,
				nullptr,
				[]() {std::cout << "You clicked the 2nd Button inside the needs panel!" << std::endl; },
				0.2f
			};
			m_ButtonNeeds_02 = new Button(buttonNeeds_02Params);
			ButtonConstructorParameters buttonNeeds_03Params = ButtonConstructorParameters{
				m_Scene->m_Registry,
				m_PanelNeeds->entityID,
				v3{ 0.75f, 3.25f, 0.0011f },
				buttonSize,
				buttonColor,
				nullptr,
				[]() {std::cout << "You clicked the 3rd Button inside the needs panel!" << std::endl; },
				0.2f
			};
			m_ButtonNeeds_03 = new Button(buttonNeeds_03Params);
			ButtonConstructorParameters buttonNeeds_04Params = ButtonConstructorParameters{
				m_Scene->m_Registry,
				m_PanelNeeds->entityID,
				v3{ 2.0f, 3.25f, 0.0011f },
				buttonSize,
				buttonColor,
				nullptr,
				[]() {std::cout << "You clicked the 4th Button inside the needs panel!" << std::endl; },
				0.2f
			};
			m_ButtonNeeds_04 = new Button(buttonNeeds_04Params);
			ButtonConstructorParameters buttonNeeds_05Params = ButtonConstructorParameters{
				m_Scene->m_Registry,
				m_PanelNeeds->entityID,
				v3{ 0.75f, 4.5f, 0.0011f },
				buttonSize,
				buttonColor,
				nullptr,
				[]() {std::cout << "You clicked the 5th Button inside the needs panel!" << std::endl; },
				0.2f
			};
			m_ButtonNeeds_05 = new Button(buttonNeeds_05Params);
			ButtonConstructorParameters buttonNeeds_06Params = ButtonConstructorParameters{
				m_Scene->m_Registry,
				m_PanelNeeds->entityID,
				v3{ 2.0f, 4.5, 0.0011f },
				buttonSize,
				buttonColor,
				nullptr,
				[]() {std::cout << "You clicked the 6th Button inside the needs panel!" << std::endl; },
				0.2f
			};
			m_ButtonNeeds_06 = new Button(buttonNeeds_06Params);
			ButtonConstructorParameters buttonNeeds_07Params = ButtonConstructorParameters{
				m_Scene->m_Registry,
				m_PanelNeeds->entityID,
				v3{ 0.75f, 5.75, 0.0011f },
				buttonSize,
				buttonColor,
				nullptr,
				[]() {std::cout << "You clicked the 7th Button inside the needs panel!" << std::endl; },
				0.2f
			};
			m_ButtonNeeds_07 = new Button(buttonNeeds_07Params);
			ButtonConstructorParameters buttonNeeds_08Params = ButtonConstructorParameters{
				m_Scene->m_Registry,
				m_PanelNeeds->entityID,
				v3{ 2.0f, 5.75, 0.0011f },
				buttonSize,
				buttonColor,
				nullptr,
				[]() {std::cout << "You clicked the 8th Button inside the needs panel!" << std::endl; },
				0.2f
			};
			m_ButtonNeeds_08 = new Button(buttonNeeds_08Params);
			ButtonConstructorParameters buttonNeeds_09Params = ButtonConstructorParameters{
				m_Scene->m_Registry,
				m_PanelNeeds->entityID,
				v3{ 0.75f, 7.0f, 0.0011f },
				buttonSize,
				buttonColor,
				nullptr,
				[]() {std::cout << "You clicked the 9th Button inside the needs panel!" << std::endl; },
				0.2f
			};
			m_ButtonNeeds_09 = new Button(buttonNeeds_09Params);
			ButtonConstructorParameters buttonNeeds_10Params = ButtonConstructorParameters{
				m_Scene->m_Registry,
				m_PanelNeeds->entityID,
				v3{ 2.0f, 7.0f, 0.0011f },
				buttonSize,
				buttonColor,
				nullptr,
				[]() {std::cout << "You clicked the 10th Button inside the needs panel!" << std::endl; },
				0.2f
			};
			m_ButtonNeeds_10 = new Button(buttonNeeds_10Params);
			ButtonConstructorParameters buttonNeeds_11Params = ButtonConstructorParameters{
				m_Scene->m_Registry,
				m_PanelNeeds->entityID,
				v3{ 0.75f, 8.25, 0.0011f },
				buttonSize,
				buttonColor,
				nullptr,
				[]() {std::cout << "You clicked the 11th Button inside the needs panel!" << std::endl; },
				0.2f
			};
			m_ButtonNeeds_11 = new Button(buttonNeeds_11Params);
			ButtonConstructorParameters buttonNeeds_12Params = ButtonConstructorParameters{
				m_Scene->m_Registry,
				m_PanelNeeds->entityID,
				v3{ 2.0f, 8.25, 0.0011f },
				buttonSize,
				buttonColor,
				nullptr,
				[]() {std::cout << "You clicked the 12th Button inside the needs panel!" << std::endl; },
				0.2f
			};
			m_ButtonNeeds_12 = new Button(buttonNeeds_12Params);
			ButtonConstructorParameters buttonNeeds_13Params = ButtonConstructorParameters{
				m_Scene->m_Registry,
				m_PanelNeeds->entityID,
				v3{ 0.75f, 9.5, 0.0011f },
				buttonSize,
				buttonColor,
				nullptr,
				[]() {std::cout << "You clicked the 13th Button inside the needs panel!" << std::endl; },
				0.2f
			};
			m_ButtonNeeds_13 = new Button(buttonNeeds_13Params);
			ButtonConstructorParameters buttonNeeds_14Params = ButtonConstructorParameters{
				m_Scene->m_Registry,
				m_PanelNeeds->entityID,
				v3{ 2.0f, 9.5, 0.0011f },
				buttonSize,
				buttonColor,
				nullptr,
				[]() {std::cout << "You clicked the 14th Button inside the needs panel!" << std::endl; },
				0.2f
			};
			m_ButtonNeeds_14 = new Button(buttonNeeds_14Params);
			std::vector<entt::entity> needsButtonList = {
			 m_ButtonNeeds_01->entityID,
			 m_ButtonNeeds_02->entityID,
			 m_ButtonNeeds_03->entityID,
			 m_ButtonNeeds_04->entityID,
			 m_ButtonNeeds_05->entityID,
			 m_ButtonNeeds_06->entityID,
			 m_ButtonNeeds_07->entityID,
			 m_ButtonNeeds_08->entityID,
			 m_ButtonNeeds_09->entityID,
			 m_ButtonNeeds_10->entityID,
			 m_ButtonNeeds_11->entityID,
			 m_ButtonNeeds_12->entityID,
			 m_ButtonNeeds_13->entityID,
			 m_ButtonNeeds_14->entityID
			};
			m_Scene->m_Registry.emplace<ChildrenComponent>(m_PanelNeeds->entityID, needsButtonList);
		}

		// Move this
		std::vector<Prefab*> result;
		namespace fs = std::filesystem;
		std::string s = fs::current_path().string();

		std::string pathh = s + "\\assets\\objects\\Houses";
		std::vector<std::string> buildingtumbnailimagefiles = Helper::GetFiles(pathh, "Thumbnail_", ".png");

		std::string patht = s + "\\assets\\objects\\Trees";
		std::vector<std::string> treetumbnailimagefiles = Helper::GetFiles(patht, "Thumbnail_", ".png");

		/*Buttons in the Roads panel*/ {
			size_t roadCount = m_Parent->road_types.size();
			ChildrenComponent& children = m_Scene->m_Registry.get_or_emplace<ChildrenComponent>(m_ScrollViewRoads->entityID, std::vector<entt::entity>{});
			for (size_t i = 0; i < roadCount; i++)
			{
				ButtonConstructorParameters roadPanelButtonParams = ButtonConstructorParameters{
					m_Scene->m_Registry,
					m_ScrollViewRoads->entityID,
					v3(0.0f),
					v2(3.5f),
					v4{ 1.0f, 1.0f, 1.0f, 1.0f },
					m_Parent->road_types[i].thumbnail,
					[i, this]() {
						if (!Input::IsMouseButtonPressed(MouseCode::Button0))
							return;
						std::cout << "You clicked the " << (i + 1) << "th Button inside the Road panel!" << std::endl;
						this->m_Parent->gameScene->SetConstructionMode(ConstructionMode::Road);
						auto mode = this->m_Parent->gameScene->m_RoadManager.GetConstructionMode();
						if (mode == RoadConstructionMode::None || mode == RoadConstructionMode::Destruct)
							this->m_Parent->gameScene->m_RoadManager.SetConstructionMode(RoadConstructionMode::Straight);
						this->m_Parent->gameScene->m_RoadManager.SetType(i);
					},
					0.1f
				};
				Button* roadPanelButton = new Button(roadPanelButtonParams);
				children.Children.push_back(roadPanelButton->entityID);
				m_RoadPanelButtonList.push_back(roadPanelButton);
			}
			m_ScrollViewRoads->Update();
		}
		/*Buttons in the Tree panel*/ {
			size_t treeCount = m_Parent->trees.size();
			ChildrenComponent& children = m_Scene->m_Registry.emplace<ChildrenComponent>(m_ScrollViewTrees->entityID, std::vector<entt::entity>{});
			for (size_t i = 0; i < treeCount; i++)
			{
				ButtonConstructorParameters treePanelButtonParams = ButtonConstructorParameters{
					m_Scene->m_Registry,
					m_ScrollViewTrees->entityID,
					v3(0.0f),
					v2(3.5f),
					v4{ 1.0f, 1.0f, 1.0f, 1.0f },
					Texture2D::Create(treetumbnailimagefiles[i]),
					[i, this]() {
						if (!Input::IsMouseButtonPressed(MouseCode::Button0))
							return;
						std::cout << "You clicked the " << (i + 1) << "th Button inside the Tree panel!" << std::endl;
						this->m_Parent->gameScene->SetConstructionMode(ConstructionMode::Tree);
						auto mode = this->m_Parent->gameScene->m_TreeManager.GetConstructionMode();
						if (mode == TreeConstructionMode::None || mode == TreeConstructionMode::Removing)
							this->m_Parent->gameScene->m_TreeManager.SetConstructionMode(TreeConstructionMode::Adding);
						this->m_Parent->gameScene->m_TreeManager.SetType(i);
					},
					0.1f
				};
				Button* treePanelButton = new Button(treePanelButtonParams);
				children.Children.push_back(treePanelButton->entityID);
				m_TreePanelButtonList.push_back(treePanelButton);
			}
			m_ScrollViewTrees->Update();
		}
	}

	UIScene::~UIScene()
	{
		delete m_ButtonPause;
		delete m_ButtonNormal;
		delete m_Button2Times;
		delete m_Button4Times;

		delete m_ButtonRoads;
		delete m_ButtonBuildings;
		delete m_ButtonTrees;
		delete m_ButtonNeeds;
		delete m_ButtonTools;

		delete m_PanelRoads;
		delete m_PanelTrees;
		delete m_PanelNeeds;
		delete m_PanelTools;

		delete m_StraightRoadButton;
		delete m_QuadraticRoadButton;
		delete m_Cubic1234RoadButton;
		delete m_Cubic1243RoadButton;
		delete m_Cubic1342RoadButton;
		delete m_Cubic1432RoadButton;
		delete m_DestructRoadButton;
		delete m_ChangeRoadButton;
		delete m_CancelRoadButton;

		delete m_AddTreeButton;
		delete m_RemoveTreeButton;
		delete m_CancelTreeButton;

		delete m_ScrollViewRoads;
		delete m_ScrollViewTrees;

		delete m_ButtonTools_01;
		delete m_ButtonTools_02;
		delete m_ButtonTools_03;
		delete m_ButtonTools_04;
		delete m_ButtonTools_05;
		delete m_ButtonTools_06;
		delete m_ButtonTools_07;
		delete m_ButtonTools_08;
		delete m_ButtonTools_09;
		delete m_ButtonTools_10;
		delete m_ButtonTools_11;
		delete m_ButtonTools_12;
		delete m_ButtonTools_13;
		delete m_ButtonTools_14;

		delete m_ButtonNeeds_01;
		delete m_ButtonNeeds_02;
		delete m_ButtonNeeds_03;
		delete m_ButtonNeeds_04;
		delete m_ButtonNeeds_05;
		delete m_ButtonNeeds_06;
		delete m_ButtonNeeds_07;
		delete m_ButtonNeeds_08;
		delete m_ButtonNeeds_09;
		delete m_ButtonNeeds_10;
		delete m_ButtonNeeds_11;
		delete m_ButtonNeeds_12;
		delete m_ButtonNeeds_13;
		delete m_ButtonNeeds_14;


		u64 index = m_RoadPanelButtonList.size();
		while (index != 0)
		{
			index--;
			Button* b = m_RoadPanelButtonList[index];
			m_RoadPanelButtonList.pop_back();
			delete b;
		}
		index = m_TreePanelButtonList.size();
		while (index != 0)
		{
			index--;
			Button* b = m_TreePanelButtonList[index];
			m_TreePanelButtonList.pop_back();
			delete b;
		}

		delete m_Scene;
	}

	void UIScene::OnAttach()
	{
	}

	bool UIScene::OnUpdate(Can::TimeStep ts)
	{
		float widthHalf = m_AspectRatio * m_ZoomLevel;
		float heightHalf = m_ZoomLevel;

		//RenderCommand::SetClearColor({ 0.9f, 0.9f, 0.9f, 1.0f });
		//RenderCommand::Clear();

		Renderer2D::BeginScene(m_CameraController.m_Camera);
		v2 offset = { -widthHalf, heightHalf };
		ChildrenComponent& children = m_Scene->m_Registry.get_or_emplace<ChildrenComponent>(m_Scene->entityID, std::vector<entt::entity>{});
		for (auto entity : children)
			Draw(entity, &(m_Scene->m_Registry), offset);

		auto view = m_Scene->m_Registry.view<OnDragCallbackComponent>();

		for (entt::entity e : view)
			m_Scene->m_Registry.get<OnDragCallbackComponent>(e)();
		char fpsText[15];
		sprintf(fpsText, "FPS:%3.2f", (1.0f / ts));
		Renderer2D::DrawText(
			fpsText,
			v3{ -1330.0f , -420.0f, 2.0f },
			v4(v3(0.0f), 1.0f),
			m_ZoomLevel
		);
		Renderer2D::EndScene();
		return false;
	}
	void UIScene::OnEvent(Can::Event::Event& event)
	{
		Can::Event::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Event::MouseButtonPressedEvent>(CAN_BIND_EVENT_FN(UIScene::OnMousePressed));

	}
	bool UIScene::OnMousePressed(Can::Event::MouseButtonPressedEvent& event)
	{
		float cameraWidth = m_AspectRatio * m_ZoomLevel * 2.0f;
		float cameraHeight = m_ZoomLevel * 2.0f;

		float mouseX = Can::Input::GetMouseX();
		float mouseY = Can::Input::GetMouseY();
		Application& app = Application::Get();
		unsigned int w = app.GetWindow().GetWidth();
		unsigned int h = app.GetWindow().GetHeight();

		v2 clickPos = { cameraWidth * (mouseX / w), cameraHeight * (mouseY / h) };

		ChildrenComponent& children = m_Scene->m_Registry.get<ChildrenComponent>(m_Scene->entityID);

		for (auto it = children.end(); it != children.begin();)
		{
			event.m_Handled = CheckCollision((*--it), &(m_Scene->m_Registry), clickPos);
			if (event.m_Handled)
				return true;
		}
		return false;
	}

	bool CheckCollision(entt::entity id, entt::registry* registry, const v2& clickPosition)
	{
		if (registry->has<HiddenComponent>(id))
			return false;
		if (registry->has<TransformComponent, SpriteRendererComponent>(id) == false)
			return false;

		if (registry->has<ChildrenComponent>(id))
		{
			ChildrenComponent& children = registry->get<ChildrenComponent>(id);
			for (auto it = children.end(); it != children.begin();)
			{
				bool didColide = CheckCollision((*--it), registry, clickPosition);
				if (didColide)
					return true;
			}
		}

		if (registry->has<IgnoreCollisionComponent>(id))
			return false;

		auto [transform, spriteRenderer] = registry->get< TransformComponent, SpriteRendererComponent>(id);

		v2 size = spriteRenderer.size * transform.Scale;
		v2 leftTop = {
			transform.Position.x,
			transform.Position.y
		};
		v2 rightBottom = leftTop + size * v2{ spriteRenderer.trim[1], spriteRenderer.trim[2] };
		leftTop += size * v2{ spriteRenderer.trim[3], spriteRenderer.trim[0] };
		if (
			clickPosition.x < leftTop.x ||
			clickPosition.y < leftTop.y ||
			clickPosition.x > rightBottom.x ||
			clickPosition.y > rightBottom.y
			)
			return false;

		if (registry->has<OnClickCallbackComponent>(id))
			registry->get<OnClickCallbackComponent>(id)();

		return true;
	}

	void Draw(entt::entity id, entt::registry* registry, const v2& offset)
	{
		if (registry->has<HiddenComponent>(id))
			return;
		if (registry->has<TransformComponent, SpriteRendererComponent>(id) == false)
			return;


		auto [transform, spriteRenderer] = registry->get< TransformComponent, SpriteRendererComponent>(id);
		v3 pos = transform.Position;

		pos.x = pos.x + offset.x + spriteRenderer.size.x / 2.0f;
		pos.y = -1 * (pos.y - offset.y + spriteRenderer.size.y / 2.0f);

		glm::mat4 newTransform = glm::translate(glm::mat4(1.0f), pos) * glm::scale(glm::mat4(1), v3(spriteRenderer.size, 1.0f));
		if (spriteRenderer.borderRadius >= 0.0f)
		{
			if (spriteRenderer.border) Renderer2D::DrawRoundedQuad(DrawQuadParameters{ pos - v3{ 0.0f, 0.0f, 0.0001f }, v3(spriteRenderer.size, 1.0f) + v3{ 0.075f, 0.075f, 0.0f }, 0.0f, spriteRenderer.bordercolor, nullptr, spriteRenderer.trim, spriteRenderer.borderRadius, 3 });
			Renderer2D::DrawRoundedQuad(newTransform, DrawQuadParameters{ transform.Position, spriteRenderer.size, 0.0f, spriteRenderer.color, spriteRenderer.texture, spriteRenderer.trim, spriteRenderer.borderRadius, 3 });
		}
		else
		{
			if (spriteRenderer.border) Renderer2D::DrawQuad(DrawQuadParameters{ pos - v3{ 0.0f, 0.0f, 0.0001f }, v3(spriteRenderer.size, 1.0f) + v3{ 0.075f, 0.075f, 0.0f }, 0.0f, spriteRenderer.bordercolor, nullptr, spriteRenderer.trim });
			Renderer2D::DrawQuad(newTransform, DrawQuadParameters{ transform.Position, spriteRenderer.size, 0.0f, spriteRenderer.color, spriteRenderer.texture, spriteRenderer.trim });
		}

		if (registry->has<ChildrenComponent>(id))
		{
			ChildrenComponent& children = registry->get<ChildrenComponent>(id);

			for (auto entity : children)
				Draw(entity, registry, offset);
		}
	}
}