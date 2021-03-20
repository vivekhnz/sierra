#version 430 core
layout(location = 0) in vec2 in_pos;
layout(location = 1) in vec2 in_uv;

layout (std140, binding = 0) uniform Camera
{
    mat4 camera_transform;
};

layout(location = 0) out vec2 out_uv;

void main()
{
    gl_Position = camera_transform * vec4(in_pos, 0, 1);
    out_uv = in_uv;
}