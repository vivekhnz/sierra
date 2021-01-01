#version 430 core
layout(location = 0) in vec2 in_mesh_pos;
layout(location = 1) in vec2 in_instance_pos;

uniform mat4 transform;

void main()
{
    gl_Position = (vec4(in_mesh_pos, 0.0f, 1.0f) * transform) + vec4(in_instance_pos, 0.0f, 0.0f);
}