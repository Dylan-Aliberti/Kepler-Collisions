#version 330 core
layout (lines) in;
layout (line_strip, max_vertices = 6) out;

uniform float scale = 0.1;

uniform mat4 camMatrix;

uniform vec3 camPos;

void main() {
	vec4 startPoint = gl_in[0].gl_Position;
	vec4 endPoint = gl_in[1].gl_Position;

	vec3 playerToArrow = normalize(endPoint.xyz - camPos.xyz);

	vec4 delta = endPoint - startPoint;

	//crosspoint
	vec4 cPoint = endPoint - scale * delta;

	//side points
	vec4 sideVector = vec4(0.5 * scale * normalize(cross(playerToArrow, delta.xyz)) * length(delta.xyz), delta.w);
	vec4 s1Point = cPoint - sideVector;
	vec4 s2Point = cPoint + sideVector;

	//Now connect all the points
	gl_Position = camMatrix * startPoint;
	EmitVertex();
	gl_Position = camMatrix * cPoint;
	EmitVertex();
	gl_Position = camMatrix * s1Point;
	EmitVertex();
	gl_Position = camMatrix * endPoint;
	EmitVertex();
	gl_Position = camMatrix * s2Point;
	EmitVertex();
	gl_Position = camMatrix * cPoint;
	EmitVertex();
	EndPrimitive();
}