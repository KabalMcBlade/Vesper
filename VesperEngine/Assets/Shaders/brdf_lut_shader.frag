#version 450

//layout(location = 0) in vec2 texCoord;

layout(location = 0) out vec4 outBRDF;

layout(std140, push_constant) uniform Push 
{
    vec2 Resolution;
} push;

const float PI = 3.14159265359;

float RadicalInverse_VdC(uint bits) 
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // Divide by 2^32
}

vec2 Hammersley(uint i, uint N) 
{
    return vec2(float(i) / float(N), RadicalInverse_VdC(i));
}

float GeometrySchlickGGX(float NdotV, float roughness) 
{
    float k = (roughness * roughness) / 2.0; // GGX k term
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(float NdotV, float NdotL, float roughness) 
{
    float ggxV = GeometrySchlickGGX(NdotV, roughness);
    float ggxL = GeometrySchlickGGX(NdotL, roughness);
    return ggxV * ggxL;
}


vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness) 
{
    float a = roughness * roughness;
    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;

    vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);

    return tangent * H.x + bitangent * H.y + N * H.z;
}

vec3 FresnelSchlick(float VdotH, vec3 F0) 
{
    return F0 + (1.0 - F0) * pow(1.0 - VdotH, 5.0);
}

vec2 IntegrateBRDF(float NdotV, float roughness) 
{
    const uint SAMPLE_COUNT = 1024u;
    vec3 V = vec3(sqrt(1.0 - NdotV * NdotV), 0.0, NdotV);

    float A = 0.0;
    float B = 0.0;
    for (uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        vec3 H = ImportanceSampleGGX(Xi, vec3(0.0, 0.0, 1.0), roughness);
        vec3 L = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(L.z, 0.0);
        float NdotH = max(H.z, 0.0);
        float VdotH = max(dot(V, H), 0.0);

        if (NdotL > 0.0) 
        {
            float G = GeometrySmith(NdotV, NdotL, roughness);

            // FresnelSchlick with F0 = vec3(1.0), as this is BRDF integration
            vec3 F0 = vec3(1.0);
            vec3 F = FresnelSchlick(VdotH, F0);

            A += G * F.r;       // Use the red channel for BRDF integration
            B += G * (1.0 - F.r);
        }
    }

    A /= float(SAMPLE_COUNT);
    B /= float(SAMPLE_COUNT);
    return vec2(A, B);
}

void main() 
{
    //outBRDF = vec4(texCoord, 0.0, 1.0); // Map texCoord to colors
    //outBRDF = vec4(NdotV, roughness, 0.0, 1.0);

    float NdotV = gl_FragCoord.x / push.Resolution.x;
    float roughness = gl_FragCoord.y / push.Resolution.y;
    vec2 brdf = IntegrateBRDF(NdotV, roughness);
    
    outBRDF = vec4(brdf, 0.0, 1.0); // Fill remaining channels with appropriate values
}
