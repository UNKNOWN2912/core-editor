#version 450

layout(location = 0) out vec2 uv;

vec2 positions[] = vec2[](
    vec2(1, 1),
    vec2(1, -1),
    vec2(-1, -1),
    vec2(-1, -1),
    vec2(-1, 1),
    vec2(1, 1));

void main()
{
    uv = (positions[gl_VertexIndex] + vec2(1)) / vec2(2);
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
}