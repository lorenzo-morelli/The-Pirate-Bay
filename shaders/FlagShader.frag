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

layout(binding = 2) uniform sampler2D texFlag;

void main() {
    outColor = vec4(texture(texFlag, fragUV).rgb, 1.0f);
}