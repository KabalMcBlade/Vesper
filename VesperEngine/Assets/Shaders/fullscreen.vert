#version 450

// not used, just for pipeline definition
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;
layout(location = 4) in vec3 inMorphPos0;
layout(location = 5) in vec3 inMorphNorm0;
layout(location = 6) in vec3 inMorphPos1;
layout(location = 7) in vec3 inMorphNorm1;
layout(location = 8) in vec3 inMorphPos2;
layout(location = 9) in vec3 inMorphNorm2;
layout(location = 10) in vec3 inMorphPos3;
layout(location = 11) in vec3 inMorphNorm3;

layout (location = 0) out vec2 outUV;

void main()
{
	outUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	gl_Position = vec4(outUV * 2.0f - 1.0f, 0.0f, 1.0f);
}
