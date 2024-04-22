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

//void main() {
//    vec3 fragToLight = lightWorldPos - worldPos;
//    float distanceSquare = dot(fragToLight, fragToLight);
//    vec3 lightDir = normalize(fragToLight);
//    vec3 halfDir = normalize(lightDir + normalize(viewPos - worldPos));
//     outColor = vec4(texture(texSampler, texCoord).xyz *
//        clamp((
//            max(dot(normalize(normal), lightDir), 0.0) * vec3(1.0f) +
//            pow(max(dot(halfDir, normal), 0.0), 15) * vec3(1.0f) * 10.0
//            ) + 0.5, 0.0, 1.0), 1.0)
//        * getShadow(shadowCoord);
//    // outColor = vec4(0.5);
//    // outColor = vec4(texture(texSampler, texCoord).xyz, 1.0);
//    // outColor = vec4(texture(texSampler, texCoord).xyz, 1.0) * (getShadow(shadowCoord));
//    /*vec3 projCoord = shadowCoord.xyz / shadowCoord.w;
//    projCoord.xy = projCoord.xy * 0.5 + 0.5;
//    outColor = vec4(vec3(getShadowDiff(shadowCoord)), 1.0);*/
//}

const float lightRadius = 5.0f;

vec2 spiralPattern(int k, float radius, int samples) {
    const float goldenAngle = 2.39996323; // Use golden angle for even distribution
    float angle = goldenAngle * float(k);
    float distance = mix(0.0, radius, sqrt(float(k) / float(samples - 1))); // Distribute samples within a radius
    return vec2(cos(angle), sin(angle)) * distance;
}

vec2 hammersley2d(uint i, uint N) {
    // Radical inverse based on http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
    uint bits = (i << 16u) | (i >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    float rdi = float(bits) * 2.3283064365386963e-10;
    return vec2(float(i) / float(N), rdi);
}

// return -1 if fully lit
float getAvgBlockerDepth(vec3 coord, float lightDis, float searchRadius) {
    // the further the point is away from the light,
    // the larger the area it casts on the shadowMap
    int count = 0;
    float sum = 0;
    // Spiral search pattern
    int numSamples = 20;
    for (int k = 0; k < numSamples; ++k) {
        vec2 samplePoint = hammersley2d(k, numSamples) - vec2(0.5f, 0.5f);
        vec2 offset = searchRadius * samplePoint; // Generate offset for sample k
        float sampledDepth = texture(gBufferSamplers[3], coord.xy + offset).r;
        if (sampledDepth < coord.z) {
            count++;
            sum += sampledDepth;
        }
    }
    if (count == 0) {
        return -1.0;
    } else {
        return sum / float(count);
    }
}


float getShadowPCSS(vec4 shadowCoord, float lightDis, int numSamples) {
    vec3 projCoord = shadowCoord.xyz / shadowCoord.w;
    projCoord.xy = projCoord.xy * 0.5 + 0.5;
    float radius = lightRadius * lightDis / 5000;
    float blockerDepth = getAvgBlockerDepth(projCoord, lightDis, radius);
    if (blockerDepth < 0) {
        return 1.0;
    } else {
        int count = 0;
        int sum = 0;
        float kernelSize = max((projCoord.z - blockerDepth) * lightRadius / 100.0 / blockerDepth, 0.0);
        /* for (float i = -kernelSize; i <= kernelSize; i += unit) {
             for (float j = -kernelSize; j <= kernelSize; j += unit) {
                 count++;
                 if (projCoord.z <= texture(shadowSampler, projCoord.xy + vec2(i, j)).r) {
                     sum++;
                }
             }
         }*/
        if (kernelSize < 0.0001) return 1.0;
        for (int i = 0; i < numSamples; i++) {
            vec2 samplePoint = hammersley2d(i, numSamples) - vec2(0.5f, 0.5f);
            if (projCoord.z <= texture(gBufferSamplers[3], projCoord.xy + kernelSize * samplePoint).r) {
                sum++;
            }
        }

        return float(sum) / numSamples;
    }
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
        * clamp(getShadowPCSS(shadowCoord, length(fragToLight), 40), 0.1, 1.0);
}

