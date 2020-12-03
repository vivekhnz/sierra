#version 430 core
layout(location = 0) in vec2 in_pos;

uniform mat4 instance_transform;

void main()
{
    gl_Position = instance_transform * vec4(in_pos, 0.0f, 1.0f);
}