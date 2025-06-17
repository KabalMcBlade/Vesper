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
    vec4 AmbientColor; // xyz color, w intensity
} sceneUBO;

layout(std140, set = 0, binding = 1) uniform LightUBO
{
    vec4 LightPos;
    vec4 LightColor; // rgb color,  a intensity
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

const float M_PI = 3.141592653589793;
const float c_MinRoughness = 0.04;

struct PBRInfo
{
    float NdotL;
    float NdotV;
    float NdotH;
    float LdotH;
    float VdotH;
    float perceptualRoughness;
    float metalness;
    vec3 reflectance0;
    vec3 reflectance90;
    float alphaRoughness;
    vec3 diffuseColor;
    vec3 specularColor;
};

vec4 tonemap(vec4 color)
{
    // TODO: implement proper tonemapping
    return color;
}

vec4 SRGBtoLINEAR(vec4 srgbIn)
{
    return pow(srgbIn, vec4(2.2));
}

vec3 diffuse(PBRInfo pbrInputs)
{
    return pbrInputs.diffuseColor / M_PI;
}

vec3 specularReflection(PBRInfo pbrInputs)
{
    return pbrInputs.reflectance0 + (pbrInputs.reflectance90 - pbrInputs.reflectance0) * pow(clamp(1.0 - pbrInputs.VdotH, 0.0, 1.0), 5.0);
}

float geometricOcclusion(PBRInfo pbrInputs)
{
    float NdotL = pbrInputs.NdotL;
    float NdotV = pbrInputs.NdotV;
    float r = pbrInputs.alphaRoughness;

    float attenuationL = 2.0 * NdotL / (NdotL + sqrt(r * r + (1.0 - r * r) * (NdotL * NdotL)));
    float attenuationV = 2.0 * NdotV / (NdotV + sqrt(r * r + (1.0 - r * r) * (NdotV * NdotV)));
    return attenuationL * attenuationV;
}

float microfacetDistribution(PBRInfo pbrInputs)
{
    float roughnessSq = pbrInputs.alphaRoughness * pbrInputs.alphaRoughness;
    float f = (pbrInputs.NdotH * roughnessSq - pbrInputs.NdotH) * pbrInputs.NdotH + 1.0;
    return roughnessSq / (M_PI * f * f);
}

vec3 getNormal(bool hasNormal)
{
    vec3 N = normalize(fragNormalWorld);
#if BINDLESS == 1
    if (hasNormal)
        N = normalize(texture(textures[nonuniformEXT(materials[materialIndexUBO.materialIndex].TextureIndices[4])], fragUV).xyz * 2.0 - 1.0);
#else
    if (hasNormal)
        N = normalize(texture(normalTexture, fragUV).xyz * 2.0 - 1.0);
#endif
    return N;
}

vec3 getIBLContribution(PBRInfo pbrInputs, vec3 n, vec3 reflection)
{
    float mipCount = float(textureQueryLevels(prefilteredEnvMap));

    float lod = pbrInputs.perceptualRoughness * (mipCount - 1.0);

    vec3 brdf = texture(brdfLUT, vec2(pbrInputs.NdotV, 1.0 - pbrInputs.perceptualRoughness)).rgb;
    vec3 diffuseLight = SRGBtoLINEAR(tonemap(texture(irradianceMap, n))).rgb;
    vec3 specularLight = SRGBtoLINEAR(tonemap(textureLod(prefilteredEnvMap, reflection, lod))).rgb;

    vec3 diffuse = diffuseLight * pbrInputs.diffuseColor;
    vec3 specular = specularLight * (pbrInputs.specularColor * brdf.x + brdf.y);

    // Hardcoded ambient scale for now
    diffuse *= 1.0;
    specular *= 1.0;

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

    vec3 n = getNormal(hasNormal);

    vec4 baseColor = vec4(fragColor, 1.0);
#if BINDLESS == 1
    if (hasBaseColor)
        baseColor *= SRGBtoLINEAR(texture(textures[nonuniformEXT(materials[matIdx].TextureIndices[5])], fragUV));
    float ao = hasAO ? texture(textures[nonuniformEXT(materials[matIdx].TextureIndices[6])], fragUV).r : 1.0;
#else
    if (hasBaseColor)
        baseColor *= SRGBtoLINEAR(texture(baseColorTexture, fragUV));
    float ao = hasAO ? texture(aoTexture, fragUV).r : 1.0;
#endif


#if BINDLESS == 1
    if(hasRoughness) roughness *= texture(textures[nonuniformEXT(materials[matIdx].TextureIndices[0])], fragUV).g;
    if(hasMetallic) metallic *= texture(textures[nonuniformEXT(materials[matIdx].TextureIndices[1])], fragUV).b;
#else
    if(hasRoughness) roughness *= texture(roughnessTexture, fragUV).g;
    if(hasMetallic) metallic *= texture(metallicTexture, fragUV).b;
#endif

    roughness = clamp(roughness, c_MinRoughness, 1.0);
    metallic = clamp(metallic, 0.0, 1.0);

    float alphaRoughness = roughness * roughness;

    vec3 f0 = vec3(0.04);
    vec3 diffuseColor = baseColor.rgb * (1.0 - f0);
    diffuseColor *= 1.0 - metallic;
    vec3 specularColor = mix(f0, baseColor.rgb, metallic);

    float reflectance = max(max(specularColor.r, specularColor.g), specularColor.b);
    float reflectance90 = clamp(reflectance * 25.0, 0.0, 1.0);
    vec3 specularEnvironmentR0 = specularColor;
    vec3 specularEnvironmentR90 = vec3(1.0) * reflectance90;

    vec3 v = normalize(vec3(inverse(sceneUBO.ViewMatrix)[3]) - fragPositionWorld);
    vec3 l = normalize(lightUBO.LightPos.xyz - fragPositionWorld);
    vec3 h = normalize(l + v);
    vec3 reflection = normalize(reflect(-v, n));

    float NdotL = clamp(dot(n, l), 0.001, 1.0);
    float NdotV = clamp(abs(dot(n, v)), 0.001, 1.0);
    float NdotH = clamp(dot(n, h), 0.0, 1.0);
    float LdotH = clamp(dot(l, h), 0.0, 1.0);
    float VdotH = clamp(dot(v, h), 0.0, 1.0);

    PBRInfo pbrInputs = PBRInfo(
        NdotL,
        NdotV,
        NdotH,
        LdotH,
        VdotH,
        roughness,
        metallic,
        specularEnvironmentR0,
        specularEnvironmentR90,
        alphaRoughness,
        diffuseColor,
        specularColor
    );

    vec3 F = specularReflection(pbrInputs);
    float G = geometricOcclusion(pbrInputs);
    float D = microfacetDistribution(pbrInputs);

    vec3 lightColor = lightUBO.LightColor.rgb * lightUBO.LightColor.a;
    vec3 diffuseContrib = (1.0 - F) * diffuse(pbrInputs);
    vec3 specContrib = F * G * D / (4.0 * NdotL * NdotV);
    vec3 color = NdotL * lightColor * (diffuseContrib + specContrib);

    color += getIBLContribution(pbrInputs, n, reflection);
    color = mix(color, color * ao, 1.0);

#if BINDLESS == 1
    if(hasEmissive)
        color += texture(textures[nonuniformEXT(materials[matIdx].TextureIndices[3])], fragUV).rgb;
#else
    if(hasEmissive)
        color += texture(emissiveTexture, fragUV).rgb;
#endif

    outColor = vec4(color, baseColor.a);
}
