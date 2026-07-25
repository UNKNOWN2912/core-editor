#version 450

layout(location = 0) out vec4 outputColor;
layout(set = 0, binding = 0) uniform sampler2D inImage;
layout(set = 0, binding = 1) uniform sampler2D depthMap;

layout(location = 0) in vec2 uv;

void main()
{
    outputColor = texture(inImage, uv);
}