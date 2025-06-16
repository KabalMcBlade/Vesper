#version 450

#if BINDLESS == 1
#extension GL_EXT_nonuniform_qualifier : require
#endif

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPositionWorld;
layout(location = 2) in vec3 fragNormalWorld;
layout(location = 3) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

layout(std140, set = 0, binding = 0) uniform SceneUBO
{
    mat4 ProjectionMatrix;
    mat4 ViewMatrix;
    vec4 AmbientColor;
} sceneUBO;

layout(std140, set = 0, binding = 1) uniform LightUBO
{
    vec4 LightPos;
    vec4 LightColor;
} lightUBO;
layout(set = 0, binding = 2) uniform samplerCube irradianceMap;
layout(set = 0, binding = 3) uniform samplerCube prefilteredEnvMap;
layout(set = 0, binding = 4) uniform sampler2D brdfLUT;

#if BINDLESS == 1
layout(set = 1, binding = 0) uniform sampler2D textures[];
layout(std140, set = 1, binding = 1) uniform MaterialData
{
    float Roughness;
    float Metallic;
    float Sheen;
    float ClearcoatThickness;
    float ClearcoatRoughness;
    float Anisotropy;
    float AnisotropyRotation;
    int TextureIndices[7];
} materials[];
layout(std140, set = 3, binding = 0) uniform MaterialIndexUBO
{
    int materialIndex;
} materialIndexUBO;
#else
layout(set = 2, binding = 0) uniform sampler2D roughnessTexture;
layout(set = 2, binding = 1) uniform sampler2D metallicTexture;
layout(set = 2, binding = 2) uniform sampler2D sheenTexture;
layout(set = 2, binding = 3) uniform sampler2D emissiveTexture;
layout(set = 2, binding = 4) uniform sampler2D normalTexture;
layout(set = 2, binding = 5) uniform sampler2D baseColorTexture;
layout(set = 2, binding = 6) uniform sampler2D aoTexture;
layout(std140, set = 2, binding = 7) uniform MaterialData
{
    float Roughness;
    float Metallic;
    float Sheen;
    float ClearcoatThickness;
    float ClearcoatRoughness;
    float Anisotropy;
    float AnisotropyRotation;
    int TextureIndices[7];
} material;
#endif

const float PI = 3.14159265359;

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float distributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / (PI * denom * denom + 0.0001);
}

float geometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    float denom = NdotV * (1.0 - k) + k;
    return NdotV / denom;
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = geometrySchlickGGX(NdotV, roughness);
    float ggx1 = geometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}
vec3 getIBLContribution(vec3 N, vec3 V, float roughness, vec3 F0, vec3 kS, vec3 kD, vec3 baseColor)
{
    vec3 R = reflect(-V, N);
    float mipCount = float(textureQueryLevels(prefilteredEnvMap));
    float lod = roughness * (mipCount - 1.0);
    vec3 prefilteredColor = textureLod(prefilteredEnvMap, R, lod).rgb;
    vec2 brdf = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 F = fresnelSchlick(max(dot(N, V), 0.0), F0);
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse = irradiance * kD * baseColor;
    return diffuse + specular;
}

void main()
{
#if BINDLESS == 1
    int matIdx = materialIndexUBO.materialIndex;
    float roughness = materials[matIdx].Roughness;
    float metallic = materials[matIdx].Metallic;
    bool hasNormal = materials[matIdx].TextureIndices[4] != -1;
    bool hasRoughness = materials[matIdx].TextureIndices[0] != -1;
    bool hasMetallic = materials[matIdx].TextureIndices[1] != -1;
    bool hasEmissive = materials[matIdx].TextureIndices[3] != -1;
    bool hasBaseColor = materials[matIdx].TextureIndices[5] != -1;
    bool hasAO = materials[matIdx].TextureIndices[6] != -1;
#else
    float roughness = material.Roughness;
    float metallic = material.Metallic;
    bool hasNormal = material.TextureIndices[4] != -1;
    bool hasRoughness = material.TextureIndices[0] != -1;
    bool hasMetallic = material.TextureIndices[1] != -1;
    bool hasEmissive = material.TextureIndices[3] != -1;
    bool hasBaseColor = material.TextureIndices[5] != -1;
    bool hasAO = material.TextureIndices[6] != -1;
#endif

    vec3 N = normalize(fragNormalWorld);
#if BINDLESS == 1
    if(hasNormal)
        N = normalize(texture(textures[nonuniformEXT(materials[matIdx].TextureIndices[4])], fragUV).rgb * 2.0 - 1.0);
#else
    if(hasNormal)
        N = normalize(texture(normalTexture, fragUV).rgb * 2.0 - 1.0);
#endif

    vec3 baseColor = fragColor;
#if BINDLESS == 1
    if(hasBaseColor)
        baseColor *= texture(textures[nonuniformEXT(materials[matIdx].TextureIndices[5])], fragUV).rgb;
    float ao = hasAO ? texture(textures[nonuniformEXT(materials[matIdx].TextureIndices[6])], fragUV).r : 1.0;
#else
    if(hasBaseColor)
        baseColor *= texture(baseColorTexture, fragUV).rgb;
    float ao = hasAO ? texture(aoTexture, fragUV).r : 1.0;
#endif

    vec3 V = normalize(vec3(inverse(sceneUBO.ViewMatrix)[3]) - fragPositionWorld);
    vec3 L = normalize(lightUBO.LightPos.xyz - fragPositionWorld);
    vec3 H = normalize(V + L);

#if BINDLESS == 1
    if(hasRoughness) roughness *= texture(textures[nonuniformEXT(materials[matIdx].TextureIndices[0])], fragUV).g;
    if(hasMetallic) metallic *= texture(textures[nonuniformEXT(materials[matIdx].TextureIndices[1])], fragUV).g;
#else
    if(hasRoughness) roughness *= texture(roughnessTexture, fragUV).g;
    if(hasMetallic) metallic *= texture(metallicTexture, fragUV).g;
#endif

    vec3 F0 = mix(vec3(0.04), baseColor, metallic);
    float NDF = distributionGGX(N, H, roughness);
    float G   = geometrySmith(N, V, L, roughness);
    vec3  F   = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 numerator = NDF * G * F;
    float denom = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
    vec3 specular = numerator / denom;

    vec3 kS = F;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
    vec3 diffuse = kD * baseColor / PI;

    float NdotL = max(dot(N, L), 0.0);
    vec3 result = (diffuse + specular) * lightUBO.LightColor.rgb * lightUBO.LightColor.a * NdotL;
    result += getIBLContribution(N, V, roughness, F0, kS, kD, baseColor);
    result *= ao;

#if BINDLESS == 1
    if(hasEmissive)
        result += texture(textures[nonuniformEXT(materials[matIdx].TextureIndices[3])], fragUV).rgb;
#else
    if(hasEmissive)
        result += texture(emissiveTexture, fragUV).rgb;
#endif

    outColor = vec4(result, 1.0);
}
