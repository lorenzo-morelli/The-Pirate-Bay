#version 450
#extension GL_ARB_separate_shader_objects : enable
layout(location = 0) in vec3 fragPos;

layout(location = 1) in vec3 fragNorm;


layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform GlobalUniformBufferObject {
    bool spot;
    vec3 lightPos;
    vec3 lightDir;
    vec4 lightColor;
    vec3 eyePos;
    float time;
} gubo;

void main() {
    vec3 norm = normalize(fragNorm); // n
    vec3 eyeDir = normalize(gubo.eyePos.xyz - fragPos); // wr
    vec3 lightTest = normalize(vec3(1.0f, 1.0f, 1.0f));

    vec3 r = -reflect(lightTest, norm);
    vec3 specular = clamp(eyeDir * r, 0.0f, 1.0f);
    vec3 diffuse = r;
    //float altitudeCoeff = fragPos.y/(3.0f*2.0f);
    vec3 darkColor = vec3(80.0f, 130.0f, 80.0f)/255.0f;
    vec3 highColor = vec3(150.0f, 255.0f, 150.0f)/255.f;
    //  vec3 specular = toonSpecular(gubo.lightDir, norm, eyeDir, gubo.lightColor, 0.5);
    vec3 ambient = highColor * 0.025f;
    outColor = vec4(diffuse + specular + ambient, 1.0f);
}

