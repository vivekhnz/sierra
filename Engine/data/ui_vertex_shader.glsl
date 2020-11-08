#version 430 core
layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec2 in_uv;

uniform mat4 instance_transform;

void main()
{
    gl_Position = instance_transform * vec4(in_pos, 1.0f);
}