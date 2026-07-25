#version 450

#extension GL_EXT_nonuniform_qualifier : require

layout(set = 0, binding = 0) uniform sampler2D textures[];

layout(location = 0) out vec4 outcolor;

layout(location = 0) in InputData
{
    vec2 uv;
    flat uint charaterId;
    vec4 color;
}
indata;

void main()
{
    float a = texture(textures[indata.charaterId], indata.uv).r;
    outcolor = vec4(indata.color);
    outcolor.a = a;
    if (a < 0.01)
        discard;
}