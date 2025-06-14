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
    vec4 AmbientColor; // w is intensity
} sceneUBO;

layout(std140, set = 0, binding = 1) uniform LightUBO 
{
    vec4 LightPos;
    vec4 LightColor; // w is intensity
} lightUBO;


#if BINDLESS == 1

layout(set = 1, binding = 0) uniform sampler2D textures[];

layout(std140, set = 1, binding = 1) uniform MaterialData
{
    vec4 AmbientColor;
    vec4 DiffuseColor;
    vec4 SpecularColor;
    vec4 EmissionColor;
    float Shininess;
    int TextureIndices[5]; // Indices for the textures: [Ambient, Diffuse, Specular, Normal, Alpha] (Value of -1 indicates it's missing)
} materials[];

layout(std140, set = 3, binding = 0) uniform MaterialIndexUBO
{
    int materialIndex;
} materialIndexUBO;

#else

layout(set = 2, binding = 0) uniform sampler2D ambientTexture;
layout(set = 2, binding = 1) uniform sampler2D diffuseTexture;
layout(set = 2, binding = 2) uniform sampler2D specularTexture;
layout(set = 2, binding = 3) uniform sampler2D normalTexture;

layout(set = 2, binding = 4) uniform sampler2D alphaTexture;

layout(std140, set = 2, binding = 5) uniform MaterialData
{
    vec4 AmbientColor;
    vec4 DiffuseColor;
    vec4 SpecularColor;
    vec4 EmissionColor;
    float Shininess;
    int TextureIndices[5]; // Indices for the textures: [Ambient, Diffuse, Specular, Normal, Alpha] (Value of -1 means does not exist, >= 0 exist)
} material;

#endif

// Specialization constant for brightness adjustment
layout(constant_id = 0) const float kBrightnessFactor = 1.0;

void main() 
{
    // DEBUG UV COLOR
    //outColor = vec4(fragUV, 0.0, 1.0); // Visualize UVs as colors
    
#if BINDLESS == 1
    // Access material data using the index
    int matIdx = materialIndexUBO.materialIndex;
    vec4 ambientColor = materials[matIdx].AmbientColor;
    vec4 diffuseColor = materials[matIdx].DiffuseColor;
    vec4 specularColor = materials[matIdx].SpecularColor;
    vec4 emissionColor = materials[matIdx].EmissionColor;
    float shininess = materials[matIdx].Shininess;
    bool bHasAmbientTexture = materials[matIdx].TextureIndices[0] != -1;
    bool bHasDiffuseTexture = materials[matIdx].TextureIndices[1] != -1;
    bool bHasSpecularTexture = materials[matIdx].TextureIndices[2] != -1;
    bool bHasNormalTexture = materials[matIdx].TextureIndices[3] != -1;
    bool bHasAlphaTexture = materials[matIdx].TextureIndices[4] != -1;
#else
    vec4 ambientColor = material.AmbientColor;
    vec4 diffuseColor = material.DiffuseColor;
    vec4 specularColor = material.SpecularColor;
    vec4 emissionColor = material.EmissionColor;
    float shininess = material.Shininess;
    bool bHasAmbientTexture = material.TextureIndices[0] != -1;
    bool bHasDiffuseTexture = material.TextureIndices[1] != -1;
    bool bHasSpecularTexture = material.TextureIndices[2] != -1;
    bool bHasNormalTexture = material.TextureIndices[3] != -1;
    bool bHasAlphaTexture = material.TextureIndices[4] != -1;
#endif

    float alpha = diffuseColor.a;
#if BINDLESS == 1
    if (bHasAlphaTexture)
    {
        alpha *= texture(textures[nonuniformEXT(materials[matIdx].TextureIndices[4])], fragUV).r;
    }
#else
    if (bHasAlphaTexture)
    {
        alpha *= texture(alphaTexture, fragUV).r;
    }
#endif

    vec3 normal = normalize(fragNormalWorld);
    if (bHasNormalTexture) 
    {
#if BINDLESS == 1
        vec3 normalMap = texture(textures[nonuniformEXT(materials[matIdx].TextureIndices[3])], fragUV).rgb * 2.0 - 1.0;
#else
        vec3 normalMap = texture(normalTexture, fragUV).rgb * 2.0 - 1.0;
#endif

        normal = normalize(normalMap);
    }

    vec3 cameraPosition = vec3(inverse(sceneUBO.ViewMatrix)[3]);
    vec3 viewDir = normalize(cameraPosition - fragPositionWorld);
    vec3 lightDir = normalize(lightUBO.LightPos.xyz - fragPositionWorld);

    vec4 combinedLighting = vec4(0.0);

    // Ambient Contribution
    if (bHasAmbientTexture) 
    {
#if BINDLESS == 1
        vec4 ambientTextureColor = texture(textures[nonuniformEXT(materials[matIdx].TextureIndices[0])], fragUV);
#else
        vec4 ambientTextureColor = texture(ambientTexture, fragUV);
#endif

        vec4 finalAmbientColor = ambientTextureColor * ambientColor;
        combinedLighting += finalAmbientColor * vec4(sceneUBO.AmbientColor.rgb, 1.0) * sceneUBO.AmbientColor.a;
    }

    // Diffuse Contribution
    if (bHasDiffuseTexture) 
    {
#if BINDLESS == 1
        vec4 diffuseTextureColor = texture(textures[nonuniformEXT(materials[matIdx].TextureIndices[1])], fragUV);
#else
        vec4 diffuseTextureColor = texture(diffuseTexture, fragUV);
#endif

        vec4 finalDiffuseColor = diffuseTextureColor * diffuseColor;

        float diffIntensity = max(dot(normal, lightDir), 0.0);
        combinedLighting += finalDiffuseColor * vec4(lightUBO.LightColor.rgb * diffIntensity * lightUBO.LightColor.a, 1.0);
    }

    // Specular Contribution
    if (bHasSpecularTexture) 
    {
#if BINDLESS == 1
        vec4 specularTextureColor = texture(textures[nonuniformEXT(materials[matIdx].TextureIndices[2])], fragUV);
#else
        vec4 specularTextureColor = texture(specularTexture, fragUV);
#endif
        
        vec4 finalSpecularColor = specularTextureColor * specularColor;

        vec3 reflectDir = reflect(-lightDir, normal);
        float specIntensity = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
        combinedLighting += finalSpecularColor * lightUBO.LightColor * clamp(specIntensity, 0.0, 1.0);
    }

    // fallback vertex color
    if (!bHasAmbientTexture && !bHasDiffuseTexture && !bHasSpecularTexture) 
    {
        float attenuation = 1.0 / dot(lightDir, lightDir); // distance squared
        vec3 lightColor = lightUBO.LightColor.rgb * lightUBO.LightColor.a * attenuation;
        vec3 ambientLight = sceneUBO.AmbientColor.rgb * sceneUBO.AmbientColor.a;
        vec3 diffuseLight = lightColor * max(dot(fragNormalWorld, normalize(lightDir)), 0);
        combinedLighting = vec4((diffuseLight + ambientLight) * fragColor, 1.0);
    }

    // Emission Contribution (Always Added)
    combinedLighting += emissionColor;

    // Clamp Final Result
    outColor = clamp(vec4(combinedLighting.rgb, alpha) * kBrightnessFactor, 0.0, 1.0);
}
