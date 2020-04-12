#version 410 core
in vec3 normal;
in vec2 texcoord;

uniform sampler2D terrainTexture;

out vec4 FragColor;

void main()
{
    vec3 lightDir = vec3(0.7f, 0.3f, 0.2f);
    float ambientLight = 0.2f;
    float nDotL = dot(normal, lightDir);
    vec3 texCol = texture(terrainTexture, texcoord).rgb;
    FragColor = vec4(texCol * abs(normal) * max(nDotL, ambientLight), 1.0f);
}