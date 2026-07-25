#version 450
layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

layout(location = 0) out OutputData
{
    vec3 fragPos;
    vec3 cameraPos;
    vec2 uv;
}
Output;

layout(location = 0) in InputData
{
    mat4 projections[6];
    vec3 fragPos;
    vec3 cameraPos;
    vec2 uv;
}
Input[];

void main()
{
    for (int face = 0; face < 6; face++)
    {
        gl_Layer = face;
        for (int i = 0; i < 3; i++)
        {
            Output.fragPos = vec3(gl_in[i].gl_Position);
            Output.cameraPos = Input[i].cameraPos;
            Output.uv = Input[i].uv;
            gl_Position = Input[i].projections[face] * gl_in[i].gl_Position;
            EmitVertex();
        }

        EndPrimitive();
    }
}