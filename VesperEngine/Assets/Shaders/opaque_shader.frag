#version 450

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


layout(set = 2, binding = 0) uniform sampler2D diffuseTexture;
layout(set = 2, binding = 1) uniform sampler2D specularTexture;
layout(set = 2, binding = 2) uniform sampler2D ambientTexture;
layout(set = 2, binding = 3) uniform sampler2D normalTexture;

layout(std140, set = 2, binding = 4) uniform MaterialData 
{
    vec4 AmbientColor;
    vec4 DiffuseColor;
    vec4 SpecularColor;
    vec4 EmissionColor;
    float Shininess;
    int TextureFlags;      // 0: HasAmbientTexture, 1: HasDiffuseTexture, 2: HasSpecularTexture, 3: HasNormalTexture
} material;

bool getFlag(int flags, int bitIndex) 
{
    return ((flags >> bitIndex) & 1) != 0;
}

layout(push_constant) uniform PushConstants 
{
    vec3 colorTint;
} pushConstants;


// Specialization constant for brightness adjustment
layout(constant_id = 0) const float kBrightnessFactor = 1.0;

void main() 
{
    // DEBUG UV COLOR
    //outColor = vec4(fragUV, 0.0, 1.0); // Visualize UVs as colors
    
    // get flags
    bool bHasAmbientTexture = getFlag(material.TextureFlags, 0);    // Extract bit 0 - HasAmbientTexture
    bool bHasDiffuseTexture = getFlag(material.TextureFlags, 1);    // Extract bit 1 - HasDiffuseTexture  
    bool bHasSpecularTexture = getFlag(material.TextureFlags, 2);   // Extract bit 2 - HasSpecularTexture
    bool bHasNormalTexture = getFlag(material.TextureFlags, 3);     // Extract bit 3 - HasNormalTexture

    vec3 normal = normalize(fragNormalWorld);
    if (bHasNormalTexture) 
    {
        vec3 normalMap = texture(normalTexture, fragUV).rgb * 2.0 - 1.0;
        normal = normalize(normalMap);
    }

    vec3 cameraPosition = vec3(inverse(sceneUBO.ViewMatrix)[3]);
    vec3 viewDir = normalize(cameraPosition - fragPositionWorld);
    vec3 lightDir = normalize(lightUBO.LightPos.xyz - fragPositionWorld);

    vec4 combinedLighting = vec4(0.0);

    // Ambient Contribution
    if (bHasAmbientTexture) 
    {
        vec4 ambientTextureColor = texture(ambientTexture, fragUV);
        vec4 finalAmbientColor = ambientTextureColor * material.AmbientColor;
        combinedLighting += finalAmbientColor * vec4(sceneUBO.AmbientColor.rgb, 1.0) * sceneUBO.AmbientColor.a;
    }

    // Diffuse Contribution
    if (bHasDiffuseTexture) 
    {
        vec4 diffuseTextureColor = texture(diffuseTexture, fragUV);
        vec4 finalDiffuseColor = diffuseTextureColor * material.DiffuseColor;

        float diffIntensity = max(dot(normal, lightDir), 0.0);
        combinedLighting += finalDiffuseColor * vec4(lightUBO.LightColor.rgb * diffIntensity * lightUBO.LightColor.a, 1.0);
    }

    // Specular Contribution
    if (bHasSpecularTexture) 
    {
        vec4 specularTextureColor = texture(specularTexture, fragUV);       
        vec4 finalSpecularColor = specularTextureColor * material.SpecularColor;

        vec3 reflectDir = reflect(-lightDir, normal);
        float specIntensity = pow(max(dot(viewDir, reflectDir), 0.0), material.Shininess);
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
    combinedLighting += material.EmissionColor;
    
    // Apply Color Tint
    combinedLighting.rgb *= pushConstants.colorTint;

    // Clamp Final Result
    outColor = clamp(combinedLighting * kBrightnessFactor, 0.0, 1.0);
}
