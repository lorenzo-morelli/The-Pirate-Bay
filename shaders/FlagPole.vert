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

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNorm;

void main() {
    float x = pos.x;
    float y = pos.y;
    float z = pos.z;
    float t = gubo.time;


    vec3 vpos = vec3(x*0.1f, y*5.75f, z*0.1f) + vec3(7.5f-(0.75f*2),0.0f,7.5f+0.75f);
    gl_Position = ubo.mvpMat * vec4(vpos, 1.0);
    fragPos = (ubo.mMat * vec4(vpos, 1.0)).xyz;
    fragNorm = norm;
}