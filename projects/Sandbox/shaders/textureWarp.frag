#version 450



layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in float disturbance;
layout(location = 2) in vec4 baseColor;
layout(location = 3) in vec4 secondaryColor;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D colorSampler;
layout(binding = 2) uniform sampler2D texSampler;
layout(binding = 3) uniform sampler2D texSampler2;

float clamp(float toClamp) {
    if  (toClamp < 0.5)
    {
        return 0;
    }
    else
    {
        return 1;        
    }  
}

void main() {
    vec2 textureDimensions = textureSize(colorSampler, 0);
    float delta = 1 / textureDimensions.x;
    vec4 fragColor = texture(colorSampler, fragTexCoord);

    vec2 movingTexCoord = fragTexCoord;
    movingTexCoord *= .2;
    movingTexCoord.y += disturbance;
    vec4 texNoise = texture(texSampler, movingTexCoord);

    vec4 smallBallColor = texture(texSampler2, fragTexCoord);

    outColor = baseColor * texNoise.x;
    outColor += smallBallColor;
    
    if (outColor.w < 0.5)
    {
        outColor = vec4(0);
    }
    else
    {
        outColor = secondaryColor;
        outColor.w = 1;
    }
}