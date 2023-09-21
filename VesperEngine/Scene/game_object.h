#pragma once

#include "Core/core_defines.h"

#define GLM_FORCE_INTRINSICS
//#define GLM_FORCE_SSE2		// or GLM_FORCE_SSE42 or else, but the above one use compiler to find out which one is enabled
#define GLM_FORCE_ALIGNED
#define GLM_FORCE_SWIZZLE
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "Geometry/model.h"


VESPERENGINE_NAMESPACE_BEGIN

struct VESPERENGINE_DLL TransformComponent2D
{
	glm::vec4 translation{};  // (position offset)
	glm::vec4 scale{ 1.f, 1.f, 1.f, 1.f };
	float rotation;

	glm::mat4 mat4();
};

class VESPERENGINE_DLL GameObject
{
public:
	using id_t = unsigned int;

	static GameObject CreateGameObject()
	{
		static id_t currentId = 0;
		return GameObject{ ++currentId };
	}

	GameObject(const GameObject&) = delete;
	GameObject& operator=(const GameObject&) = delete;
	GameObject(GameObject&&) = default;
	GameObject& operator=(GameObject&&) = default;

	id_t GetId() { return m_id; }

	std::shared_ptr<Model> m_model{};
	glm::vec4 m_color{};
	TransformComponent2D m_transform2D{};

private:
	GameObject(id_t _id) : m_id{ _id } {}

	id_t m_id;
};

VESPERENGINE_NAMESPACE_END