#version 450

layout(location = 0) in vec3 fragPosition;
layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform samplerCube environmentMap;

void main()
{
    outColor = texture(environmentMap, normalize(fragPosition));
}
