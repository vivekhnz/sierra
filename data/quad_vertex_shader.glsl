#version 430 core
layout(location = 0) in vec2 in_mesh_pos;
layout(location = 1) in vec2 in_mesh_uv;
layout(location = 2) in vec4 in_instance_rect;

layout (std140, binding = 0) uniform Camera
{
    mat4 camera_transform;
};

layout(location = 0) out vec2 out_uv;

void main()
{
    vec2 pos = in_instance_rect.xy + (in_instance_rect.zw * in_mesh_pos);
    gl_Position = camera_transform * vec4(pos, 0, 1);
    out_uv = in_mesh_uv;
}