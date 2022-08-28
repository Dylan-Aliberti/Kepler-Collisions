#pragma once
#include<iostream>
#include<list>
#include<algorithm>
#include<random>
#include<functional>
#include<chrono>
#include<string>
#include<fstream>

#include<glad/glad.h>
#include<glm/gtx/string_cast.hpp>

#include"Particle.h"
#include"Collision.h"
#include"ClosestApproach.h"


struct SimParameters {
	//Parameters
	double S = 0.00465; //AU (this is the actual radius of the sun)
	double M = 1; //Sunmasses
	//planetesimal disk
	unsigned int N = 1e3;
	double max_I = 0.1; //Radians
	double min_a = 1; //AU
	double max_a = 2; //AU
	double max_eps = 0.1; //Dimensionless
	double m = 1e-6; //Sunmasses
	double s = 1e-5; //AU
};

class Simulation {
public:
	std::list<Particle> ParticleList; //List of all particles
	std::list<Particle> NewParticleList; //Temporary list of new particles in the current step
	std::list<Collision> CollisionList; //List of all valid possible collisions
	std::list<Collision> NewCollisionList; //Temporary list of new collisions in the current step
#if GRAPHICAL
	GLfloat* KeplerVertices = NULL; //Array of Kepler data of orbits, directly usable for rendering
	double ViewTime = 0; //The time that is currently being viewed
#endif
	ClosestApproach CurrentClosestApproach; //Contains the currently checked closest approach
	double realElapsedTime = 0; //Real-life time passed
	std::ofstream* fw; //File tp stream simulation data to
	Particle* colNewParticle = NULL;
	//Visualization and debugging
#if GRAPHICAL
	unsigned int focusCol = 0; //0 = show all particles, 1 = show particles about to collide, 2 = show result of collision
#if VISUAL_MODE
	std::list<Collision>::iterator ColIterator; //Iterator for navigating through all collision pairs
#endif
#endif

	unsigned int seed; //The seed used while generating random particles. Will be generated based on system time if not specified

	//testing and performance
	std::chrono::steady_clock::time_point t_init_start;
	std::chrono::steady_clock::time_point t_init_finish;
	int checks = 0; //number of pairs that passed the sweep-and-prune and needs to be checked
	int pairs = 0; //Number of collision pairs, not yet cut-off based on time
	int pairsInTime = 0; //Number of collisions that actually ends up in the list (thus "pairs", but cut-off based on time)

	//constant
	const double G = 39.42; //AU^3 * Sunmass^-1 * year^-2
	const int max_iterations = 100; //Maximum amount of iterations that will be done when calculating collision time, before assuming that it doesn't happen

	//Parameters
	double S = 0.00465; //AU (this is the actual radius of the sun)
	double M = 1; //Sunmasses
	double T_sim = 1e6; //simulation time. Collisions after this time are ignored.
	//planetesimal disk
	unsigned int N = 1e3;
	double max_I = 0.1; //Radians
	double min_a = 1; //AU
	double max_a = 2; //AU
	double max_eps = 0.1; //Dimensionless
	double m = 1e-6; //Sunmasses
	double s = 1e-5; //AU

	//Values dependent on parameters
	double GM; //AU^3 / year^2

	void Initialize_HighEpsilonTest();

	void PairTest(); //Does most of the initialization, but does not compute collision times (and therefore doesn't filter late/inf time collisions)

	void Initialize();

	//void CalcCollision(Particle* first, Particle* second); //Calculates if and when 2 particles will collide. Adds collision to list if valid

	void Step();

	void RunSim(); //Runs the whole simulation and does everything needed to properly start and finish

#if GRAPHICAL
	void UpdateKeplerVertices(); //Prepares vertex array of particle to pass to shader for visualization
#if VISUAL_MODE
	void ResetCANavigator(); //Sets ClosestApproach navigator back to first one if it exists
#endif
#endif

	//Excluding sweep-and-prune filter, it calculates possible collisions between 2 given particles at both nodes and writes the result into the given lists
	void CalcCollision(Particle* first, Particle* second, std::list<Collision>* colList);

	//Fills collision list and CA list with collisions that are calculated from the particle list, including a sweep-and-prune filter
	void FillColList(std::list<Particle>* pList, std::list<Collision>* colList);

	//Same as FillColList, but assuming that the particle list is sorted before, so that sweep-and-prune can be used extra efficiently
	void SortedFillColList(std::list<Particle>* pList, std::list<Collision>* colList);

	//Fills collision list with all collisions between particle from pList1 and particle from pList2, including a sweep-and-prune filter
	void CrossFillColList(std::list<Particle>* pList1, std::list<Particle>* pList2, std::list<Collision>* colList);

	void CountPairs(std::list<Particle>* pList); //Used in ColPairTest(). While a full initialization would compute the whole CollisionList
	//this function only counts number of pairs without calculating collision time nor applying corresponding cut-off.

	void AppendParticleData(); //Appends all particle data into the text file

	void AppendCollisionData(double runTime); //Appends collision data and sizes of lists

	void AppendSimHeader(double initTime); //Appends simulation parameters and initialization time

	void SetParameters(SimParameters pam); //Sets parameters from the given object

	void CreateRandomParticles(); //Fills ParticleList with randomly generated particles (based on parameters in the sim object)
};
