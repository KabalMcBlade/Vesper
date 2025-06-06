#version 450

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 fragPosition;

layout(std140, set = 0, binding = 0) uniform SceneUBO
{
    mat4 ProjectionMatrix;
    mat4 ViewMatrix;
    vec4 AmbientColor;
} sceneUBO;

void main()
{
    fragPosition = inPosition;
    gl_Position = sceneUBO.ProjectionMatrix * sceneUBO.ViewMatrix *
                  vec4(inPosition, 1.0);
}
