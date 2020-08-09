#version 430 core
layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec2 in_uv;

uniform mat4 transform;

layout(location = 0) out vec2 out_uv;

void main()
{
    gl_Position = transform * vec4(in_pos, 1.0f);
    out_uv = in_uv;
}