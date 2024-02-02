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
	mat4 ProjectionMatrix;
	mat4 ViewMatrix;
	vec4 AmbientColor;		// w is intensity
} sceneUBO;

// set 0 and binding 1 is on fragment shader only

layout(set = 0, binding = 2) uniform ObjectUBO
{
	mat4 ModelMatrix;
} objectUBO;

void main()
{
	vec4 positionWorld = objectUBO.ModelMatrix * vec4(inPosition, 1.0);
	gl_Position = sceneUBO.ProjectionMatrix * sceneUBO.ViewMatrix * positionWorld;
	
	outColor = inColor;
	outPositionWorld = positionWorld.xyz;
	outNormalWorld = normalize(mat3(transpose(inverse(objectUBO.ModelMatrix))) * inNormal);
}