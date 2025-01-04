#version 450

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 fragPosition;

layout(push_constant) uniform PushConstants 
{
    mat4 viewProjection;
} pushConstants;

void main() 
{
    fragPosition = inPosition;
    gl_Position = pushConstants.viewProjection * vec4(inPosition, 1.0);
}
