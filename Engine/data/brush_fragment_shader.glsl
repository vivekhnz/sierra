#version 430 core
out vec4 FragColor;

void main()
{
    vec2 resolution = vec2(400.0f, 400.0f);
    vec2 uv = gl_FragCoord.xy / resolution;
    
    float innerRadius = 0.05f;
    float outerRadius = 0.15f;

    float value = distance(uv, vec2(0.5f)) * 2.0f;
    value = (-value + outerRadius) / (outerRadius - innerRadius);
    value = max(min(value, 1.0f), 0.0f);
    value = value * 0.5f;
    
    FragColor = vec4(1.0f, 1.0f, 1.0f, value);
}