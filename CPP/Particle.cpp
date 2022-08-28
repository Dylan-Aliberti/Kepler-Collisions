#include"Particle.h"

bool Particle::ComparePeriapsis(Particle first, Particle second) {
	if (first.a - first.c < second.a - first.c) {
		return true;
	}
	else {
		return false;
	}
}

bool Particle::SortedRangeOverlap(Particle small_periapsis, Particle large_periapsis) {
	if (small_periapsis.a + small_periapsis.c + small_periapsis.s >= large_periapsis.a - large_periapsis.c - large_periapsis.s) {
		return true;
	}
	else {
		return false;
	}
}

glm::dmat3x3 Particle::KeplerRotationMatrix(double AP, double AN, double I) {
	glm::dmat3x3 R_AN = glm::dmat3x3(
		cos(AN), -sin(AN), 0,
		sin(AN), cos(AN), 0,
		0, 0, 1
	);

	glm::dmat3x3 R_I = glm::dmat3x3(
		1, 0, 0,
		0, cos(I), -sin(I),
		0, sin(I), cos(I)
	);

	glm::dmat3x3 R_AP = glm::dmat3x3(
		cos(AP), -sin(AP), 0,
		sin(AP), cos(AP), 0,
		0, 0, 1
	);

	glm::dmat3x3 R = R_AN * R_I * R_AP;

	return R;
}

Particle Particle::ParticleFromKepler(double AP, double AN, double I, double TA, double a, double eps, double m, double s, double t_0, double GM) {
	Particle p;
	p.t_0 = t_0;
	p.a = a;
	p.c = a * eps;
	p.omega = sqrt(GM / pow(a, 3));
	p.s = s;
	p.m = m;
	glm::dmat3x3 R = Particle::KeplerRotationMatrix(AP, AN, I);
	double b = sqrt(1 - pow(eps, 2)) * a;
	double L = m * p.omega * a * b;
	p.L = L * R * glm::dvec3(0, 0, 1);
	p.eps = eps * R * glm::dvec3(1, 0, 0);
	double r_mag = p.Parameter_l(GM) / (1 + eps * cos(TA));
	p.r_0 = r_mag * R * glm::dvec3(cos(TA), sin(TA), 0);

	return p;
}

Particle Particle::ParticleFromKepler_Eccentric(double AP, double AN, double I, double EA, double a, double eps, double m, double s, double t_0, double GM) {
	Particle p;
	p.t_0 = t_0;
	p.a = a;
	p.c = a * eps;
	p.omega = sqrt(GM / pow(a, 3));
	p.s = s;
	p.m = m;
	glm::dmat3x3 R = Particle::KeplerRotationMatrix(AP, AN, I);
	double b = sqrt(1 - pow(eps, 2)) * a;
	double L = m * p.omega * a * b;
	p.L = L * R * glm::dvec3(0, 0, 1);
	p.eps = eps * R * glm::dvec3(1, 0, 0);
	p.r_0 = R * glm::dvec3(-a*eps + a * cos(EA), b * sin(EA), 0);

	return p;
}

Particle Particle::ParticleFromCartesian(glm::dvec3 r, glm::dvec3 v, double m, double s, double t_0, double GM) {
	Particle p;
	p.t_0 = t_0;
	p.L = glm::cross(r, m * v);
	p.eps = glm::cross(v, p.L) / (GM * m) - glm::normalize(r);
	if (length(p.eps) == 1) {
		//Nope, avoid that zero division
		p.a = INFINITY;
		p.c = INFINITY;
		p.omega = 0;
	}
	else {
		p.a = 1 / (1 - glm::dot(p.eps, p.eps)) * glm::dot(p.L, p.L) / (GM * pow(m, 2));
		p.c = p.a * glm::length(p.eps);
		p.omega = sqrt(GM / pow(p.a, 3));
	}
	p.s = s;
	p.m = m;
	p.r_0 = r;
	return p;
}

Particle Particle::ParticleFromLER(glm::dvec3 L, glm::dvec3 eps, glm::dvec3 r_0, double m, double s, double t_0, double GM) {
	Particle p;
	p.L = L;
	p.eps = eps;
	p.r_0 = r_0;
	p.t_0 = t_0;
	p.m = m;
	p.s = s;
	if (length(p.eps) == 1) {
		//Avoid division by zero
		p.a = INFINITY;
		p.c = INFINITY;
		p.omega = 0;
	}
	else {
		p.a = 1 / (1 - glm::dot(p.eps, p.eps)) * glm::dot(p.L, p.L) / (GM * pow(m, 2));
		p.c = p.a * glm::length(p.eps);
		p.omega = sqrt(GM / pow(p.a, 3));
	}
	return p;
}

double Particle::EAFromMA(double MA, double eps, int iterations) {
	double E = MA;
	for (int i = 0; i < iterations; i++)
	{
		E = MA + eps * std::sin(E);
	}
	return E;
}

glm::dvec3 Particle::VelocityFromR(glm::dvec3 r, double GM) {
	return (GM * this->m / glm::dot(this->L, this->L)) * glm::cross(this->L, this->eps + glm::normalize(r));
}

double Particle::Parameter_l(double GM) {
	return glm::dot(L, L) / (GM * std::pow(m, 2));
}

glm::dvec3 Particle::EpsFromVLR(glm::dvec3 v, glm::dvec3 r, double GM) {
	return glm::cross(v, this->L) / (GM * this->m) - glm::normalize(r);
}

glm::dvec3 Particle::PositionFromTA(double TA, double GM) {
	double l = this->Parameter_l(GM);
	double r_mag = l / (1 + glm::length(this->eps) * std::cos(TA));
	glm::dvec3 eps_hat;
	glm::dvec3 ortho_eps_hat;

	glm::dvec3 L_hat = glm::normalize(this->L);

	if (glm::length(this->eps) == 0) {
		//no eccentricity
		if (this->L.z == 1) {
			//Angular momentum has only non-zero z, so no rotation. Eccentricity unit becomes unit vector along x
			eps_hat = glm::dvec3(1, 0, 0);
		}
		else {
			//Angular momentum has other non-zero components besides z
			double denominator = std::sqrt(1 - std::pow(L_hat.z, 2));
			eps_hat = glm::dvec3(L_hat.y / denominator, -L_hat.x / denominator, 0);
		}
	}
	else {
		//Non-zero eccentricity
		eps_hat = glm::normalize(this->eps);
	}

	ortho_eps_hat = glm::normalize(glm::cross(L_hat, eps_hat));

	glm::dvec3 r = r_mag * ((eps_hat * std::cos(TA)) + (ortho_eps_hat * std::sin(TA)));

	//Final position in perspective view
	return r;
}

double Particle::TAFromPosition(glm::dvec3 r) {
	glm::dvec3 eps_hat;
	glm::dvec3 ortho_eps_hat;

	glm::dvec3 L_hat = glm::normalize(this->L);

	if (glm::length(this->eps) == 0) {
		//no eccentricity
		if (this->L.z == 1) {
			//Angular momentum has only non-zero z, so no rotation. Eccentricity unit becomes unit vector along x
			eps_hat = glm::dvec3(1, 0, 0);
		}
		else {
			//Angular momentum has other non-zero components besides z
			double denominator = std::sqrt(1 - std::pow(L_hat.z, 2));
			eps_hat = glm::dvec3(L_hat.y / denominator, -L_hat.x / denominator, 0);
		}
	}
	else {
		//Non-zero eccentricity
		eps_hat = glm::normalize(this->eps);
	}

	ortho_eps_hat = glm::normalize(glm::cross(L_hat, eps_hat));

	double r_ortho = glm::dot(r, ortho_eps_hat);
	double r_para = glm::dot(r, eps_hat);
	double TA = std::atan2(r_ortho, r_para); //Note that syntax is atan2(y, x)
	//convert from [-pi, pi] = [0, 2*pi]
	if (TA < 0) {
		TA += 2 * PI;
	}

	return TA;
}

double Particle::SphereOfInfluence(double M) {
	return std::pow(this->m / M, 0.4f) * this->a;
}

double Particle::deltaE(glm::dvec3 r_CA, glm::dvec3 v_CA) {
	double eps_sq = glm::dot(this->eps, this->eps);
	double r_mag = glm::length(r_CA);
	double b_sq = std::pow(this->a, 2) * (1 - eps_sq);
	double b = std::sqrt(b_sq);
	//METHOD 1 - formula with complex number
	glm::dvec3 temp = (this->r_0 - this->eps * glm::dot(this->eps, this->r_0)) / (this->a - eps_sq * this->a) + this->eps;
	double z_re = glm::dot(r_CA / this->a, temp) + glm::dot(this->r_0, this->eps) / this->a + eps_sq;
	double z_im = -(r_mag / (std::pow(this->a, 2) * this->omega)) * glm::dot(v_CA, temp);
	double deltaE = std::atan2(z_im, z_re);

	//convert from [-PI, PI] to [0, 2*PI]
	if (deltaE < 0) {
		deltaE += 2 * PI;
	}
	return deltaE;
}

double Particle::EccFromPosition(glm::dvec3 r) {
	//Returns eccentric anomaly, given the corresponding position of the particle
	double b = this->a * sqrt(1 - dot(eps, eps));
	glm::dvec3 eps_hat;
	glm::dvec3 ortho_eps_hat;

	glm::dvec3 L_hat = glm::normalize(L);

	if (glm::length(eps) == 0) {
		//no eccentricity
		if (L.z == 1) {
			//Angular momentum has only non-zero z, so no rotation. Eccentricity unit becomes unit vector along x
			eps_hat = glm::dvec3(1, 0, 0);
		}
		else {
			//Angular momentum has other non-zero components besides z
			double denominator = sqrt(1 - pow(L_hat.z, 2));
			eps_hat = glm::dvec3(-L_hat.y / denominator, L_hat.x / denominator, 0);
		}
	}
	else {
		//Non-zero eccentricity
		eps_hat = glm::normalize(eps);
	}

	ortho_eps_hat = glm::normalize(glm::cross(L_hat, eps_hat));

	glm::dvec3 rel_r = r + eps * a;

	double sinE = glm::dot(rel_r, ortho_eps_hat) / b;
	double cosE = glm::dot(rel_r, eps_hat) / this->a;
	double E = std::atan2(sinE, cosE); //Note that syntax is atan(y, x)
	//convert from [-pi, pi] = [0, 2*pi]
	if (E < 0) {
		E += 2 * PI;
	}

	return E;
}

std::string Particle::String() {
	std::string output = "Particle(";
	output += "t_0: " + std::to_string(this->t_0) + ", ";
	output += "a: " + std::to_string(this->a) + ", ";
	output += "c: " + std::to_string(this->c) + ", ";
	output += "s: " + std::to_string(this->s) + ", ";
	output += "m: " + std::to_string(this->m) + ", ";
	output += "r_0: " + glm::to_string(this->r_0) + ", ";
	output += "L: " + glm::to_string(this->L) + ", ";
	output += "eps: " + glm::to_string(this->eps) + ", ";
	output += "omega: " + std::to_string(this->omega) + ")";
	return output;
}