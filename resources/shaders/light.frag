#version 450

layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 2) uniform sampler2D gBufferSamplers[4];

layout(location = 0) in vec3 normal;
layout(location = 1) in vec3 worldPos;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec3 lightWorldPos;
layout(location = 4) in vec3 viewPos;
layout(location = 5) in vec4 shadowCoord;

layout(location = 0) out vec4 outColor;

float getShadow(vec4 shadowCoord) {
    vec3 projCoord = shadowCoord.xyz / shadowCoord.w;
    projCoord.xy = projCoord.xy * 0.5 + 0.5;
    return (projCoord.z > (texture(gBufferSamplers[3], projCoord.xy).r)) ? 0.1 : 1.0;
    /*return texture(shadowSampler, projCoord.xy).r;*/
    // return projCoord.x;
}

// for debug
float getShadowDiff(vec4 shadowCoord) {
    vec3 projCoord = shadowCoord.xyz / shadowCoord.w;
    projCoord.xy = projCoord.xy * 0.5 + 0.5;
    return clamp((projCoord.z - (texture(gBufferSamplers[3], projCoord.xy).r)) / 10.0f, 0.0f, 1.0f);
}

void main() {
    vec3 fragToLight = lightWorldPos - worldPos;
    float distanceSquare = dot(fragToLight, fragToLight);
    vec3 lightDir = normalize(fragToLight);
    vec3 halfDir = normalize(lightDir + normalize(viewPos - worldPos));
    outColor = vec4(texture(texSampler, texCoord).xyz *
        clamp((
            max(dot(normalize(normal), lightDir), 0.0) * vec3(1.0f) +
            pow(max(dot(halfDir, normal), 0.0), 15) * vec3(1.0f) * 10.0
            ) + 0.05, 0.0, 1.0), 1.0)
        * clamp(getShadow(shadowCoord), 0.05, 1.0);
}

