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
        outColor = vec4(texture(texDay, outUV).rgb, 1.0f);
    } else {
        vec3 gradient = vec3(0.0f, 0.0f, (int(fragUV.y*20.0f)/20.0f)*0.05f);
        float resU = int(fragUV.x*500.0f)/500.0f;
        float resV = int(fragUV.y*500.0f)/500.0f;
        float random = fract(sin(resU*resV + resU+resV)*43758.5453)+sin(0.001f*gubo.time);
        float random2 = fract(sin(resU/resV - resU-resV)*43759.5453);
        float flickering = sin(pow(random2*10.0f, 40.0f)+gubo.time);
        vec3 star = gradient;

        if (random>0.99f)star=gradient + vec3(clamp(flickering, 0.0f, 1.0f));
        if (fragUV.y<0.05f)star=gradient;//to hide stars at polar distorsion
        outColor = vec4(star, 1.0f);
    }
}