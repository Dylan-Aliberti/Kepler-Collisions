#pragma once
#include<cmath>
#include<iostream>
#include<string>
#include<sstream>
#include<glm/gtx/string_cast.hpp>

#include"Particle.h"
#include"ClosestApproach.h"
#include"GeneralSettings.h"

struct Collision {
public:
	Particle* first; //first particle of pair
	Particle* second; //second particle of pair
	double time; //time of collision
	glm::dvec3 r_first; //Center of first particle when colliding
	glm::dvec3 r_second; //Center of second particle when colliding

#if GRAPHICAL and VISUAL_MODE
	ClosestApproach CA; //Closest approach object containing full calculation details of the Collision
#endif

	Collision(Particle* first, Particle* second, double time, glm::dvec3 r_first, glm::dvec3 r_second) {
		this->first = first;
		this->second = second;
		this->time = time;
		this->r_first = r_first;
		this->r_second = r_second;
	}

	Collision() {
		first = NULL;
		second = NULL;
		time = INFINITY;
		r_first = glm::dvec3(NAN, NAN, NAN);
		r_second = glm::dvec3(NAN, NAN, NAN);
	}

	static bool CompareCollisionTime(Collision col1, Collision col2); //Returns true if col1 happens before col2

	static Particle SimpleMerge(Collision col, double GM, double S); //Returns result of simple merger

	static void CalcEarliestPassageTimes(ClosestApproach* CA, double GM); //Calculates earliest passate time of the particle to the crossing and stores it in CA

	static void CalcParameter_delta(ClosestApproach* CA); //Calculates the dimensionless parameter and stores it in CA

	//Calculating the time until the first collision between the two planets of the current pair/node being calculated:
	static void CalcCollisionTime_XSearch(ClosestApproach* CA, int max_iterations, double max_colTime); //Improved continued fractions Visser & Schouten

	static Collision ColFromCA(ClosestApproach* CA); //Returns Collision object with data from the ClosestApproach object


	std::string String(); //Returns string with collision info
};