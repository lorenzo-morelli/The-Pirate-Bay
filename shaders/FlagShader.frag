#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
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

const float beta = 2.0f;
const float g = 1.5;
const float cosout = 0.85;
const float cosin  = 0.85;

void main() {
    vec3 norm = normalize(fragNorm);
    vec3 eyeDir = normalize(gubo.eyePos.xyz - fragPos);
    float t = gubo.time;
    vec3 lightTest = normalize(vec3(1.0f, 1.0f, 1.0f));
    vec3 darkColor = vec3(0.0f, 0.0f, 0.0f)/255.0f;
    vec3 highColor = vec3(80.0f, 50.0f, 50.0f)/255.f;
    vec3 diffuse = texture(texFlag, fragUV).rgb; // texture
    vec3 ambient = highColor * 0.025f;

    if (gubo.spot) {
        vec3 lightDir = gubo.lightPos - fragPos;

        float dim = clamp(( dot(normalize(lightDir),gubo.lightDir) - cosout)/(cosin - cosout), 0.0f, 1.0f);
        float fadeOut = pow(g/length(lightDir), beta);
        vec3 lightColor = gubo.lightColor.rgb * fadeOut * dim;
        
        vec3 specular = vec3(pow(clamp(dot(norm, normalize(lightDir + eyeDir)), 0.0f, 1.0f), 160.0f));
        vec3 ambient = darkColor * 0.025f;

        outColor = vec4(clamp((diffuse + specular) * lightColor.rgb + ambient, 0.0f, 1.0f), 1.0f);
    }
    else outColor = vec4(diffuse + ambient, 1.0f);

}