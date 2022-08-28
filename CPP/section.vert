//Vertex shader source code
#version 330 core
layout (location = 0) in float m;
layout (location = 1) in vec3 eps;
layout (location = 2) in vec3 L;
layout (location = 3) in vec3 r0;
layout (location = 4) in float deltaE;

out VS_OUT {
	float m;
	vec3 eps;
	vec3 L;
	vec3 r0;
	float deltaE;
} vs_out;


void main()
{
	gl_Position = vec4(0, 0, 0, 1);
	vs_out.m = m;
	vs_out.eps = eps;
	vs_out.L = L;
	vs_out.r0 = r0;
	vs_out.deltaE = deltaE;
}