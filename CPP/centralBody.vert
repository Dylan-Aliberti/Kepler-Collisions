//Vertex shader source code
#version 330 core
layout (location = 0) in vec3 aPos;

out float eyeDistance;

uniform mat4 camMatrix;

void main()
{
	gl_Position = camMatrix * vec4(aPos, 1.0);
	eyeDistance = gl_Position.w;
}