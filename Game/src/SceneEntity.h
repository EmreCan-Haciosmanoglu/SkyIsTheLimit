#pragma once
#include "Can.h"
#include "Can/ECS/Entity.h"

namespace Can
{
	class SceneEntity : public Entity
	{
	public:
		SceneEntity()
			: Entity()
		{
		}
		virtual ~SceneEntity() = default;

		virtual void Draw(const glm::vec3&) override;
		virtual void OnEvent(Event::Event& e) override {}
		virtual bool OnClickEvent(const glm::vec2&) override { return false; }
	};
}