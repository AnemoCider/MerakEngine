#version 450

layout(push_constant) uniform constants {
    mat4 model;
    mat4 lightVP;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexcoord;

layout(location = 0) out vec3 outWorldPos;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec2 outTexcoord;

void main() {
    vec4 worldPos = ubo.model * vec4(inPosition, 1.0);

    gl_Position = ubo.lightVP * worldPos;

    outWorldPos = worldPos.xyz;
    outNormal = transpose(inverse(mat3(ubo.model))) * normalize(inNormal);
    outTexcoord = inTexcoord;
}