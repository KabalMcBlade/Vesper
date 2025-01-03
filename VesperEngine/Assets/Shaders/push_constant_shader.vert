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
    vec4 positionWorld = push.ModelMatrix * vec4(inPosition, 1.0);
    gl_Position = sceneUBO.ProjectionMatrix * sceneUBO.ViewMatrix * positionWorld;
    
    outColor = inColor;
    outPositionWorld = positionWorld.xyz;
    outNormalWorld = normalize(mat3(transpose(inverse(push.ModelMatrix))) * inNormal);
}
