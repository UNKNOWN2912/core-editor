#version 450

layout(location = 0) in InputData
{
    vec3 color;
}
Input;

layout(location = 0) out vec4 outcolor;

void main()
{
    outcolor = vec4(Input.color, 1);
}