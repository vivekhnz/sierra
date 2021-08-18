#version 430 core
layout(location = 0) in vec2 uv;

layout(binding = 0) uniform sampler2D sceneTexture;
layout(binding = 1) uniform sampler2D sceneDepthTexture;
layout(binding = 2) uniform usampler2D selectionTexture;
layout(binding = 3) uniform sampler2D selectionDepthTexture;

out vec4 FragColor;

void main()
{
    vec3 sceneColor = texture(sceneTexture, uv).rgb;
    vec3 selectionColor = vec3(1, 1, 0);
    
    float sceneDepth = texture(sceneDepthTexture, uv).r;
    float selectionDepth = texture(selectionDepthTexture, uv).r;
    
    vec4 gather0 = textureGatherOffset(selectionTexture, uv, ivec2(-2, 2));
    vec4 gather1 = textureGatherOffset(selectionTexture, uv, ivec2(0, 2));
    vec4 gather2 = textureGatherOffset(selectionTexture, uv, ivec2(-2, 0));
    vec4 gather3 = textureGatherOffset(selectionTexture, uv, ivec2(0, 0));

    gather2.xw = gather1.zw;
    vec4 centerSelVal = vec4(gather3.x);
    gather3.x = gather0.z;
    
    vec4 one = vec4(1);
    float blend = 0.25f * (
        dot(vec4(notEqual(gather2, centerSelVal)), one) +
        dot(vec4(notEqual(gather3, centerSelVal)), one)
    );
    if (sceneDepth < selectionDepth)
    {
        blend *= 0.4;
    }
    if ((uvec4(gather3) & 0x80) != 0 || (uvec4(gather2) & 0x80) != 0)
    {
        selectionColor = vec3(1, 1, 1);
    }
    vec3 outColor = mix(sceneColor, selectionColor, blend);
    
    FragColor = vec4(outColor, 1);
}