#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform UniformBufferObject {
	mat4 mvpMat;
	mat4 mMat;
	mat4 nMat;
} ubo;

layout(set = 0, binding = 1) uniform GlobalUniformBufferObject {
	vec3 selector;
	vec3 lightDir;
	vec3 lightColor;
	vec3 eyePos;
} gubo;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNorm;

void main() {
	float x = pos.x;
	float y = pos.y;
	float z = pos.z;
//	float t = gubo.time;
	
//	/***** Insert the code here to compute the correct value of y ****/
//	float cx = sin(t/3) / 2;
//	float cz = cos(t/3) / 2;
//	float l = sqrt(pow((x - cx), 2) + pow((z - cz), 2));
//	y = y + sin(t/4  + 4 * l) * exp(-l * l / 4);
//	/***** Leave it as is from this point on ****/
	
	vec3 vpos = vec3(x, y, z);
	
	gl_Position = ubo.mvpMat * vec4(vpos, 1.0);
	fragPos = (ubo.mMat * vec4(vpos, 1.0)).xyz;
	fragNorm = norm;
}