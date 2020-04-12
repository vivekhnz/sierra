#version 410 core
in vec3 normal;

out vec4 FragColor;

void main()
{
    vec3 lightDir = vec3(0.7f, 0.3f, 0.2f);
    float ambientLight = 0.2f;
    float nDotL = dot(normal, lightDir);
    FragColor = vec4(abs(normal) * max(nDotL, ambientLight), 1.0f);
}