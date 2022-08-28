#version 330 core
layout (points) in;
layout (triangle_strip, max_vertices = 205) out;

#define M_PI 3.1415926535897932384626433832795

in VS_OUT {
	float m;
	vec3 eps;
	vec3 L;
	vec3 r0;
	float deltaE;
} gs_in[];

uniform float GM = 1;

uniform mat4 camMatrix;

vec3 posFrom_mel_eccentric(float E, float m, vec3 eps, vec3 L){
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
			eps_hat = vec3(-L_hat.y / denominator, L_hat.x / denominator, 0);
		}
	}else{
		//Non-zero eccentricity
		eps_hat = normalize(eps);
	}

	ortho_eps_hat = normalize(cross(L_hat, eps_hat));

	vec3 r = -eps * a + eps_hat * a * cos(E) + ortho_eps_hat * b * sin(E);

	//Final position in perspective view
	return r;
}

float EFromPosition(vec3 r_0, float m, vec3 eps, vec3 L){
	//Returns eccentric anomaly, given the corresponding position of the particle
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
			eps_hat = vec3(-L_hat.y / denominator, L_hat.x / denominator, 0);
		}
	}else{
		//Non-zero eccentricity
		eps_hat = normalize(eps);
	}

	ortho_eps_hat = normalize(cross(L_hat, eps_hat));

	vec3 rel_r_0 = r_0 + eps * a;

	float sinE = dot(rel_r_0, ortho_eps_hat) / b;
	float cosE = dot(rel_r_0, eps_hat) / a;
	float E_0 = atan(sinE, cosE); //Note that syntax is atan(y, x)
	//convert from [-pi, pi] = [0, 2*pi]
	if (E_0 < 0) {
		E_0 += 2 * M_PI;
	}

	return E_0;
}

void main() {
	float minAngle = EFromPosition(gs_in[0].r0, gs_in[0].m, gs_in[0].eps, gs_in[0].L);
	float maxAngle = minAngle + gs_in[0].deltaE;
	float angle = 0;
	//vertex at central body
	gl_Position = camMatrix * vec4(0, 0, 0, 1);
	EmitVertex();
    for(int i = 0; i <= 100; i++){
		angle = minAngle + (maxAngle - minAngle) * i / 100.0;
		gl_Position = camMatrix * vec4(posFrom_mel_eccentric(angle, gs_in[0].m, gs_in[0].eps, gs_in[0].L), 1.0);
		EmitVertex();
		gl_Position = camMatrix * vec4(0, 0, 0, 1);
		EmitVertex();
	}
	//vertex at central body
	gl_Position = camMatrix * vec4(0, 0, 0, 1);
	EmitVertex();
	EndPrimitive();
}