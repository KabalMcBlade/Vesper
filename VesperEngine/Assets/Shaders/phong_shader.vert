#version 450

#if BINDLESS == 1
    #extension GL_EXT_nonuniform_qualifier : require
#endif

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;
layout(location = 4) in vec3 inMorphPos0;
layout(location = 5) in vec3 inMorphNorm0;
layout(location = 6) in vec3 inMorphPos1;
layout(location = 7) in vec3 inMorphNorm1;
layout(location = 8) in vec3 inMorphPos2;
layout(location = 9) in vec3 inMorphNorm2;
layout(location = 10) in vec3 inMorphPos3;
layout(location = 11) in vec3 inMorphNorm3;
layout(location = 12) in vec3 inMorphPos4;
layout(location = 13) in vec3 inMorphNorm4;
layout(location = 14) in vec3 inMorphPos5;
layout(location = 15) in vec3 inMorphNorm5;
layout(location = 16) in vec3 inMorphPos6;
layout(location = 17) in vec3 inMorphNorm6;
layout(location = 18) in vec3 inMorphPos7;
layout(location = 19) in vec3 inMorphNorm7;

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
    vec4 MorphWeights;
    int MorphTargetCount;
} entityUBO;
#else
layout(std140, set = 1, binding = 0) uniform EntityUBO 
{
    mat4 ModelMatrix;
    vec4 MorphWeights;
    int MorphTargetCount;
} entityUBO;
#endif


void main()
{
    vec3 morphPos[8] = vec3[](inMorphPos0, inMorphPos1, inMorphPos2, inMorphPos3, inMorphPos4, inMorphPos5, inMorphPos6, inMorphPos7);
    vec3 morphNorm[8] = vec3[](inMorphNorm0, inMorphNorm1, inMorphNorm2, inMorphNorm3, inMorphNorm4, inMorphNorm5, inMorphNorm6, inMorphNorm7);

    vec3 finalPos = inPosition;
    vec3 finalNorm = inNormal;
    int morphCount = clamp(entityUBO.MorphTargetCount, 0, 8);
    for (int i = 0; i < morphCount; ++i)
    {
        finalPos += morphPos[i] * entityUBO.MorphWeights[i];
        finalNorm += morphNorm[i] * entityUBO.MorphWeights[i];
    }

    vec4 positionWorld = entityUBO.ModelMatrix * vec4(finalPos, 1.0);
    gl_Position = sceneUBO.ProjectionMatrix * sceneUBO.ViewMatrix * positionWorld;

    fragColor = inColor;
    fragPositionWorld = positionWorld.xyz;
    fragNormalWorld = normalize(mat3(transpose(inverse(entityUBO.ModelMatrix))) * finalNorm);
    fragUV = inUV;
}
