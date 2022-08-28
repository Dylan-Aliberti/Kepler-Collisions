//Fragment shader source code
#version 330 core

in float eyeDistance;

uniform vec3 primaryColor = vec3(0, 0, 1);
uniform vec3 secondaryColor = vec3(0, 1, 0);

out vec4 FragColor;

float eyeDistanceScale = 1;

void main()
{
	//FragColor = vec4(0, pow(eyeDistanceScale / eyeDistance, 0.5), 1, 1);
	FragColor = vec4(primaryColor + secondaryColor * pow(eyeDistanceScale / eyeDistance, 0.5), 1);
}