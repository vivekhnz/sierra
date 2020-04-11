#version 410 core

in vec3 normal;

out vec4 FragColor;

void main()
{
    FragColor = vec4((normal.xyz * 0.5f) + 0.5f, 1.0f);
}