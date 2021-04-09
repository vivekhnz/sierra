#version 430 core
layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in mat4 in_instance_transform;

layout(location = 0) out vec3 out_normal;

layout (std140, binding = 0) uniform Camera
{
    mat4 camera_transform;
};

void main()
{
    gl_Position = camera_transform * in_instance_transform * vec4(in_pos, 1);
    mat4 inverseTransposeTransform = transpose(inverse(in_instance_transform));
    out_normal = normalize((inverseTransposeTransform * vec4(in_normal, 0)).xyz);
}