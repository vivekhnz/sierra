#version 430 core
layout(location = 0) in vec2 in_pos;
layout(location = 1) in vec2 in_uv;

layout (std140, binding = 0) uniform Camera
{
    mat4 camera_transform;
};
uniform mat4 instance_transform;

layout(location = 0) out vec2 out_uv;

void main()
{
    gl_Position = camera_transform * instance_transform * vec4(in_pos, 0.0f, 1.0f);
    out_uv = in_uv;
}