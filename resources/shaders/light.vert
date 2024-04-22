#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec4 lightPos;
    vec4 viewPos;
    mat4 lightPV;
    vec4 lightFov;
} ubo;

layout(push_constant) uniform constants {
    mat4 model;
    mat4 normalRot;
} pushConstants;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec3 fragWorldPos;
layout(location = 2) out vec2 fragTexCoord;
layout(location = 3) out vec3 fragLightPos;
layout(location = 4) out vec3 fragViewPos;
layout(location = 5) out vec4 fragShadowCoord;

void main() {
    vec4 transformedPos = pushConstants.model * vec4(inPosition, 1.0);
    gl_Position = ubo.proj * ubo.view * transformedPos;
    fragNormal = (pushConstants.normalRot * vec4(inNormal, 1.0)).xyz;
    fragWorldPos = transformedPos.xyz;
    fragTexCoord = inTexCoord;
    fragLightPos = ubo.lightPos.xyz;
    fragViewPos = ubo.viewPos.xyz;
    vec3 normalizedNormal = normalize(inNormal);
    vec3 vecToLight = ubo.lightPos.xyz - inPosition;
    fragShadowCoord = ubo.lightPV * pushConstants.model * vec4(
        inPosition + normalizedNormal * length(vecToLight) * ubo.lightFov.x / 256.0f * 
        sqrt(1 - pow(dot(normalizedNormal, normalize(vecToLight)), 2))
        , 1.0);
    // fragShadowCoord = ubo.lightPV * transformedPos;
}