#version 430 core
layout(location = 0) in vec2 uv;

layout(binding = 0) uniform usampler2D idTexture;

out vec4 FragColor;

// based on: http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl
vec3 hueToRgb(float hue)
{
    vec4 K = vec4(1, 2 / 3.0f, 1 / 3.0f, 3);
    vec3 p = abs((fract(hue + K.xyz) * 6) - 3);
    return clamp(p - 1, 0, 1);
}

void main()
{
    uint id = texture(idTexture, uv).r;
    
    // mod operates on signed integers so will clamp any IDs above 2^31
    // we need to mask out the most significant bit so that we can still distinguish between
    // objects with IDs above 2^31
    id &= ((1 << 31) - 1);

    vec3 outColor = vec3(0);
    if (id > 0)
    {
        float hue = mod(id, 10) * 0.1f;
        outColor = hueToRgb(hue);
    }
    
    FragColor = vec4(outColor, 1);
}