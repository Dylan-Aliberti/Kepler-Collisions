#include"Simulation.h"
#include"Delimiters.h"

void Simulation::FillColList(std::list<Particle>* pList, std::list<Collision>* colList) {
	std::list<Particle>::iterator looper_p2;
	for (std::list<Particle>::iterator looper_p1 = (*pList).begin(); looper_p1 != (*pList).end(); looper_p1++) {
		looper_p2 = looper_p1;
		looper_p2++;
		for (; looper_p2 != (*pList).end(); looper_p2++) {
			if (Particle::SortedRangeOverlap(*looper_p1, *looper_p2)) { //Sweep and prune
				//Ranges overlap. Candidate for collision
				CalcCollision(&*looper_p1, &*looper_p2, colList);
			}
		}
	}
}

void Simulation::SortedFillColList(std::list<Particle>* pList, std::list<Collision>* colList) {
#if INIT_MSG
	int counter = 0;
	int messageInterval = std::max((int)(*pList).size() / 100, 1);
#endif
	std::list<Particle>::iterator looper_p2;
	for (std::list<Particle>::iterator looper_p1 = (*pList).begin(); looper_p1 != (*pList).end(); looper_p1++) {
		looper_p2 = looper_p1;
		looper_p2++;
		for (; looper_p2 != (*pList).end(); looper_p2++) {
			if (Particle::SortedRangeOverlap(*looper_p1, *looper_p2)) { //Sweep and prune
				//Ranges overlap. Candidate for collision
				CalcCollision(&*looper_p1, &*looper_p2, colList);
				checks++;
			}
			else {
				break; //extra efficiency using info that given particle list is sorted
			}
		}
#if INIT_MSG
		counter++;
		if (counter % messageInterval == 0) {
			std::cout << "[Initialization] Collision pairs " << 100 * counter / (*pList).size() << " %\n";
		}
#endif
	}
}

void Simulation::CrossFillColList(std::list<Particle>* pList1, std::list<Particle>* pList2, std::list<Collision>* colList) {
	for (std::list<Particle>::iterator looper_p1 = pList1->begin(); looper_p1 != pList1->end(); looper_p1++)
	{
		for (std::list<Particle>::iterator looper_p2 = pList2->begin(); looper_p2 != pList2->end(); looper_p2++)
		{
			if (Particle::SortedRangeOverlap(*looper_p1, *looper_p2)) { //Sweep and prune
				//Ranges overlap. Candidate for collision
				CalcCollision(&*looper_p1, &*looper_p2, colList);
			}
		}
	}
}

void Simulation::CalcCollision(Particle* first, Particle* second, std::list<Collision>* colList) {
	for (int i = 0; i < 2; i++) //i is which of the 2 nodes we are looking at
	{
		ClosestApproach::CalcClosestApproach(&*first, &*second, &CurrentClosestApproach, GM, M, (i == 0));
		//TEMP ONLY FOR interaction > 1. SPHERE OF INFLUENCE STILL NEEDS TO BE IMPLEMENTED
		if (CurrentClosestApproach.possibleInteraction > 1) {
			pairs++;
			Collision::CalcEarliestPassageTimes(&CurrentClosestApproach, GM);
			Collision::CalcParameter_delta(&CurrentClosestApproach);
			Collision::CalcCollisionTime_XSearch(&CurrentClosestApproach, max_iterations, T_sim);
			if (CurrentClosestApproach.colTime < INFINITY) {
				//Planets are on collision course!
				Collision col = Collision::ColFromCA(&CurrentClosestApproach);
				colList->push_back(col);
			}
		}
	}
}

void Simulation::CountPairs(std::list<Particle>* pList) {
	std::list<Particle>::iterator looper_p2;
	for (std::list<Particle>::iterator looper_p1 = (*pList).begin(); looper_p1 != (*pList).end(); looper_p1++) {
		looper_p2 = looper_p1;
		looper_p2++;
		for (; looper_p2 != (*pList).end(); looper_p2++) {
			if (Particle::SortedRangeOverlap(*looper_p1, *looper_p2)) { //Sweep and prune
				checks++;
				//positive node
				ClosestApproach::CalcClosestApproach(&*looper_p1, &*looper_p2, &CurrentClosestApproach, GM, M, true);
				if (CurrentClosestApproach.possibleInteraction > 1) {
					pairs++;
				}
				//negative node
				ClosestApproach::CalcClosestApproach(&*looper_p1, &*looper_p2, &CurrentClosestApproach, GM, M, false);
				if (CurrentClosestApproach.possibleInteraction > 1) {
					pairs++;
				}
			}
			else {
				break; //extra efficiency using info that given particle list is sorted
			}
		}
	}
}

void Simulation::CreateRandomParticles() {
	//Setting up the seed / random number generator
	if (seed == NULL) {
		//No seed given. Generate a pseudo-random seed based on system time
		std::chrono::system_clock::duration timeDuration = std::chrono::system_clock::now() - std::chrono::system_clock::time_point();
		seed = (unsigned int)timeDuration.count();
	}
	std::default_random_engine rndGen(seed);
	std::uniform_real_distribution<double> UniDistr(0.0f, 1.0f);
	auto Random01 = std::bind(UniDistr, rndGen);

	//Now actually randomize the particles
	double AP, AN, I, MA, EA, a, eps, m, s, r_av;
	Particle p;
	double min_a_sq = std::pow(min_a, 2);
	double max_a_sq = std::pow(max_a, 2);
	double min_a_3 = std::pow(min_a, 3);
	double max_a_3 = std::pow(max_a, 3);
	double minCosI = std::cos(max_I);

	ParticleList.clear();
	for (int looper_p = 0; looper_p < N; looper_p++)
	{
		AP = 2 * PI * Random01();
		AN = 2 * PI * Random01();
		I = std::acos(minCosI + (1 - minCosI) * Random01());
		MA = 2 * PI * Random01();

		r_av = std::pow((max_a_3 - min_a_3) * Random01() + min_a_3, 1.0 / 3.0);
		eps = max_eps * Random01();
		a = r_av / (1 + 0.5 * std::pow(eps, 2));
		m = this->m;
		s = this->s;

		EA = Particle::EAFromMA(MA, eps, 100);

		p = Particle::ParticleFromKepler_Eccentric(AP, AN, I, EA, a, eps, m, s, 0.0f, GM);
		ParticleList.push_back(p); //add particle to list
	}
}

void Simulation::Initialize_HighEpsilonTest() {
	Particle p;
	for (int i = 0; i < 4; i++)
	{
		//Not all combinations of vectors are allowed. But if we choose them all orthogonal it is easily asserted that it works.
		p = Particle::ParticleFromLER(glm::dvec3(0, 0, 1), glm::dvec3(0.5*i, 0, 0), glm::dvec3(1/(1 + 0.5*i), 0, 0), m, s, 0, GM);
		ParticleList.push_back(p);
	}
}

void Simulation::PairTest() {
	//Start the timer
	t_init_start = std::chrono::high_resolution_clock::now();

	//Values dependent on parameters
	GM = G * M;

	//Fill the list with random particles
	CreateRandomParticles();

	//Sort by periapsis
	ParticleList.sort(Particle::ComparePeriapsis);

	//Instead of computing full CollisionList, as a full initialization would, we just count the number of pairs
	CountPairs(&ParticleList);

	t_init_finish = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double, std::ratio<1, 1>> seconds = t_init_finish - t_init_start;
	//Write initialization time into log
	AppendSimHeader(seconds.count());
	*fw << CONTENT_DLMT;
	//write initial distribution
	AppendParticleData();
}

void Simulation::Initialize() {
#if INIT_MSG
	std::cout << "[Initialization] starting\n";
#endif

	//Start the timer
	t_init_start = std::chrono::high_resolution_clock::now();

	//Values dependent on parameters
	GM = G * M;

	//Fill the list with random particles
	CreateRandomParticles();
	
#if INIT_MSG
	std::cout << "[Initialization] Created all particles\n";
#endif

	//Sort by periapsis
	ParticleList.sort(Particle::ComparePeriapsis);

#if INIT_MSG
	std::cout << "[Initialization] Sorted particle list\n";
#endif

	//Consider all particle pairs and list the pairs that are on orbits that lead to a collision
	//If they will collide, store pair and collision time, and we may store data of new particle

	SortedFillColList(&ParticleList, &CollisionList);

#if INIT_MSG
	std::cout << "[Initialization] Calculated collisions\n";
#endif

	//Sort collision list
	CollisionList.sort(Collision::CompareCollisionTime);

#if INIT_MSG
	std::cout << "[Initialization] Sorted Collision list\n";
#endif

	t_init_finish = std::chrono::high_resolution_clock::now();
#if INIT_MSG
	std::cout << "[Initialization] Done\n";
#endif

	std::chrono::duration<double, std::ratio<1, 1>> seconds = t_init_finish - t_init_start;
	//Write initialization time into log
	AppendSimHeader(seconds.count());
	*fw << CONTENT_DLMT;
	//write initial distribution
	AppendParticleData();

#if GRAPHICAL and VISUAL_MODE
	//Rest CA Navigator
	ResetCANavigator();
#endif
}

//Main loop
void Simulation::Step() {
	
	//[1] if collision list is empty, end simulation
	if (CollisionList.empty()) {
		//End simulation
		return;
	}

	//[2] Take the pair (i, j) with the soonest collision: The first in the list
	Collision soonestCol = CollisionList.front();

	//[3] Update the time to the time of collision
#if GRAPHICAL
	ViewTime = soonestCol.time;
#endif

	//[4] Remove from the pairs list any pair containing i and any pair containing j
	for (std::list<Collision>::iterator i = CollisionList.begin(); i != CollisionList.end();)
	{
		Collision col = *i;
		if (col.first == soonestCol.first ||
			col.first == soonestCol.second ||
			col.second == soonestCol.first ||
			col.second == soonestCol.second) {
			//Currently checked pair contains 1 or 2 of the soonest colliding particles. Remove from list
			i = CollisionList.erase(i);
		}
		else {
			i++;
		}
	}

	//[6] If the orbit of the new particle intersects the central mass or is unbound, go to the next collision on the list
	Particle p = Collision::SimpleMerge(soonestCol, GM, S);
	if (p.r_0 == glm::dvec3(0, 0, 0)) {
		//Particle collided with central body or is invalid. No particle will be added this simulation step
	}
	else {
		//[7] Create new particle(s) from collision
		NewParticleList.push_back(p);
	}

	//[5] Remove from the particle list the particles i and j
	for (std::list<Particle>::iterator i = ParticleList.begin(); i != ParticleList.end();)
	{
		if (&(*i) == soonestCol.first || &(*i) == soonestCol.second) {
			i = ParticleList.erase(i);
		}
		else {
			i++;
		}
	}

	//[8] For any new particle, consider the other particles and decide if the pair is on collision course.
	//If this is the case, calculate the time of earliest collision
	//Calculate new collisions of new particles with old ones
	CrossFillColList(&NewParticleList, &ParticleList, &NewCollisionList);

	//Calculate new collisions of new particles with other new particles
	FillColList(&NewParticleList, &NewCollisionList);

	//[9] Make sorted list of new collision possibilities
	NewCollisionList.sort(Collision::CompareCollisionTime);

	//[10] Merge new collision list with existing one.
	if (NewCollisionList.empty() == false) {
		CollisionList.merge(NewCollisionList, Collision::CompareCollisionTime);
	}

	//Merge new particle list into existing particle list
	if (NewParticleList.empty() == false) {
		colNewParticle = &NewParticleList.front();
		ParticleList.merge(NewParticleList, Particle::ComparePeriapsis);
	}

#if GRAPHICAL and VISUAL_MODE
	//Rest CA Navigator
	ResetCANavigator();

	//Update Kepler vertices for visualization
	if (focusCol == 1) {
		focusCol++;
	}
#endif
}

void Simulation::RunSim() {
	std::chrono::steady_clock::time_point t_start = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::ratio<1, 1>> seconds = std::chrono::high_resolution_clock::now() - t_start;

	while (CollisionList.size() > 0) {
		//Record frame
		*fw << CONTENT_DLMT;
		AppendCollisionData(seconds.count());
		//next step
		t_start = std::chrono::high_resolution_clock::now();
		Step();
		seconds = std::chrono::high_resolution_clock::now() - t_start;
		//Show progress for big simulations
		if (N > 1e3) {
			if (ParticleList.size() % (N / 100) == 0) {
				std::cout << "[Sim Runner] Remaining particles: " << ParticleList.size() << ".\n";
			}
		}
	}
	*fw << CONTENT_DLMT;
	AppendParticleData();
}

#if GRAPHICAL
void Simulation::UpdateKeplerVertices() {
	delete[] KeplerVertices;
	KeplerVertices = new GLfloat[(int) (ParticleList.size() * 12)];
	int counter = 0;
	for (std::list<Particle>::iterator i = ParticleList.begin(); i != ParticleList.end(); i++)
	{
#if VISUAL_MODE
		switch (focusCol) {
		case 1:
			if (!(&*i == CurrentClosestApproach.first || &*i == CurrentClosestApproach.second)) {
				continue;
			}
			break;
		case 2:
			if (!(&*i == colNewParticle)) {
				continue;
			}
			break;
		}
#endif

		//semi-major axis
		KeplerVertices[counter++] = (GLfloat) i->m;
		//Eccentricity vector
		KeplerVertices[counter++] = (GLfloat) i->eps.x;
		KeplerVertices[counter++] = (GLfloat) i->eps.y;
		KeplerVertices[counter++] = (GLfloat) i->eps.z;
		//Angular momentum vector
		KeplerVertices[counter++] = (GLfloat) i->L.x;
		KeplerVertices[counter++] = (GLfloat) i->L.y;
		KeplerVertices[counter++] = (GLfloat) i->L.z;
		//r_0
		KeplerVertices[counter++] = (GLfloat)i->r_0.x;
		KeplerVertices[counter++] = (GLfloat)i->r_0.y;
		KeplerVertices[counter++] = (GLfloat)i->r_0.z;
		//t_0
		KeplerVertices[counter++] = (GLfloat)i->t_0;
		//phase from point of birth part of mean anomaly added to the "zero" mean anomaly
		KeplerVertices[counter++] = (GLfloat) std::fmod(i->omega * (ViewTime - i->t_0), 2 * PI);
	}
}

#if VISUAL_MODE
void Simulation::ResetCANavigator() {
	ColIterator = CollisionList.begin();
	if (CollisionList.empty() == false) {
		CurrentClosestApproach = CollisionList.front().CA;
	}
}
#endif
#endif

void Simulation::AppendParticleData() {
	std::list<Particle>::iterator i;
	//Each particle has 9 stored components
	//For each component we stream a list of the values of that component for each particle
	for (i = ParticleList.begin(); i != --ParticleList.end(); i++)
	{
		*fw << i->t_0 << VALUE_DLMT;
	}
	*fw << (--ParticleList.end())->t_0 << ARR_DLMT;

	for (i = ParticleList.begin(); i != --ParticleList.end(); i++)
	{
		*fw << i->a << VALUE_DLMT;
	}
	*fw << (--ParticleList.end())->a << ARR_DLMT;

	for (i = ParticleList.begin(); i != --ParticleList.end(); i++)
	{
		*fw << i->c << VALUE_DLMT;
	}
	*fw << (--ParticleList.end())->c << ARR_DLMT;

	for (i = ParticleList.begin(); i != --ParticleList.end(); i++)
	{
		*fw << i->s << VALUE_DLMT;
	}
	*fw << (--ParticleList.end())->s << ARR_DLMT;

	for (i = ParticleList.begin(); i != --ParticleList.end(); i++)
	{
		*fw << i->m << VALUE_DLMT;
	}
	*fw << (--ParticleList.end())->m << ARR_DLMT;

	for (i = ParticleList.begin(); i != --ParticleList.end(); i++)
	{
		*fw << i->r_0.x << VALUE_DLMT;
	}
	*fw << (--ParticleList.end())->r_0.x << ARR_DLMT;

	for (i = ParticleList.begin(); i != --ParticleList.end(); i++)
	{
		*fw << i->r_0.y << VALUE_DLMT;
	}
	*fw << (--ParticleList.end())->r_0.y << ARR_DLMT;

	for (i = ParticleList.begin(); i != --ParticleList.end(); i++)
	{
		*fw << i->r_0.z << VALUE_DLMT;
	}
	*fw << (--ParticleList.end())->r_0.z << ARR_DLMT;

	for (i = ParticleList.begin(); i != --ParticleList.end(); i++)
	{
		*fw << i->L.x << VALUE_DLMT;
	}
	*fw << (--ParticleList.end())->L.x << ARR_DLMT;

	for (i = ParticleList.begin(); i != --ParticleList.end(); i++)
	{
		*fw << i->L.y << VALUE_DLMT;
	}
	*fw << (--ParticleList.end())->L.y << ARR_DLMT;

	for (i = ParticleList.begin(); i != --ParticleList.end(); i++)
	{
		*fw << i->L.z << VALUE_DLMT;
	}
	*fw << (--ParticleList.end())->L.z << ARR_DLMT;

	for (i = ParticleList.begin(); i != --ParticleList.end(); i++)
	{
		*fw << i->eps.x << VALUE_DLMT;
	}
	*fw << (--ParticleList.end())->eps.x << ARR_DLMT;

	for (i = ParticleList.begin(); i != --ParticleList.end(); i++)
	{
		*fw << i->eps.y << VALUE_DLMT;
	}
	*fw << (--ParticleList.end())->eps.y << ARR_DLMT;

	for (i = ParticleList.begin(); i != --ParticleList.end(); i++)
	{
		*fw << i->eps.z << VALUE_DLMT;
	}
	*fw << (--ParticleList.end())->eps.z << ARR_DLMT;

	for (i = ParticleList.begin(); i != --ParticleList.end(); i++)
	{
		*fw << i->omega << VALUE_DLMT;
	}
	*fw << (--ParticleList.end())->omega;
}

void Simulation::AppendCollisionData(double runTime) {
	if (CollisionList.size() > 0) {
		*fw << ParticleList.size() << VALUE_DLMT << CollisionList.size() << VALUE_DLMT << CollisionList.front().time << VALUE_DLMT;
		glm::dvec3 r_0 = (CollisionList.front().first->m * CollisionList.front().r_first + CollisionList.front().second->m * CollisionList.front().r_second)
			/ (CollisionList.front().first->m + CollisionList.front().second->m);
		*fw << r_0.x << VALUE_DLMT << r_0.y << VALUE_DLMT << r_0.z;
	}
	else {
		//Not even 1 collision. Print a 'zero' collision line
		*fw << ParticleList.size() << VALUE_DLMT << 0 << VALUE_DLMT << INFINITY << VALUE_DLMT;
		*fw << 0 << VALUE_DLMT << 0 << VALUE_DLMT << 0;
	}
	*fw << VALUE_DLMT << runTime;
}

void Simulation::AppendSimHeader(double initTime) {
	*fw << GM << VALUE_DLMT << S << VALUE_DLMT << M << VALUE_DLMT << max_I << VALUE_DLMT << min_a << VALUE_DLMT;
	*fw << max_a << VALUE_DLMT << max_eps << VALUE_DLMT << m << VALUE_DLMT << s << VALUE_DLMT << T_sim << VALUE_DLMT;
	*fw << max_iterations << VALUE_DLMT << initTime << VALUE_DLMT << checks << VALUE_DLMT << seed << VALUE_DLMT;
	*fw << N << VALUE_DLMT << pairs << VALUE_DLMT << pairsInTime;
}

void Simulation::SetParameters(SimParameters pam) {
	//Parameters
	S = pam.S; //AU
	M = pam.M; //Sunmasses
	//planetesimal disk
	N = pam.N;
	max_I = pam.max_I; //Radians
	min_a = pam.min_a; //AU
	max_a = pam.max_a; //AU
	max_eps = pam.max_eps; //Dimensionless
	m = pam.m; //Sunmasses
	s = pam.s; //AU
}