#version 450

layout(set = 0, binding = 0) uniform UniformBufferObject {
    vec4 meshPosition;
    vec4 cameraPosition;
    mat4 model;
    mat4 viewAndProjection;        
    vec4 vec4s[32];
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out float disturbance;
layout(location = 2) out vec4 baseColor;
layout(location = 3) out vec4 secondaryColor;


void main() {    
    gl_Position = ubo.viewAndProjection * (ubo.model * vec4(inPosition.x, inPosition.y, inPosition.z, 1) + ubo.meshPosition);        
    fragTexCoord = inTexCoord;
    baseColor = ubo.vec4s[0];
    disturbance = ubo.vec4s[1].x;
    secondaryColor = ubo.vec4s[2];
}