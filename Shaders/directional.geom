#version 450

layout(triangles) in;
layout(triangle_strip, max_vertices = 12) out;

layout(location = 0) in InputData
{
    mat4 projections[6];
    vec2 uv;
}
Input[];

layout(location = 0) out OutputData
{
    vec2 uv;
}
Output;

const int cascadeCount = 4;

void main()
{
    for (int cascade = 0; cascade < cascadeCount; cascade++)
    {
        gl_Layer = cascade;
        for (int i = 0; i < 3; i++)
        {
            Output.uv = Input[i].uv;
            gl_Position = Input[i].projections[cascade] * gl_in[i].gl_Position;
            EmitVertex();
        }

        EndPrimitive();
    }
}