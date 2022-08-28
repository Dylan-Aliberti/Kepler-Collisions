//Fragment shader source code
#version 330 core

out vec4 FragColor;

uniform vec3 color = vec3(1, 1, 0);
uniform bool schematicMode = false;
in float eyeDistance;

float scale_color = 5;
float scale_white = 10;

float distanceItensity(vec2 pos, float scale){
	vec2 relPos = scale * pos;
	//Gaussian decay
	return clamp(exp(- dot(relPos, relPos)), 0, 1);
}

void main()
{
	vec2 pos =  (gl_PointCoord - vec2(0.5, 0.5) );
	if(schematicMode){
		if(length(pos) <= 0.5){
			FragColor = vec4(color, 1);
		}else{
			FragColor = vec4(0, 0, 0, 0);
		}
	}else{
		float I1 = distanceItensity(pos, scale_color);
	float I2 = distanceItensity(pos, scale_white);
	FragColor = vec4(color + I2 * vec3(1, 1, 1), I1);
	}
}