#version 430 core
layout(location = 0) in vec2 in_pos;
layout(location = 1) in vec2 instance_offset;

uniform mat4 transform;

void main()
{
    gl_Position = (vec4(in_pos, 0.0f, 1.0f) * transform) + vec4(instance_offset, 0.0f, 0.0f);
}