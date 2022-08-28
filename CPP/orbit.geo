#version 330 core
layout (points) in;
layout (triangle_strip, max_vertices = 203) out;

in VS_OUT {
	float m;
	vec3 eps;
	vec3 L;
} gs_in[];

out float eyeDistance;
out float distToLine;

uniform float scale = 0.01; //world space scale of orbits

uniform float GM = 1;

uniform mat4 camMatrix;

uniform vec3 camPos;

vec3 posFrom_mel(float angle, float m, vec3 eps, vec3 L){
	float l = dot(L, L) / (GM * pow(m, 2));
	float r_mag = l / (1 + length(eps) * cos(angle));
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

	vec3 r = r_mag * ( (eps_hat * cos(angle)) + (ortho_eps_hat * sin(angle)) );

	//Final position in perspective view
	return r;
}

vec3 velocity(vec3 pos, float m, vec3 eps, vec3 L){
	float l = dot(L, L) / (GM * pow(m, 2));
	return (1 / (m * l )) * cross(L, eps + normalize(pos));
}

void main() {
	float minAngle, maxAngle, angle, angle_1, angle_2;
	vec3 pos, normalVel, orthoVec;
	float lEps = length(gs_in[0].eps);
	if(lEps >= 1){
		//escape orbit
		float p = acos(-1 / lEps) - 0.001;
		minAngle = - p;
		maxAngle = p;
	}else{
		minAngle = 0;
		maxAngle = 2 * 3.141592;
	}

    for(int i = 0; i <= 100; i++){
		angle = minAngle + (maxAngle - minAngle) * i / 100;
		pos = posFrom_mel(angle, gs_in[0].m, gs_in[0].eps, gs_in[0].L);
		normalVel = velocity(pos, gs_in[0].m, gs_in[0].eps, gs_in[0].L);
		orthoVec = normalize(cross(pos - camPos, normalVel));

		gl_Position = camMatrix * vec4(pos + scale * orthoVec, 1);
		eyeDistance = gl_Position.w;
		EmitVertex();

		gl_Position = camMatrix * vec4(pos - scale * orthoVec, 1);
		eyeDistance = gl_Position.w;
		EmitVertex();
	}

	if(length(lEps) < 1){
		//close the orbit
		angle = minAngle;
		pos = posFrom_mel(angle, gs_in[0].m, gs_in[0].eps, gs_in[0].L);
		normalVel = velocity(pos, gs_in[0].m, gs_in[0].eps, gs_in[0].L);
		orthoVec = normalize(cross(pos - camPos, normalVel));

		gl_Position = camMatrix * vec4(pos + scale * orthoVec, 1);
		eyeDistance = gl_Position.w;
		EmitVertex();
	}

	EndPrimitive();
}