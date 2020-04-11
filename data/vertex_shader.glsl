#version 410 core
layout (location = 0) in vec3 pos;

uniform mat4 transform;
uniform vec2 unitSize;
uniform sampler2D heightmapTexture;

out vec3 normal;

void main()
{
    gl_Position = transform * vec4(pos, 1.0f);

    vec2 texcoord = vec2((pos.x * unitSize.x) + 0.5f, (pos.z * unitSize.y) + 0.5f);
    float hL = texture(heightmapTexture, vec2(texcoord.x - unitSize.x, texcoord.y)).x;
    float hR = texture(heightmapTexture, vec2(texcoord.x + unitSize.x, texcoord.y)).x;
    float hD = texture(heightmapTexture, vec2(texcoord.x, texcoord.y - unitSize.y)).x;
    float hU = texture(heightmapTexture, vec2(texcoord.x, texcoord.y + unitSize.y)).x;
    normal = vec3(hR - hL, hD - hU, 0.0f);
}