#version 450

layout(location = 0) in vec3 inColor;
layout(location = 1) in vec3 inPositionWorld;
layout(location = 2) in vec3 inNormalWorld;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform LightUBO
{
	vec4 AmbientColor;		// w is intensity
	vec4 LightPos;
	vec4 LightColor;		// w is intensity
} lightUBO;

void main()
{
	vec3 directionToLight = lightUBO.LightPos.xyz - inPositionWorld;
	float attenuation = 1.0 / dot(directionToLight, directionToLight); // distance squared
	
	vec3 lightColor = lightUBO.LightColor.xyz * lightUBO.LightColor.w * attenuation;
	vec3 ambientLight = lightUBO.AmbientColor.xyz * lightUBO.AmbientColor.w;
	vec3 diffuseLight = lightColor * max(dot(inNormalWorld, normalize(directionToLight)), 0);

	outColor = vec4((diffuseLight + ambientLight) * inColor, 1.0);
}