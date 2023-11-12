#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;

layout(location = 0) out vec3 outColor;
layout(location = 1) out vec3 outPositionWorld;
layout(location = 2) out vec3 outNormalWorld;

layout(set = 0, binding = 0) uniform SceneUBO
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	vec4 ambientLightColor;		// w is intensity
	vec4 pointLightPosition;
	vec4 pointLightColor;		// w is intensity
} sceneUBO;

layout(push_constant) uniform Push
{
	mat4 modelMatrix;
} push;


void main()
{
	vec4 positionWorld = push.modelMatrix * vec4(inPosition, 1.0);
	gl_Position = sceneUBO.projectionMatrix * sceneUBO.viewMatrix * positionWorld;
	
	outColor = inColor;
	outPositionWorld = positionWorld.xyz;
	outNormalWorld = normalize(mat3(transpose(inverse(push.modelMatrix))) * inNormal);
}