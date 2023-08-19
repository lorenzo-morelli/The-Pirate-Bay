#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 2) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform GlobalUniformBufferObject {
    bool spot;
    vec3 lightPos;
    vec3 lightDir;
    vec4 lightColor;
    vec3 eyePos;
    float time;
} gubo;

layout(binding = 2) uniform sampler2D texDay;
layout(binding = 3) uniform sampler2D texNight;

void main() {
    vec2 outUV = vec2(fragUV.x + gubo.time / 500, fragUV.y);
    if (!gubo.spot) {
        outColor =
        outColor = vec4(texture(texDay, outUV).rgb, 1.0f);
    } else {
        outColor = vec4(texture(texNight, outUV).rgb, 1.0f);
    }
}