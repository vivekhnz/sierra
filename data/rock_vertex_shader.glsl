#version 430 core
layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normal;

layout(location = 0) out vec3 out_normal;

layout (std140, binding = 0) uniform Camera
{
    mat4 camera_transform;
};

void main()
{
    gl_Position = camera_transform * vec4(in_pos, 1);
    out_normal = in_normal;
}