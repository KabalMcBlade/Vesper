#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragPositionWorld;
layout(location = 2) out vec3 fragNormalWorld;
layout(location = 3) out vec2 fragUV;

layout(std140, set = 0, binding = 0) uniform SceneUBO 
{
    mat4 ProjectionMatrix;
    mat4 ViewMatrix;
    vec4 AmbientColor; // w is intensity
} sceneUBO;

// set 0 and binding 1 is on fragment shader only

layout(std140, set = 1, binding = 0) uniform ObjectUBO 
{
    mat4 ModelMatrix;
} objectUBO;

void main()
{
    vec4 positionWorld = objectUBO.ModelMatrix * vec4(inPosition, 1.0);
    gl_Position = sceneUBO.ProjectionMatrix * sceneUBO.ViewMatrix * positionWorld;

    fragColor = inColor;
    fragPositionWorld = positionWorld.xyz;
    fragNormalWorld = normalize(mat3(transpose(inverse(objectUBO.ModelMatrix))) * inNormal);
    fragUV = inUV;
}
