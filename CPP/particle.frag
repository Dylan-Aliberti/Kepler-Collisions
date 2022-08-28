//Fragment shader source code
#version 330 core

out vec4 FragColor;

in float eyeDistance;
in float depth;
in float pointSize;

uniform vec3 color = vec3(1, 0, 0);
uniform float radius = 0.05; //worldspace coordinates?

void main()
{
	vec2 relPos = 1 * vec2(gl_PointCoord.x - 0.5, gl_PointCoord.y - 0.5);
	
	//if(length(relPos) <= radius / eyeDistance){
	if(length(relPos) <= 0.5){
		FragColor = vec4(color, 1);
		gl_FragDepth = gl_FragCoord.z;
	}else{
		FragColor = vec4(0, 0, 0, 0);
		gl_FragDepth = 100;
	}
	
}