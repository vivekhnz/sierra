#version 410 core

uniform vec3 lowColor;
uniform vec3 highColor;

in float height;

out vec4 FragColor;

void main()
{
    FragColor = vec4(mix(lowColor, highColor, height), 1.0f);
}