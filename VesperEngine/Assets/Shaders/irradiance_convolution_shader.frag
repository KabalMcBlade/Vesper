#version 450

layout(location = 0) in vec3 fragPosition;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform samplerCube environmentMap;

const float PI = 3.14159265359;

vec3 SampleHemisphere(vec3 normal) 
{
    // Generate random sample in a hemisphere aligned with normal
    float phi = 2.0 * 3.14159265359 * fract(sin(dot(normal.xy, vec2(12.9898, 78.233))) * 43758.5453123);
    float cosTheta = sqrt(1.0 - fract(sin(dot(normal.zx, vec2(34.1234, 12.4567))) * 7867.123));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    vec3 sampleDir;
    sampleDir.x = cos(phi) * sinTheta;
    sampleDir.y = sin(phi) * sinTheta;
    sampleDir.z = cosTheta;

    // Construct tangent space for the hemisphere
    vec3 up = abs(normal.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, normal));
    vec3 bitangent = cross(normal, tangent);

    // Transform the sample into the hemisphere aligned with the 'normal'
    return tangent * sampleDir.x + bitangent * sampleDir.y + normal * sampleDir.z;
}

void main() 
{
    vec3 normal = normalize(fragPosition);

    vec3 irradiance = vec3(0.0);
    const uint SAMPLE_COUNT = 1024u;
    for (uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        vec3 sampleDir = SampleHemisphere(normal); // Generate random sample
        float NdotL = max(dot(normal, sampleDir), 0.0);
        irradiance += texture(environmentMap, sampleDir).rgb * NdotL;
    }

    irradiance *= (1.0 / float(SAMPLE_COUNT)) * PI;
    outColor = vec4(irradiance, 1.0);
}
