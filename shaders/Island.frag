#version 450#extension GL_ARB_separate_shader_objects : enablelayout(location = 0) in vec3 fragPos;layout(location = 1) in vec3 fragNorm;layout(location = 0) out vec4 outColor;layout(set = 0, binding = 1) uniform GlobalUniformBufferObject {    bool spot;    vec3 lightPos;    vec3 lightDir;    vec4 lightColor;    vec3 eyePos;    float time;} gubo;vec3 toonDiffuse(vec3 L, vec3 N, vec3 dark, vec3 light) {    float dotL = dot(L, N);    vec3 color;    if (dotL >= 0.9f) color = light;    else color = mix(dark, light, dotL);    return color;}const float beta = 2.0f;const float g = 1.5;const float cosout = 0.85;const float cosin  = 0.85;void main() {    vec3 norm = normalize(fragNorm);    vec3 eyeDir = normalize(gubo.eyePos - fragPos);    float normalizedAlt = int((fragPos.y)/(2.0f)*10.0f)/10.0f;    vec3 brightColor;    if (normalizedAlt>=(0.6f)) brightColor = vec3(100.0f, 255.0f, 100.0f)/255.f;    else if (normalizedAlt>=(0.35f)) brightColor = vec3(50.0f, 225.0f, 50.0f)/255.f;    else if (normalizedAlt>=(0.25f)) brightColor = vec3(40.0f, 150.0f, 40.0f)/255.f;    else if (normalizedAlt>=(0.2f)) brightColor = vec3(200.0f, 100.0f, 0.0f)/255.f;    else brightColor = vec3(255.0f, 255.0f, 0.0f)/255.f;    vec3 darkColor = brightColor*0.25f;    if (!gubo.spot) {        vec3 diffuse = toonDiffuse(gubo.lightDir, norm, darkColor, brightColor);        vec3 ambient = gubo.lightColor.xyz * 0.25f;        outColor = vec4(diffuse + ambient, 1.0f);    } else {        vec3 lightDir = gubo.lightPos - fragPos;        float dim = clamp((dot(normalize(lightDir), gubo.lightDir) - cosout)/(cosin - cosout), 0.0f, 1.0f);        float fadeOut = pow(g/length(lightDir), beta);        vec3 lightColor = gubo.lightColor.rgb * fadeOut * dim;        vec3 diffuse = toonDiffuse(lightDir, norm, darkColor, brightColor);        vec3 specular = vec3(pow(clamp(dot(norm, normalize(lightDir + eyeDir)), 0.0f, 1.0f), 160.0f));        outColor = vec4(clamp((diffuse + specular) * lightColor.rgb, 0.0f, 1.0f), 1.0f);    }}