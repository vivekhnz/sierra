#version 430 core
layout(location = 0) in vec2 uv;

uniform sampler2D imageTexture;

out vec4 FragColor;

void main()
{
    FragColor = vec4(texture(imageTexture, uv).rrr, 1.0f);
}