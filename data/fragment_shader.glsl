#version 410 core
in vec3 normal;

out vec4 FragColor;

void main()
{
    FragColor = vec4(abs(normal.xyz), 1.0f);
}