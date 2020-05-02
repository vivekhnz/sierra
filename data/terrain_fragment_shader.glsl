#version 410 core
layout(location = 0) in vec3 normal;
layout(location = 1) in vec2 texcoord;

uniform sampler2D terrainTexture;
uniform bool isLightingEnabled;
uniform bool isTextureEnabled;
uniform bool isNormalDisplayEnabled;

out vec4 FragColor;

void main()
{
    vec3 lightDir = vec3(0.7f, 0.3f, 0.2f);
    float ambientLight = 0.2f;
    float nDotL = dot(normal, lightDir);
    float lightingCol = isLightingEnabled ? max(nDotL, ambientLight) : 1.0f;
    vec3 texCol = isTextureEnabled ? texture(terrainTexture, texcoord).rgb : vec3(1.0f);
    vec3 normalCol = isNormalDisplayEnabled ? abs(normal) : vec3(1.0f);
    FragColor = vec4(texCol * normalCol * lightingCol, 1.0f);
}