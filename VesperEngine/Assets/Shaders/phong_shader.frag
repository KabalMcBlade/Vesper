#version 450

#if BINDLESS == 1
#extension GL_EXT_nonuniform_qualifier : require
#endif


layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPositionWorld;
layout(location = 2) in vec3 fragNormalWorld;
layout(location = 3) in vec2 fragUV1;
layout(location = 4) in vec2 fragUV2;

layout(location = 0) out vec4 outColor;

layout(std140, set = 0, binding = 0) uniform SceneUBO 
{
    mat4 ProjectionMatrix;
    mat4 ViewMatrix;
    vec4 CameraPosition;
    vec4 AmbientColor; // w is intensity
} sceneUBO;

const int c_MaxDirectionalLights = 16;
const int c_MaxPointLights = 256;
const int c_MaxSpotLights = 256;


struct DirectionalLightData { vec4 Direction; vec4 Color; };
struct PointLightData { vec4 Position; vec4 Color; vec4 Attenuation; };
struct SpotLightData { vec4 Position; vec4 Direction; vec4 Color; vec4 Params; };

layout(std140, set = 0, binding = 1) uniform LightsUBO
{
    int DirectionalCount;
    int PointCount;
    int SpotCount;
    int _padding;
    DirectionalLightData DirectionalLights[c_MaxDirectionalLights];
    PointLightData PointLights[c_MaxPointLights];
    SpotLightData SpotLights[c_MaxSpotLights];
} lightsUBO;


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
    //outColor = vec4(fragUV1, 0.0, 1.0); // Visualize UVs as colors
    
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
        alpha *= texture(textures[nonuniformEXT(materials[matIdx].TextureIndices[4])], fragUV1).r;
    }
#else
    if (bHasAlphaTexture)
    {
        alpha *= texture(alphaTexture, fragUV1).r;
    }
#endif

    vec3 normal = normalize(fragNormalWorld);
    if (bHasNormalTexture) 
    {
#if BINDLESS == 1
        vec3 normalMap = texture(textures[nonuniformEXT(materials[matIdx].TextureIndices[3])], fragUV1).rgb * 2.0 - 1.0;
#else
        vec3 normalMap = texture(normalTexture, fragUV1).rgb * 2.0 - 1.0;
#endif

        normal = normalize(normalMap);
    }

    vec3 cameraPosition = sceneUBO.CameraPosition.xyz;
    vec3 viewDir = normalize(cameraPosition - fragPositionWorld);

    vec4 combinedLighting = vec4(0.0);

    // Ambient Contribution
    if (bHasAmbientTexture) 
    {
#if BINDLESS == 1
        vec4 ambientTextureColor = texture(textures[nonuniformEXT(materials[matIdx].TextureIndices[0])], fragUV1);
#else
        vec4 ambientTextureColor = texture(ambientTexture, fragUV1);
#endif

        vec4 finalAmbientColor = ambientTextureColor * ambientColor;
        combinedLighting += finalAmbientColor * vec4(sceneUBO.AmbientColor.rgb, 1.0) * sceneUBO.AmbientColor.a;
    }

    // Diffuse Contribution
    if (bHasDiffuseTexture) 
    {
#if BINDLESS == 1
        vec4 diffuseTextureColor = texture(textures[nonuniformEXT(materials[matIdx].TextureIndices[1])], fragUV1);
#else
        vec4 diffuseTextureColor = texture(diffuseTexture, fragUV1);
#endif

        vec4 finalDiffuseColor = diffuseTextureColor * diffuseColor;

        for (int i = 0; i < lightsUBO.DirectionalCount; ++i)
        {
            vec3 lDir = normalize(lightsUBO.DirectionalLights[i].Direction.xyz);
            float diffIntensity = max(dot(normal, lDir), 0.0);
            vec3 lightCol = lightsUBO.DirectionalLights[i].Color.rgb * lightsUBO.DirectionalLights[i].Color.a;
            combinedLighting += finalDiffuseColor * vec4(lightCol * diffIntensity, 1.0);
        }

        for (int i = 0; i < lightsUBO.PointCount; ++i)
        {
            vec3 L = lightsUBO.PointLights[i].Position.xyz - fragPositionWorld;
            float dist = length(L);
            vec3 lDir = normalize(L);
            float att = 1.0 / (lightsUBO.PointLights[i].Attenuation.x + lightsUBO.PointLights[i].Attenuation.y * dist + lightsUBO.PointLights[i].Attenuation.z * dist * dist);
            float diffIntensity = max(dot(normal, lDir), 0.0);
            vec3 lightCol = lightsUBO.PointLights[i].Color.rgb * lightsUBO.PointLights[i].Color.a * att;
            combinedLighting += finalDiffuseColor * vec4(lightCol * diffIntensity, 1.0);
        }

        for (int i = 0; i < lightsUBO.SpotCount; ++i)
        {
            vec3 L = lightsUBO.SpotLights[i].Position.xyz - fragPositionWorld;
            float dist = length(L);
            vec3 lDir = normalize(L);
            float theta = dot(lDir, normalize(-lightsUBO.SpotLights[i].Direction.xyz));
            float epsilon = lightsUBO.SpotLights[i].Params.x - lightsUBO.SpotLights[i].Params.y;
            float intensity = clamp((theta - lightsUBO.SpotLights[i].Params.y) / epsilon, 0.0, 1.0);
            float att = intensity / (dist * dist);
            float diffIntensity = max(dot(normal, lDir), 0.0);
            vec3 lightCol = lightsUBO.SpotLights[i].Color.rgb * lightsUBO.SpotLights[i].Color.a * att;
            combinedLighting += finalDiffuseColor * vec4(lightCol * diffIntensity, 1.0);
        }
    }

    // Specular Contribution
    if (bHasSpecularTexture) 
    {
#if BINDLESS == 1
        vec4 specularTextureColor = texture(textures[nonuniformEXT(materials[matIdx].TextureIndices[2])], fragUV1);
#else
        vec4 specularTextureColor = texture(specularTexture, fragUV1);
#endif
        
        vec4 finalSpecularColor = specularTextureColor * specularColor;

        for (int i = 0; i < lightsUBO.DirectionalCount; ++i)
        {
            vec3 lDir = normalize(lightsUBO.DirectionalLights[i].Direction.xyz);
            vec3 reflectDir = reflect(-lDir, normal);
            float specIntensity = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
            vec3 lightCol = lightsUBO.DirectionalLights[i].Color.rgb * lightsUBO.DirectionalLights[i].Color.a;
            combinedLighting += finalSpecularColor * vec4(lightCol * clamp(specIntensity, 0.0, 1.0), 1.0);
        }

        for (int i = 0; i < lightsUBO.PointCount; ++i)
        {
            vec3 L = lightsUBO.PointLights[i].Position.xyz - fragPositionWorld;
            float dist = length(L);
            vec3 lDir = normalize(L);
            float att = 1.0 / (lightsUBO.PointLights[i].Attenuation.x + lightsUBO.PointLights[i].Attenuation.y * dist + lightsUBO.PointLights[i].Attenuation.z * dist * dist);
            vec3 reflectDir = reflect(-lDir, normal);
            float specIntensity = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
            vec3 lightCol = lightsUBO.PointLights[i].Color.rgb * lightsUBO.PointLights[i].Color.a * att;
            combinedLighting += finalSpecularColor * vec4(lightCol * clamp(specIntensity, 0.0, 1.0), 1.0);
        }

        for (int i = 0; i < lightsUBO.SpotCount; ++i)
        {
            vec3 L = lightsUBO.SpotLights[i].Position.xyz - fragPositionWorld;
            float dist = length(L);
            vec3 lDir = normalize(L);
            float theta = dot(lDir, normalize(-lightsUBO.SpotLights[i].Direction.xyz));
            float epsilon = lightsUBO.SpotLights[i].Params.x - lightsUBO.SpotLights[i].Params.y;
            float intensity = clamp((theta - lightsUBO.SpotLights[i].Params.y) / epsilon, 0.0, 1.0);
            float att = intensity / (dist * dist);
            vec3 reflectDir = reflect(-lDir, normal);
            float specIntensity = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
            vec3 lightCol = lightsUBO.SpotLights[i].Color.rgb * lightsUBO.SpotLights[i].Color.a * att;
            combinedLighting += finalSpecularColor * vec4(lightCol * clamp(specIntensity, 0.0, 1.0), 1.0);
        }
    }

    // fallback vertex color
    if (!bHasAmbientTexture && !bHasDiffuseTexture && !bHasSpecularTexture) 
    {
        vec3 ambientLight = sceneUBO.AmbientColor.rgb * sceneUBO.AmbientColor.a;
        vec3 diffuseAccum = vec3(0.0);
                
        for (int i = 0; i < lightsUBO.DirectionalCount; ++i)
        {
            vec3 lDir = normalize(lightsUBO.DirectionalLights[i].Direction.xyz);
            vec3 lightColor = lightsUBO.DirectionalLights[i].Color.rgb * lightsUBO.DirectionalLights[i].Color.a;
            diffuseAccum += lightColor * max(dot(fragNormalWorld, lDir), 0.0);
        }

        for (int i = 0; i < lightsUBO.PointCount; ++i)
        {
            vec3 L = lightsUBO.PointLights[i].Position.xyz - fragPositionWorld;
            float dist = length(L);
            vec3 lDir = normalize(L);
            float att = 1.0 / (lightsUBO.PointLights[i].Attenuation.x + lightsUBO.PointLights[i].Attenuation.y * dist + lightsUBO.PointLights[i].Attenuation.z * dist * dist);
            vec3 lightColor = lightsUBO.PointLights[i].Color.rgb * lightsUBO.PointLights[i].Color.a * att;
            diffuseAccum += lightColor * max(dot(fragNormalWorld, lDir), 0.0);
        }

        for (int i = 0; i < lightsUBO.SpotCount; ++i)
        {
            vec3 L = lightsUBO.SpotLights[i].Position.xyz - fragPositionWorld;
            float dist = length(L);
            vec3 lDir = normalize(L);
            float theta = dot(lDir, normalize(-lightsUBO.SpotLights[i].Direction.xyz));
            float epsilon = lightsUBO.SpotLights[i].Params.x - lightsUBO.SpotLights[i].Params.y;
            float intensity = clamp((theta - lightsUBO.SpotLights[i].Params.y) / epsilon, 0.0, 1.0);
            float att = intensity / (dist * dist);
            vec3 lightColor = lightsUBO.SpotLights[i].Color.rgb * lightsUBO.SpotLights[i].Color.a * att;
            diffuseAccum += lightColor * max(dot(fragNormalWorld, lDir), 0.0);
        }

        combinedLighting = vec4((diffuseAccum + ambientLight) * fragColor, 1.0);
    }

    // Emission Contribution (Always Added)
    combinedLighting += emissionColor;

    // Clamp Final Result
    outColor = clamp(vec4(combinedLighting.rgb, alpha) * kBrightnessFactor, 0.0, 1.0);
}
