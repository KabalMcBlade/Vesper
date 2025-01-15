#version 450

layout(push_constant) uniform PushConstants 
{
    mat4 view;
    mat4 projection;
    int projectionType; // 0: Equirectangular, 1: Cube, 2: Hemisphere, 3: Parabolic, 4: LatLongCubemap
} pushConstants;

layout(location = 0) in vec2 inUV;         // UV coordinates from the vertex shader
layout(location = 0) out vec4 outColor;      // Output color

layout(binding = 0) uniform sampler2D hdrTexture; // Input HDR texture

vec3 SampleEquirectangular(vec2 uv) 
{
    float phi = uv.x * 2.0 * 3.14159265359; // Longitude
    float theta = uv.y * 3.14159265359;     // Latitude
    return vec3(
        cos(phi) * sin(theta),
        cos(theta),
        sin(phi) * sin(theta)
    );
}

vec3 SampleLatLongCubemap(vec2 uv) 
{
    float phi = uv.x * 2.0 * 3.14159265359; // Longitude
    float theta = uv.y * 3.14159265359;     // Latitude
    return vec3(
        cos(phi) * sin(theta),
        cos(theta),
        sin(phi) * sin(theta)
    );
}

vec3 SampleHemisphere(vec2 uv) 
{
    float phi = uv.x * 2.0 * 3.14159265359; // Longitude
    float theta = uv.y * 1.57079632679;     // Latitude (half-sphere)
    return vec3(
        cos(phi) * sin(theta),
        cos(theta),
        sin(phi) * sin(theta)
    );
}

vec3 SampleParabolic(vec2 uv) 
{
    vec2 parabolicUV = (uv - 0.5) * 2.0; // Remap to [-1, 1]
    float z = 1.0 - dot(parabolicUV, parabolicUV);
    return normalize(vec3(parabolicUV, z));
}

void main() 
{
    vec2 uv = inUV;
    vec3 direction;

    if (pushConstants.projectionType == 0) { // Equirectangular
        direction = SampleEquirectangular(uv);
    } else if (pushConstants.projectionType == 1) { // Cube
        // No reprojection required; directly sample the texture
        direction = vec3(uv, 0.0);
    } else if (pushConstants.projectionType == 2) { // Hemisphere
        direction = SampleHemisphere(uv);
    } else if (pushConstants.projectionType == 3) { // Parabolic
        direction = SampleParabolic(uv);
    } else if (pushConstants.projectionType == 4) { // LatLongCubemap
        direction = SampleLatLongCubemap(uv);
    } else {
        direction = vec3(0.0, 0.0, 1.0); // Fallback
    }

    // Transform direction using view and projection matrices
    vec4 transformedDir = pushConstants.projection * pushConstants.view * vec4(direction, 0.0);

    // Convert transformed direction to UVs for the HDR texture
    vec3 hdrSample = texture(hdrTexture, uv).rgb;

    outColor = vec4(hdrSample, 1.0);
}
