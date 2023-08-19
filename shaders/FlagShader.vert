#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 mvpMat;
    mat4 mMat;
    mat4 nMat;
} ubo;

layout(set = 0, binding = 1) uniform GlobalUniformBufferObject {
    bool spot;
    vec3 lightPos;
    vec3 lightDir;
    vec4 lightColor;
    vec3 eyePos;
    float time;
} gubo;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec2 UV;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNorm;
layout(location = 2) out vec2 fragUV;

void main() {
    float size = 0.75f;
    float x = pos.x*size;
    float y = pos.y*size;
    float z = pos.z*size;
    float t = gubo.time;

    vec3 vpos = vec3(x, -z, y + sin(x*0.5f + t*10.f))*0.1f + vec3(7.5f,5.0f,7.5f);

    gl_Position = ubo.mvpMat * vec4(vpos, 1.0);
    fragPos = (ubo.mMat * vec4(vpos, 1.0)).xyz;
    fragNorm = norm;
    fragUV = UV;
}