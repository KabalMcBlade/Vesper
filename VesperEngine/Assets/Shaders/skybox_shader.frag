#version 450

layout(location = 0) in vec3 fragPosition;
layout(location = 0) out vec4 outColor;

#if BINDLESS == 1
layout(set = 2, binding = 0) uniform samplerCube skyboxTexture;
#else
layout(set = 1, binding = 0) uniform samplerCube skyboxTexture;
#endif

void main()
{
    outColor = texture(skyboxTexture, fragPosition);
}
