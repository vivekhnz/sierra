#version 430 core
layout(location = 0) in vec2 in_mesh_pos;
layout(location = 1) in vec2 in_mesh_uv;
layout(location = 2) in vec2 in_instance_pos;

layout (std140, binding = 0) uniform Camera
{
    mat4 camera_transform;
};
uniform float brushScale;

layout(location = 0) out vec2 out_uv;

void main()
{
    gl_Position = camera_transform * vec4((in_mesh_pos * brushScale) + in_instance_pos, 0.0f, 1.0f);
    out_uv = in_mesh_uv;
}