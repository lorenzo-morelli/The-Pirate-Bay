#version 450#extension GL_ARB_separate_shader_objects : enablelayout(location = 0) in vec3 fragPos;layout(location = 1) in vec3 fragNorm;layout(location = 0) out vec4 outColor;layout(set = 0, binding = 1) uniform GlobalUniformBufferObject {    vec3 lightDir;    vec3 lightColor;    vec3 eyePos;    float time;} gubo;vec3 toonDiffuse(vec3 L, vec3 N, vec3 V, vec3 light, vec3 dark) {    float cosAngle = dot(L, N);    vec3 color;    if (cosAngle >= 0.9f)    color = dark;    else    color = mix(light, dark, cosAngle);    return color;}vec3 toonSpecular(vec3 L, vec3 N, vec3 V, vec3 C, float thr)  {    float intensity = dot(V, -reflect(L, N));    vec3 color;    if (intensity >= thr) color = C;    else color = vec3(0, 0, 0);    return color;}void main() {    vec3 norm = normalize(fragNorm);    vec3 eyeDir = normalize(gubo.eyePos.xyz - fragPos);    float t = gubo.time;    float freq = 1.0f;    vec3 lightTest = normalize(vec3(cos(freq * t), sin(freq * t), 0.0f));    float altitudeCoeff = fragPos.y/(3.0f*2.0f);    float windEffect = sin(fragPos.x*fragPos.z*0.01f + t/10.0f);    vec3 yellowShades = vec3(altitudeCoeff*altitudeCoeff*windEffect*windEffect,                                altitudeCoeff*altitudeCoeff*windEffect*windEffect, 0.0f); //yellow shades and windEffect    vec3 darkColor = vec3(80.0f, 130.0f, 80.0f)/255.0f + yellowShades;//dark color    vec3 highColor = vec3(150.0f, 255.0f, 150.0f)/255.f + yellowShades;    vec3 diffuse = toonDiffuse(lightTest, norm, eyeDir, darkColor, highColor);    //  vec3 specular = toonSpecular(gubo.lightDir, norm, eyeDir, gubo.lightColor, 0.5);    vec3 ambient = highColor * 0.025f;    outColor = vec4(diffuse + ambient, 1.0f);}