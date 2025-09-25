#version 450



layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in float disturbance;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

void main() {
    vec2 textureDimensions = textureSize(texSampler, 0);
    float delta = 1 / textureDimensions.x;

    float translateX = fragTexCoord.x + 0.5; // shift to center
    float translateY = fragTexCoord.y + 0.5;

    // Convert to polar
    float radius = sqrt((translateX * translateX) + (translateY * translateY));
    float theta = atan(translateX / translateY);
    float adjacentRadius = radius + delta + sin(fragTexCoord.x + sin(disturbance));

    // Convert back to cartesian
    float adjacentX = adjacentRadius * cos(theta);
    float adjacentY = adjacentRadius * sin(theta);

    vec2 newTexCoord = vec2(adjacentX, adjacentY);
    vec4 adjacentTexColor = texture(texSampler, newTexCoord);    

    outColor = adjacentTexColor;
}