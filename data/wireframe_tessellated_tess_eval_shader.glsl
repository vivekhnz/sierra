#version 410 core
layout(triangles, equal_spacing, ccw) in;

in vec3 worldPos[];

uniform mat4 transform;

vec3 lerp3D(vec3 a, vec3 b, vec3 c)
{
    return (a * gl_TessCoord.x) + (b * gl_TessCoord.y) + (c * gl_TessCoord.z);
}

void main()
{
    vec3 pos = lerp3D(worldPos[0], worldPos[1], worldPos[2]);
    gl_Position = transform * vec4(pos, 1.0f);
}