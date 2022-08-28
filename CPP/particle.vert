//Vertex shader source code
#version 330 core
layout (location = 0) in float m;
layout (location = 1) in vec3 eps;
layout (location = 2) in vec3 L;
layout (location = 3) in vec3 r_0;
layout (location = 4) in float t_0;
layout (location = 5) in float phaseShift;

#define M_PI 3.1415926535897932384626433832795

out float eyeDistance;
out float depth;
out float pointSize;

uniform float GM = 1;

uniform mat4 camMatrix;

float MA_to_E(float MA, float eps){
	float E = 0;
	for(int i = 0; i < 100; i++){
		E = MA + eps * sin(E);
	}
	return E;
}

void main(){
	float l = dot(L, L) / (GM * pow(m, 2));
	float a = l / (1 - dot(eps, eps));
	float b = a * sqrt(1 - dot(eps, eps));
	vec3 eps_hat;
	vec3 ortho_eps_hat;

	vec3 L_hat = normalize(L);

	if(length(eps) == 0){
		//no eccentricity
		if(L.z == 1){
			//Angular momentum has only non-zero z, so no rotation. Eccentricity unit becomes unit vector along x
			eps_hat = vec3(1, 0, 0);
		}else{
			//Angular momentum has other non-zero components besides z
			float denominator = sqrt(1 - pow(L_hat.z, 2));
			eps_hat = vec3(L_hat.y / denominator, -L_hat.x / denominator, 0);
		}
	}else{
		//Non-zero eccentricity
		eps_hat = normalize(eps);
	}

	ortho_eps_hat = normalize(cross(L_hat, eps_hat));

	//Calculate eccentric anomaly E_0
	vec3 rel_r_0 = r_0 + eps * a;
	float sinE = dot(rel_r_0, ortho_eps_hat) / b;
	float cosE = dot(rel_r_0, eps_hat) / a;
	float E_0 = atan(sinE, cosE); //Note that syntax is atan(y, x)
	//convert from [-pi, pi] = [0, 2*pi]
	if (E_0 < 0) {
		E_0 += 2 * M_PI;
	}

	//Mean anomaly
	float MA_0 = E_0 - length(eps) * sinE;
	float MA_current = MA_0 + phaseShift;

	float E = MA_to_E(MA_current, length(eps));
	vec3 r = -eps * a + eps_hat * a * cos(E) + ortho_eps_hat * b * sin(E);

	//Final position in perspective view
	gl_Position = camMatrix * vec4(r, 1);
	eyeDistance = gl_Position.w;
	depth = gl_Position.z;
	pointSize = gl_PointSize;
}