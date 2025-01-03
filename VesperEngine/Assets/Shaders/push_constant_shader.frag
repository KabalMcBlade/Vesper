#version 450

layout(location = 0) in vec3 inColor;
layout(location = 1) in vec3 inPositionWorld;
layout(location = 2) in vec3 inNormalWorld;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform SceneUBO
{
    mat4 ProjectionMatrix;
    mat4 ViewMatrix;
    vec4 AmbientLightColor;        // w is intensity
    vec4 PointLightPosition;
    vec4 PointLightColor;        // w is intensity
} sceneUBO;

layout(push_constant) uniform Push
{
    mat4 ModelMatrix;
} push;

void main()
{
    vec3 directionToLight = sceneUBO.PointLightPosition.xyz - inPositionWorld;
    float attenuation = 1.0 / dot(directionToLight, directionToLight); // distance squared
    
    vec3 lightColor = sceneUBO.PointLightColor.xyz * sceneUBO.PointLightColor.w * attenuation;
    vec3 ambientLight = sceneUBO.AmbientLightColor.xyz * sceneUBO.AmbientLightColor.w;
    vec3 diffuseLight = lightColor * max(dot(inNormalWorld, normalize(directionToLight)), 0);

    outColor = vec4((diffuseLight + ambientLight) * inColor, 1.0);
}
