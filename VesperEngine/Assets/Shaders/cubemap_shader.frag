#version 450

layout(location = 0) in vec3 fragPosition;
layout(location = 0) out vec4 outColor;

layout(std140, set = 0, binding = 0) uniform SceneUBO
{
    mat4 ProjectionMatrix;
    mat4 ViewMatrix;
    vec4 AmbientColor;
} sceneUBO;

#if BINDLESS == 1
layout(set = 2, binding = 0) uniform samplerCube cubeMap;
#else
layout(set = 1, binding = 0) uniform samplerCube cubeMap;
#endif

void main()
{
    outColor = texture(cubeMap, normalize(fragPosition));
}
