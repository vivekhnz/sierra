#version 430 core
layout(location = 0) in vec2 uv;

layout(binding = 0) uniform sampler2D sceneTexture;
layout(binding = 1) uniform sampler2D selectionTexture;

out vec4 FragColor;

void main()
{
    vec3 sceneColor = texture(sceneTexture, uv).rgb;
    vec3 selectionColor = vec3(1, 1, 0);
    
    vec4 gather0 = ceil(textureGatherOffset(selectionTexture, uv, ivec2(-2, -2)));
    vec4 gather1 = ceil(textureGatherOffset(selectionTexture, uv, ivec2(0, -2)));
    vec4 gather2 = ceil(textureGatherOffset(selectionTexture, uv, ivec2(-2, 0)));
    vec4 gather3 = ceil(textureGatherOffset(selectionTexture, uv, ivec2(0, 0)));
    
    gather2.xw = gather1.xy;
    float centerSelVal = gather3.w;
    gather3.w = gather0.y;

    float avgSelVal = 0.125f * (
        gather2.x + gather2.y + gather2.z + gather2.w +
        gather3.x + gather3.y + gather3.z + gather3.w
    );
    float blend = dot(avgSelVal, 1 - centerSelVal);
    vec3 outColor = mix(sceneColor, selectionColor, blend);
    
    FragColor = vec4(outColor, 1);
}