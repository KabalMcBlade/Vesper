#include "pch.h"
#include "Scene/game_object.h"

VESPERENGINE_NAMESPACE_BEGIN

glm::mat4 TransformComponent2D::mat4()
{
	const float s = glm::sin(rotation);
	const float c = glm::cos(rotation);

// 	// Rotation on X
// 	glm::mat4 rotMatrix
// 	{
// 		{1.0f,	0.0f,	0.0f,	0.0f},
// 		{0.0f,	c,		-s,	0.0f},
// 		{0.0f,	s,		c,	0.0f},
// 		{0.0f,	0.0f,	0.0f,	1.0f}
// 	};

// 	// Rotation on Y
// 	glm::mat4 rotMatrix
// 	{
// 		{c,		0.0f,	s,		0.0f},
// 		{0.0f,	1.0f,	0.0f,	0.0f},
// 		{-s,	0.0f,	c,		0.0f},
// 		{0.0f,	0.0f,	0.0f,	1.0f}
// 	};

	// Rotation on Z
	glm::mat4 rotMatrix
	{
		{c,		-s,		0.0f,	0.0f},
		{s,		c,		0.0f,	0.0f},
		{0.0f,	0.0f,	1.0f,	0.0f},
		{0.0f,	0.0f,	0.0f,	1.0f}
	};

	glm::mat4 scaleMat
	{
		{scale.x,	0.0f,		0.0f,		0.0f},
		{0.0f,		scale.y,	0.0f,		0.0f},
		{0.0f,		0.0f,		scale.z,	0.0f},
		{0.0f,		0.0f,		0.0f,		1.0f}
	};

	return rotMatrix * scaleMat;
}

VESPERENGINE_NAMESPACE_END