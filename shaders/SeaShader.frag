#version 450#extension GL_ARB_separate_shader_objects : enablelayout(location = 0) in vec3 fragPos;layout(location = 1) in vec3 fragNorm;layout(location = 0) out vec4 outColor;layout(set = 0, binding = 1) uniform GlobalUniformBufferObject {    bool spot;    vec3 lightPos;    vec3 lightDir;    vec4 lightColor;    vec3 eyePos;    float time;} gubo;vec3 toonDiffuse(vec3 L, vec3 N, vec3 V, vec3 light, vec3 dark) {    float cosAngle = dot(L, N);    vec3 color;    if (cosAngle >= 0.9f)    color = dark;    else    color = mix(light, dark, cosAngle);    return color;}vec3 toonSpecular(vec3 L, vec3 N, vec3 V, vec3 C, float thr)  {    float intensity = dot(V, -reflect(L, N));    vec3 color;    if (intensity >= thr) color = C;    else color = vec3(0, 0, 0);    return color;}const float beta = 2.0f;const float g = 1.5f;const float cosout = 0.85;const float cosin  = 0.85;void main() {    vec3 norm = normalize(fragNorm);    vec3 eyeDir = normalize(gubo.eyePos - fragPos);    float t = gubo.time;    float freq = 1.0f;    // Define parameters for the Gaussian RBF    float x = fragPos.x - 10.0f;    float z = fragPos.z - 10.0f;    float distanceFromCenter = sqrt(x * x + z * z);    float amplitude = 2.0f;// Amplitude of the RBF    float sigmaSquared = 100.0f;// Variance of the RBF    float normalizedDistanceFromCenter = distanceFromCenter / 10.0f;// Normalize distance to range [0,1]    float nearIsland = (-normalizedDistanceFromCenter * distanceFromCenter / sigmaSquared);    vec3 beachShore = int(10.0f*nearIsland)/10.0f * vec3(1.0f, 1.0f, 0.0f);    float trigo = sin(0.1f*fragPos.z + 0.1f*fragPos.x + cos((t*0.05f)/(nearIsland*nearIsland)));    trigo = int(5.0f*trigo)/5.0f;//discretization    vec3 highColor = vec3(0.0f, 100.0f, 150.0f)/255.0f + trigo*trigo*vec3(50.0f, 105.0f, 105.f)/255.0f + beachShore;    vec3 darkColor = trigo*trigo*vec3(0.0f, 0.0f, 100.0f)/255.0f + beachShore;    if (!gubo.spot) {        vec3 diffuse = toonDiffuse(gubo.lightDir, norm, eyeDir, darkColor, highColor);        vec3 ambient = darkColor * 0.025f;        outColor = vec4(diffuse + ambient, 1.0f);    } else {        vec3 lightDir = gubo.lightPos - fragPos;        float dim = clamp((dot(normalize(lightDir), gubo.lightDir) - cosout)/(cosin - cosout), 0.0f, 1.0f);        float fadeOut = pow(g/length(lightDir), beta);        vec3 lightColor = gubo.lightColor.rgb * fadeOut * dim;        vec3 diffuse = toonDiffuse(lightDir, norm, eyeDir, darkColor, highColor);        vec3 specular = vec3(pow(clamp(dot(norm, normalize(lightDir + eyeDir)), 0.0f, 1.0f), 160.0f));        vec3 ambient = darkColor * 0.025f;        outColor = vec4(clamp((diffuse + specular) * lightColor.rgb + ambient, 0.0f, 1.0f), 1.0f);    }}