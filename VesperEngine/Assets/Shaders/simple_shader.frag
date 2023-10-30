#version 450


layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPosWorld;
layout(location = 2) in vec3 fragNormalWorld;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform GlobalUBO
{
	mat4 projectionViewMatrix;
	vec4 ambientLightColor;		// w is intensity
	vec4 pointLightPosition;
	vec4 pointLightColor;		// w is intensity
} ubo;

layout(push_constant) uniform Push
{
	mat4 ModelMatrix;
	mat4 normalModelMatrix;
} push;

void main()
{
	vec3 directionToLight = ubo.pointLightPosition.xyz - fragPosWorld;
	float attenuation = 1.0 / dot(directionToLight, directionToLight); // distance squared
	
	vec3 lightColor = ubo.pointLightColor.xyz * ubo.pointLightColor.w * attenuation;
	vec3 ambientLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
	vec3 diffuseLight = lightColor * max(dot(fragNormalWorld, normalize(directionToLight)), 0);

	outColor = vec4((diffuseLight + ambientLight) * fragColor, 1.0);
}