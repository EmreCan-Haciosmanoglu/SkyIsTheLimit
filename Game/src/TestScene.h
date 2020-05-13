#pragma once
#include "Can.h"

class GameApp;
class TestScene : public Can::Layer::Layer
{
public:
	TestScene(GameApp* parent);
	virtual ~TestScene() = default;

	virtual void OnAttach() override {}
	virtual void OnDetach() override {}

	virtual void OnUpdate(Can::TimeStep ts) override;
	virtual void OnEvent(Can::Event::Event& event) override;

private:
	GameApp* m_Parent;
	Can::Camera::Controller::Perspective m_MainCameraController;

};