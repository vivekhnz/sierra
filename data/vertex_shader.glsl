#version 410 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 uv;

uniform mat4 transform;
uniform vec2 unitSize;
uniform sampler2D heightmapTexture;

out vec3 normal;
out vec2 texcoord;

void main()
{
    gl_Position = transform * vec4(pos, 1.0f);

    vec2 hUV = vec2((pos.x * unitSize.x) + 0.5f, (pos.z * unitSize.y) + 0.5f);
    float hL = texture(heightmapTexture, vec2(hUV.x - unitSize.x, hUV.y)).x;
    float hR = texture(heightmapTexture, vec2(hUV.x + unitSize.x, hUV.y)).x;
    float hD = texture(heightmapTexture, vec2(hUV.x, hUV.y - unitSize.y)).x;
    float hU = texture(heightmapTexture, vec2(hUV.x, hUV.y + unitSize.y)).x;
    normal = normalize(vec3(hR - hL, unitSize.x, hD - hU));
    texcoord = uv;
}