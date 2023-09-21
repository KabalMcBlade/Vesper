#version 450

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 color;

//layout(location = 0) out vec4 fragColor;

layout(push_constant) uniform Push
{
	mat4 transform;
	vec4 offsetPosition;
	vec4 color;
} push;

void main()
{
	vec4 pos = push.transform * position + push.offsetPosition;
	pos.w = 1.0;
	gl_Position = pos;

	//gl_Position = position;
	//fragColor = color;
}