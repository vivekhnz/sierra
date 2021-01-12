#version 430 core
layout(location = 0) in vec3 vertexNormal;
layout(location = 1) in vec3 texcoord;

layout (std140, binding = 1) uniform Lighting
{
    vec4 lighting_lightDir;
    bool lighting_isEnabled;
    bool lighting_isTextureEnabled;
    bool lighting_isNormalMapEnabled;
    bool lighting_isAOMapEnabled;
    bool lighting_isDisplacementMapEnabled;
};

uniform sampler2D mat1_albedo;
uniform sampler2D mat1_normal;
uniform sampler2D mat1_ao;
uniform sampler2D mat2_albedo;
uniform sampler2D mat2_normal;
uniform sampler2D mat2_ao;
uniform vec3 terrainDimensions;
uniform vec2 mat1_textureSizeInWorldUnits;
uniform vec2 mat2_textureSizeInWorldUnits;
uniform vec2 brushHighlightPos;
uniform float brushHighlightStrength;
uniform float brushHighlightRadius;
uniform float brushHighlightFalloff;

out vec4 FragColor;

vec3 calcBrushHighlight()
{
    float highlightRadius = brushHighlightRadius * 0.5f;
    vec2 normalizedUV = texcoord.xz / terrainDimensions.xz;
    float distFromHighlight = distance(normalizedUV, brushHighlightPos);
    
    // outline thickness is based on depth
    float outlineWidth = max(min(0.003 - (0.07 * gl_FragCoord.w), 0.003), 0.0005);
    float outlineIntensity =
        max(1 - abs((((distFromHighlight - highlightRadius) / outlineWidth) * 2) - 1), 0);
    float influenceAreaIntensity = (
        (distFromHighlight / highlightRadius) - brushHighlightFalloff)
        / (1 - brushHighlightFalloff);
    influenceAreaIntensity = 1 - clamp(influenceAreaIntensity, 0, 1);

    float highlightIntensity = influenceAreaIntensity + outlineIntensity;
    return vec3(0.0f, 1.0f, 0.25f) * highlightIntensity;
}

vec3 calcTriplanarBlend(vec3 normal)
{
    // bias towards Y-axis
    vec3 blend = vec3(pow(abs(normal.x), 6), pow(abs(normal.y), 1), pow(abs(normal.z), 6));
    blend = normalize(max(blend, 0.00001));
    blend /= blend.x + blend.y + blend.z;
    return blend;
}

float triplanar1D(float xVal, float yVal, float zVal, vec3 blend)
{
    return (xVal * blend.x) + (yVal * blend.y) + (zVal * blend.z);
}

vec3 triplanar3D(vec3 xVal, vec3 yVal, vec3 zVal, vec3 blend)
{
    return (xVal * blend.x) + (yVal * blend.y) + (zVal * blend.z);
}

void main()
{
    float slope = vertexNormal.y;

    vec3 triBlend = calcTriplanarBlend(vertexNormal);
    vec3 triAxisSign = sign(vertexNormal);
    
    vec2 base_texcoord_x = vec2(texcoord.z * triAxisSign.x, texcoord.y);
    vec2 base_texcoord_y = vec2(texcoord.x * triAxisSign.y, texcoord.z);
    vec2 base_texcoord_z = vec2(texcoord.x * triAxisSign.z, texcoord.y);

    vec2 mat1_texcoord_x = base_texcoord_x / mat1_textureSizeInWorldUnits.yy;
    vec2 mat1_texcoord_y = base_texcoord_y / mat1_textureSizeInWorldUnits.xy;
    vec2 mat1_texcoord_z = base_texcoord_z / mat1_textureSizeInWorldUnits.xy;

    vec2 mat2_texcoord_x = base_texcoord_x / mat2_textureSizeInWorldUnits.yy;
    vec2 mat2_texcoord_y = base_texcoord_y / mat2_textureSizeInWorldUnits.xy;
    vec2 mat2_texcoord_z = base_texcoord_z / mat2_textureSizeInWorldUnits.xy;

    // sample normal map
    vec3 texNormal = vec3(0);
    if (lighting_isNormalMapEnabled)
    {
        // flip normals to correct for the flipped UVs
        vec3 texNormal_mat1 = triplanar3D(
            ((texture(mat1_normal, mat1_texcoord_x).rgb * 2) - 1) * vec3(triAxisSign.x, 1, 1),
            ((texture(mat1_normal, mat1_texcoord_y).rgb * 2) - 1) * vec3(triAxisSign.y, 1, 1),
            ((texture(mat1_normal, mat1_texcoord_z).rgb * 2) - 1) * vec3(triAxisSign.z, 1, 1),
            triBlend);
        vec3 texNormal_mat2 = triplanar3D(
            ((texture(mat2_normal, mat2_texcoord_x).rgb * 2) - 1) * vec3(triAxisSign.x, 1, 1),
            ((texture(mat2_normal, mat2_texcoord_y).rgb * 2) - 1) * vec3(triAxisSign.y, 1, 1),
            ((texture(mat2_normal, mat2_texcoord_z).rgb * 2) - 1) * vec3(triAxisSign.z, 1, 1),
            triBlend);
        texNormal = mix(texNormal_mat2, texNormal_mat1, slope);
    }
    vec3 normal = normalize(vertexNormal - (texNormal * 0.5f));

    // sample albedo texture
    vec3 albedo = vec3(1);
    if (lighting_isTextureEnabled)
    {
        vec3 albedo_mat1 = triplanar3D(
            texture(mat1_albedo, mat1_texcoord_x).rgb,
            texture(mat1_albedo, mat1_texcoord_y).rgb,
            texture(mat1_albedo, mat1_texcoord_z).rgb,
            triBlend);
        vec3 albedo_mat2 = triplanar3D(
            texture(mat2_albedo, mat2_texcoord_x).rgb,
            texture(mat2_albedo, mat2_texcoord_y).rgb,
            texture(mat2_albedo, mat2_texcoord_z).rgb,
            triBlend);
        albedo = mix(albedo_mat2, albedo_mat1, slope);
    }

    // calculate lighting
    float ambientLight = 0.1f;
    float nDotL = dot(normal, lighting_lightDir.xyz);
    float lightingAmplitude = lighting_isEnabled
        ? ambientLight + pow(0.5 + (nDotL * 0.5), 2)
        : 1.0f;

    // sample ambient occlusion texture
    float ao = 1;
    if (lighting_isAOMapEnabled)
    {
        float ao_mat1 = triplanar1D(
            texture(mat1_ao, mat1_texcoord_x).r,
            texture(mat1_ao, mat1_texcoord_y).r,
            texture(mat1_ao, mat1_texcoord_z).r,
            triBlend);
        float ao_mat2 = triplanar1D(
            texture(mat2_ao, mat2_texcoord_x).r,
            texture(mat2_ao, mat2_texcoord_y).r,
            texture(mat2_ao, mat2_texcoord_z).r,
            triBlend);
        ao = mix(0.6, 1.0, mix(ao_mat2, ao_mat1, slope));
    }

    // calculate final fragment color
    vec3 terrainColor = albedo * lightingAmplitude * ao;
    vec3 brushHighlight = calcBrushHighlight();
    FragColor = vec4(terrainColor + (brushHighlight * brushHighlightStrength), 1.0f);
}