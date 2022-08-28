#include"ClosestApproach.h"

std::string ClosestApproach::String() {
	std::string output = "ClosestApproach(";
	output += "interact: " + std::to_string(this->possibleInteraction) + ", ";
	output += "d: " + std::to_string(this->d) + ", ";
	output += "r_1: " + glm::to_string(this->r1) + ", ";
	output += "r_2: " + glm::to_string(this->r2) + ", ";
	output += "v_1: " + glm::to_string(this->v1) + ", ";
	output += "v_2: " + glm::to_string(this->v2) + ", ";
	output += "w^2: " + std::to_string(this->w_sq) + ", ";


	output += "deltaE_1: " + std::to_string(this->deltaE_1) + ", ";
	output += "deltaE_2: " + std::to_string(this->deltaE_2) + ", ";
	output += "t_1: " + std::to_string(this->t1) + ", ";
	output += "t_2: " + std::to_string(this->t2) + ", ";
	output += "reciDiff: " + std::to_string(this->reciDiff) + ", ";
	output += "delta: " + std::to_string(this->delta) + ", ";
	output += "colTime: " + std::to_string(this->colTime) + ", ";

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

void ClosestApproach::CalcClosestApproach(Particle* first, Particle* second, ClosestApproach* target, double GM, double M, bool side) {
	//Make sure the first particle is the one with the smallest omega
	target->first = first;
	target->second = second;
	
	//[1] Calculate positions and velocities at node line
	glm::dvec3 K = glm::cross(first->L, second->L);
	double K_magnitude = glm::length(K);
	double l1 = first->Parameter_l(GM);
	double l2 = second->Parameter_l(GM);

	//Loop through both collision points
	//define a sign, which will make the second solution different
	int sign = 1;
	if (side == 0) {
		sign = -1;
	}

	glm::dvec3 v1_temp, v2_temp;
	glm::dvec3 r1_prev, r2_prev;
	glm::dvec3 r1_temp = K * l1 / (sign * K_magnitude + glm::dot(first->eps, K)); //position of node 1
	glm::dvec3 r2_temp = K * l2 / (sign * K_magnitude + glm::dot(second->eps, K)); //position of node 2
#if GRAPHICAL and VISUAL_MODE
	target->r1_node = r1_temp;
	target->r2_node = r2_temp;
	//target->r_list.push_back(r1_temp);
	//target->r_list.push_back(r2_temp);
#endif
	glm::dvec3 d_temp, w;
	double w_sq;
	double max_distance = first->s + second->s;
	unsigned int OPI = 0; //Number of Orbital Projection Iterations

	//Iteration of linearization - minimalisation - orbital projection
	//do{
		//reset previous r
		r1_prev = r1_temp;
		r2_prev = r2_temp;

		//Calculate velocity
		v1_temp = glm::cross(first->L / (first->m * l1), first->eps + glm::normalize(r1_temp));
		v2_temp = glm::cross(second->L / (second->m * l2), second->eps + glm::normalize(r1_temp));

		//Calculate approximate minimum distance point
		d_temp = r2_temp - r1_temp;
		w = glm::cross(v1_temp, v2_temp);
		w_sq = glm::dot(w, w);
		r1_temp = r1_temp + glm::dot(d_temp, glm::cross(v2_temp, w) / w_sq) * v1_temp;
		r2_temp = r2_temp + glm::dot(d_temp, glm::cross(v1_temp, w) / w_sq) * v2_temp;

		//Orbital projection
		//r1_temp = glm::normalize(r1_temp) * l1 / (1 + glm::dot(first->eps, glm::normalize(r1_temp)));
		//r2_temp = glm::normalize(r2_temp) * l2 / (1 + glm::dot(first->eps, glm::normalize(r2_temp)));

#if GRAPHICAL and VISUAL_MODE
		//target->r_list.push_back(r1_temp);
		//target->r_list.push_back(r2_temp);
#endif

		//OPI++;
		//if (OPI == 5) {
			//std::cout << "[OPI] WARNING: Reached max number of iterations.\n";
			//break;
		//}
	//} while (glm::length(r1_temp - r1_prev) + glm::length(r2_temp - r2_prev) > max_distance*1e5);
	//std::cout << "OPI: " << OPI << "\n";

	target->v1 = v1_temp;
	target->v2 = v2_temp;
	target->r1 = r1_temp;
	target->r2 = r2_temp;
	target->w_sq = w_sq;

	glm::dvec3 d_closest = target->r2 - target->r1;
	target->d = glm::length(d_closest);

	//[3] Decide if interaction can take place
	if (target->d < max_distance) {
		//Collision possible
		target->possibleInteraction = 2;
	}
	else if (target->d < std::max(first->SphereOfInfluence(M), second->SphereOfInfluence(M))) {
		//Influence possible
		target->possibleInteraction = 1;
	}
	else {
		//No interaction possible
		target->possibleInteraction = 0;
	}
}

bool ClosestApproach::CompareColTime(ClosestApproach first, ClosestApproach second) {
	if (first.colTime < second.colTime) {
		return true;
	}
	else {
		return false;
	}
}

#if GRAPHICAL and VISUAL_MODE
void ClosestApproach::WriteVAO(GLfloat* arrows, GLfloat* section) {
	//arrows pointing from orbital nodes to predicted closest approach coordinates
	arrows[0] = (GLfloat)this->r1_node.x;
	arrows[1] = (GLfloat)this->r1_node.y;
	arrows[2] = (GLfloat)this->r1_node.z;
	arrows[3] = (GLfloat)this->r1.x;
	arrows[4] = (GLfloat)this->r1.y;
	arrows[5] = (GLfloat)this->r1.z;
	arrows[6] = (GLfloat)this->r2_node.x;
	arrows[7] = (GLfloat)this->r2_node.y;
	arrows[8] = (GLfloat)this->r2_node.z;
	arrows[9] = (GLfloat)this->r2.x;
	arrows[10] = (GLfloat)this->r2.y;
	arrows[11] = (GLfloat)this->r2.z;

	/*delete[] arrowVAO;
	arrowVAO = new GLfloat[r_list.size()];
	int counter_arrows = 0;
	//Loop through r1's
	for (std::list<glm::dvec3>::iterator i = this->r_list.begin(); i != this->r_list.end();)
	{
		arrowVAO[counter_arrows++] = (GLfloat) i->x;
		arrowVAO[counter_arrows++] = (GLfloat) i->y;
		arrowVAO[counter_arrows++] = (GLfloat) i->z;
		i++++;
	}
	//Loop through r2's
	for (std::list<glm::dvec3>::iterator i = ++this->r_list.begin(); i != this->r_list.end();)
	{
		arrowVAO[counter_arrows++] = (GLfloat)i->x;
		arrowVAO[counter_arrows++] = (GLfloat)i->y;
		arrowVAO[counter_arrows++] = (GLfloat)i->z;
		i++++;
	}*/

	//Section VAO
	int counter = 0;
	Particle* p;
	for (int i = 0; i < 2; i++)
	{
		//which particle are we looking at now?
		if (i == 0) {
			p = this->first;
		}
		else {
			p = this->second;
		}

		//semi-major axis
		section[counter++] = (GLfloat)p->m;
		//Eccentricity vector
		section[counter++] = (GLfloat)p->eps.x;
		section[counter++] = (GLfloat)p->eps.y;
		section[counter++] = (GLfloat)p->eps.z;
		//Angular momentum vector
		section[counter++] = (GLfloat)p->L.x;
		section[counter++] = (GLfloat)p->L.y;
		section[counter++] = (GLfloat)p->L.z;
		//r_0
		section[counter++] = (GLfloat)p->r_0.x;
		section[counter++] = (GLfloat)p->r_0.y;
		section[counter++] = (GLfloat)p->r_0.z;
		counter++;
	}
	//deltaE
	section[10] = (GLfloat)this->deltaE_1;
	section[21] = (GLfloat)this->deltaE_2;
}
#endif