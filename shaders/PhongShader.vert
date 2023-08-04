#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform UniformBufferObject {
	mat4 mvpMat;
	mat4 mMat;
	mat4 nMat;
} ubo;

layout(set = 0, binding = 1) uniform GlobalUniformBufferObject {
	vec3 spot;
	vec3 lightPos;
	vec3 lightDir;
	vec4 lightColor;
	vec3 eyePos;
	float time;
} gubo;

layout(set = 0, binding = 2) uniform PositionsBuffer {
	vec4 positions[250000];
	vec4 colors[250000];
} positionsBuffer;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNorm;

void main() {
	float x = pos.x;
	float y = pos.y;
	float z = pos.z;
	float t = gubo.time;

	vec3 vpos = vec3(x, y, z) + positionsBuffer.positions[gl_InstanceIndex].xyz;
	
	gl_Position = ubo.mvpMat * vec4(vpos, 1.0);
	fragPos = (ubo.mMat * vec4(vpos, 1.0)).xyz;
	fragNorm = norm;
}