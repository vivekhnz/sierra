#version 410 core
layout (location = 0) in vec3 pos;

uniform mat4 transform;
uniform float maxHeight;

out float height;

void main()
{
    gl_Position = transform * vec4(pos, 1.0f);
    height = pos.y / maxHeight;
}