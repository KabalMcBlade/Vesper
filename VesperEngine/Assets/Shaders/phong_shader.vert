#version 450

#if BINDLESS == 1
    #extension GL_EXT_nonuniform_qualifier : require
#endif

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
    vec4 CameraPosition;
    vec4 AmbientColor; // w is intensity
} sceneUBO;


#if BINDLESS == 1
layout(std140, set = 2, binding = 0) uniform EntityUBO 
{
    mat4 ModelMatrix;
} entityUBO;
#else
layout(std140, set = 1, binding = 0) uniform EntityUBO 
{
    mat4 ModelMatrix;
} entityUBO;
#endif


void main()
{
    vec4 positionWorld = entityUBO.ModelMatrix * vec4(inPosition, 1.0);
    gl_Position = sceneUBO.ProjectionMatrix * sceneUBO.ViewMatrix * positionWorld;

    fragColor = inColor;
    fragPositionWorld = positionWorld.xyz;
    fragNormalWorld = normalize(mat3(transpose(inverse(entityUBO.ModelMatrix))) * inNormal);
    fragUV = inUV;
}
