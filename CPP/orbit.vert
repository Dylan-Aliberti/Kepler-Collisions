//Vertex shader source code
#version 330 core
layout (location = 0) in float m;
layout (location = 1) in vec3 eps;
layout (location = 2) in vec3 L;

out VS_OUT {
	float m;
	vec3 eps;
	vec3 L;
} vs_out;


void main()
{
	gl_Position = vec4(0, 0, 0, 1);
	vs_out.m = m;
	vs_out.eps = eps;
	vs_out.L = L;
}