#include"Collision.h"

Particle Collision::SimpleMerge(Collision col, double GM, double S) {
	Particle p;
	//[1] Use conservation laws:
	//Material volume
	p.s = std::pow(std::pow(col.first->s, 3) + std::pow(col.second->s, 3), 1.0f / 3);
	//mass
	p.m = col.first->m + col.second->m;
	//Center-of-mass motion
	p.r_0 = (col.first->m * col.r_first + col.second->m * col.r_second) / p.m;
	//Momentum
	glm::dvec3 v_first = col.first->VelocityFromR(col.r_first, GM);
	glm::dvec3 v_second = col.second->VelocityFromR(col.r_second, GM);
	glm::dvec3 v_p = (col.first->m * v_first + col.second->m * v_second) / p.m;
	//Angular momentum
	p.L = p.m * glm::cross(p.r_0, v_p);

	//[2] Decide if particle collides with central body
	double l = p.Parameter_l(GM);
	p.eps = p.EpsFromVLR(v_p, p.r_0, GM);
	double eps_l = glm::length(p.eps);
	double r_l = glm::length(p.r_0);

	//For simple merger epsilon is always < 1, so no need to check eps > 1
	if (l <= (1 + eps_l) * S) {
		//new particle collides with M
		p.r_0 = glm::dvec3(0, 0, 0);
		//Since p collides, we don't have to calculate the rest. Return immediately
		return p;
	}

	//[3] Calculate other parameters
	p.a = l / (1 - std::pow(eps_l, 2));

	if (p.a <= 0) {
		//Invalid a
		p.r_0 = glm::dvec3(0, 0, 0); //This way the simulation will know this particle will not participate anymore.
		return p;
	}

	p.c = p.a * eps_l;
	p.omega = std::sqrt(GM / std::pow(p.a, 3));
	p.t_0 = col.time;

	return p;
}

bool Collision::CompareCollisionTime(Collision col1, Collision col2) {
	if (col1.time < col2.time) {
		return true;
	}
	else {
		return false;
	}
}

void Collision::CalcEarliestPassageTimes(ClosestApproach* CA, double GM) {
	//Calculate differences in eccentric anomalies
	CA->deltaE_1 = CA->first->deltaE(CA->r1, CA->v1);
	CA->deltaE_2 = CA->second->deltaE(CA->r2, CA->v2);

	//Calculate corresponding times
	CA->t1 = CA->first->t_0 + CA->deltaE_1 / CA->first->omega - glm::dot(
		glm::cross(CA->first->eps, CA->r1 - CA->first->r_0) / (1 - glm::dot(CA->first->eps, CA->first->eps)),
		CA->first->L / (GM * CA->first->m)
	);
	CA->t2 = CA->second->t_0 + CA->deltaE_2 / CA->second->omega - glm::dot(
		glm::cross(CA->second->eps, CA->r2 - CA->second->r_0) / (1 - glm::dot(CA->second->eps, CA->second->eps)),
		CA->second->L / (GM * CA->second->m)
	);
	CA->reciDiff = 1 / std::abs(CA->t2 - CA->t1);
}

void Collision::CalcParameter_delta(ClosestApproach* CA) {
	glm::dvec3 u = CA->v2 - CA->v1;
	CA->delta = CA->reciDiff * std::sqrt(glm::dot(u, u) * (std::pow(CA->first->s + CA->second->s, 2) - std::pow(CA->d, 2)) / CA->w_sq);
}

void Collision::CalcCollisionTime_XSearch(ClosestApproach* CA, int max_iterations, double max_colTime) {
	double T1, T2, t1, t2, p, q, q0, q1, q2, q3;
	__int64 a0, a1, x_min, x_max, x_max_sim, x, y, k, k0, k1, k2, k3;

	//Make sure the first particle in this calculation is the one with the lowest omega
	if (CA->first->omega <= CA->second->omega) {
		//1 and 2 stay the same
		T1 = 2 * PI / CA->first->omega;
		T2 = 2 * PI / CA->second->omega;
		t1 = CA->t1;
		t2 = CA->t2;
	}
	else {
		//1 and 2 should switch sides
		T1 = 2 * PI / CA->second->omega;
		T2 = 2 * PI / CA->first->omega;
		t1 = CA->t2;
		t2 = CA->t1;
	}

	p = T1 / abs(t2 - t1);
	q = T2 / abs(t2 - t1);


	q0 = p;
	q1 = q;
	k0 = 1;
	k1 = 0;
	if (t2 - t1 < 0) {
		k0 = 0;
		k1 = 1;
	}

	for (int n = 0; n < max_iterations; n++)
	{
		a0 = (__int64)(q0 / q1);
		q2 = q0 - a0 * q1;
		if (std::abs(q2) <= std::numeric_limits<double>::epsilon() * std::abs(q2) * 2) {
			//Resonance! No collision will happen
			CA->colTime = INFINITY;
			return;
		}
		k2 = k0 - a0 * k1;
		a1 = (__int64)(q1 / q2);

		x_min = (__int64) std::ceil((1 - CA->delta) / q0);
		x_max = (__int64) std::floor((1 + CA->delta) / q2);
		x_max_sim = (__int64)((max_colTime - t1) / T1 * q1 - (1 + CA->delta) * k1) / (k0 * q1 - q0 * k1);

		for (x = x_min; x <= x_max; x++)
		{
			y = (__int64)std::max(0.0, std::ceil(q0 / q1 * x - (1 + CA->delta) / q1));
			//Cut-off for maximum time
			if (x > x_max_sim) {
				//Collision not within simulation time. Abort.
				CA->colTime = INFINITY;
				//std::cout << "aborted colTime: " << t1 + T1 * (x * k0 - y * k1) << "\n";
				return;
			}
			if (x * q0 - y * q1 > 1 - CA->delta) {
				k = x * k0 - y * k1;
				//Found the solution
				CA->colTime = t1 + T1 * k;
				return;
			}
		}

		q3 = q1 - a1 * q2;
		if (std::abs(q3) <= std::numeric_limits<double>::epsilon() * std::abs(q3) * 2) {
			//Resonance! No collision will happen
			CA->colTime = INFINITY;
			return;
		}
		k3 = k1 - a1 * k2;
		q0 = q2;
		q1 = q3;
		k0 = k2;
		k1 = k3;
	}
	CA->colTime = INFINITY;
	std::cout << "[CalcCollisionTime] WARNING: Exceeded max iterations (" << max_iterations << ") when calculating collision time!\n";
	return;
}

Collision Collision::ColFromCA(ClosestApproach* CA) {
	Collision col = Collision(CA->first, CA->second, CA->colTime, CA->r1, CA->r2);
#if GRAPHICAL and VISUAL_MODE
	col.CA = *CA;
#endif
	return col;
}

std::string Collision::String() {
	std::string output = "Collision(";
	output += "t: " + std::to_string(this->time) + ", ";
	output += "r_1: " + glm::to_string(this->r_first) + ", ";
	output += "r_2: " + glm::to_string(this->r_second) + ", ";

	void* p_1 = (void*)this->first;
	void* p_2 = (void*)this->second;
	std::stringstream ss;
	ss << p_1;
	output += "p_1: " + ss.str() + ", ";
	ss.str(std::string());
	ss << p_2;
	output += "p_2: " + ss.str() + ")";
	return output;
}