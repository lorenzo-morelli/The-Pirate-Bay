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
	float time = gubo.time;
	float radius = 80.0f;


	float x = pos.x + radius * cos(time);
	float y = pos.y + radius * sin(time);
	float z = pos.z;

	vec3 vpos = vec3(x, y, z);

	gl_Position = ubo.mvpMat * vec4(vpos, 1.0);
	fragPos = (ubo.mMat * vec4(vpos, 1.0)).xyz;
	fragNorm = norm;
}