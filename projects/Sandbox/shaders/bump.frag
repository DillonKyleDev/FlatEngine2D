#version 450

layout(push_constant, std430) uniform pc {
    vec4 light1Dir;
    vec4 light1Color;
    vec4 light2Dir;
    vec4 light2Color;
    vec4 light3Dir;
    vec4 light3Color;
    vec4 light4Dir;
    vec4 light4Color;
};

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 normal;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

void main() {

    vec4 newLightDir = vec4(-1,-1,1,1);
    outColor = texture(texSampler, fragTexCoord);
    float intensity = dot(normal, vec3(light1Dir.x, light1Dir.y, light1Dir.z));
    outColor.xyz *= clamp(intensity, .6, 1.5); 
}