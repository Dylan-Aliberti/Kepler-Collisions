#version 330 core
layout (points) in;
layout (line_strip, max_vertices = 6) out;

out vec3 color;

uniform float scale;

uniform mat4 camMatrix;

void main() {
	//x axis
	color = vec3(1, 0, 0);
	gl_Position = camMatrix * ( gl_in[0].gl_Position );
	EmitVertex();
	gl_Position = camMatrix * ( gl_in[0].gl_Position + vec4(scale * vec3(1, 0, 0), 1) );
	EmitVertex();
	EndPrimitive();

	//y axis
	color = vec3(0, 1, 0);
	gl_Position = camMatrix * ( gl_in[0].gl_Position );
	EmitVertex();
	gl_Position = camMatrix * ( gl_in[0].gl_Position + vec4(scale * vec3(0, 1, 0), 1) );
	EmitVertex();
	EndPrimitive();

	//z axis
	color = vec3(0.8, 0.8, 0);
	gl_Position = camMatrix * ( gl_in[0].gl_Position );
	EmitVertex();
	gl_Position = camMatrix * ( gl_in[0].gl_Position + vec4(scale * vec3(0, 0, 1), 1) );
	EmitVertex();
	EndPrimitive();
}