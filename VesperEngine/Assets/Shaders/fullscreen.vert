#version 450

// not used, just for pipeline definition
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;

layout(location = 0) out vec2 texCoord;

void main() 
{
    vec2 positions[3] = vec2[](
        vec2(-1.0, -1.0), // Bottom-left
        vec2( 3.0, -1.0), // Bottom-right, extended outside the screen
        vec2(-1.0,  3.0)  // Top-left, extended outside the screen
    );

    vec2 pos = positions[gl_VertexIndex];
    texCoord = pos * 0.5 + 0.5; // Map [-1, 1] to [0, 1]
    gl_Position = vec4(pos, 0.0, 1.0);
}
