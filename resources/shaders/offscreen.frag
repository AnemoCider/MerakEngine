#version 450

layout(binding = 0) uniform sampler2D texSampler;

layout(location = 0) in vec3 inWorldPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexcoord;

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outAlbedo;

void main() {
    outPosition = vec4(inWorldPos, 1.0);
    outNormal = vec4(inNormal, 1.0);
    outAlbedo = texture(texSampler, inTexcoord);
}