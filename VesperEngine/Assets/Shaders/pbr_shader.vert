#version 450

#if BINDLESS == 1
    #extension GL_EXT_nonuniform_qualifier : require
#endif

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV1;
layout(location = 4) in vec2 inUV2;
layout(location = 5) in vec4 inTangent;
layout(location = 6) in vec3 inMorphPos0;
layout(location = 7) in vec3 inMorphNorm0;
layout(location = 8) in vec3 inMorphPos1;
layout(location = 9) in vec3 inMorphNorm1;
layout(location = 10) in vec3 inMorphPos2;
layout(location = 11) in vec3 inMorphNorm2;
layout(location = 12) in vec3 inMorphPos3;
layout(location = 13) in vec3 inMorphNorm3;
layout(location = 14) in vec3 inMorphPos4;
layout(location = 15) in vec3 inMorphNorm4;
layout(location = 16) in vec3 inMorphPos5;
layout(location = 17) in vec3 inMorphNorm5;
layout(location = 18) in vec3 inMorphPos6;
layout(location = 19) in vec3 inMorphNorm6;
layout(location = 20) in vec3 inMorphPos7;
layout(location = 21) in vec3 inMorphNorm7;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragPositionWorld;
layout(location = 2) out vec3 fragNormalWorld;
layout(location = 3) out vec2 fragUV1;
layout(location = 4) out vec2 fragUV2;
layout(location = 5) out vec4 fragTangentWorld;

layout(std140, set = 0, binding = 0) uniform SceneUBO
{
    mat4 ProjectionMatrix;
    mat4 ViewMatrix;
    vec4 CameraPosition;
    vec4 AmbientColor;
} sceneUBO;

#if BINDLESS == 1
layout(std140, set = 2, binding = 0) uniform EntityUBO
{
    mat4 ModelMatrix;
    vec4 MorphWeights0;
    vec4 MorphWeights1;
    int MorphTargetCount;
} entityUBO;
#else
layout(std140, set = 1, binding = 0) uniform EntityUBO
{
    mat4 ModelMatrix;
    vec4 MorphWeights0;
    vec4 MorphWeights1;
    int MorphTargetCount;
} entityUBO;
#endif

void main()
{
    vec3 morphPos[8] = vec3[](inMorphPos0, inMorphPos1, inMorphPos2, inMorphPos3,
                              inMorphPos4, inMorphPos5, inMorphPos6, inMorphPos7);
    vec3 morphNorm[8] = vec3[](inMorphNorm0, inMorphNorm1, inMorphNorm2, inMorphNorm3,
                               inMorphNorm4, inMorphNorm5, inMorphNorm6, inMorphNorm7);

    vec3 finalPos = inPosition;
    vec3 finalNorm = inNormal;

    vec4 weights0 = entityUBO.MorphWeights0;
    vec4 weights1 = entityUBO.MorphWeights1;
    int morphCount = clamp(entityUBO.MorphTargetCount, 0, 8);

    for (int i = 0; i < morphCount; ++i)
    {
        float w = (i < 4) ? weights0[i] : weights1[i - 4];
        finalPos += morphPos[i] * w;
        finalNorm += morphNorm[i] * w;
    }

    vec4 positionWorld = entityUBO.ModelMatrix * vec4(finalPos, 1.0);
    gl_Position = sceneUBO.ProjectionMatrix * sceneUBO.ViewMatrix * positionWorld;

    fragColor = inColor;
    fragPositionWorld = positionWorld.xyz;
    fragNormalWorld = normalize(mat3(transpose(inverse(entityUBO.ModelMatrix))) * finalNorm);
    fragUV1 = inUV1;
    fragUV2 = inUV2;
    fragTangentWorld = vec4(normalize(mat3(transpose(inverse(entityUBO.ModelMatrix))) * inTangent.xyz), inTangent.w);
}
