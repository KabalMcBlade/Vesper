#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 fragColor;

layout(push_constant) uniform Push
{
	mat4 projectionViewModelMatrix;		// transform the vertex position 
	mat4 normalModelMatrix;
} push;

// like the sun and is defined in world space
const vec3 kDirectionToLight = normalize(vec3(1.0, -5.0, -2.0));

const float kAmbientIllumination = 0.2;

void main()
{
	gl_Position = push.projectionViewModelMatrix * vec4(position, 1.0);

	vec3 normalWorldSpace = normalize(mat3(push.normalModelMatrix) * normal);

	float lightIntensity = kAmbientIllumination + max(dot(normalWorldSpace, kDirectionToLight), 0);

	fragColor = lightIntensity * color;
}