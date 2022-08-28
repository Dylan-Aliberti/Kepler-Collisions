#pragma once
#include<cmath>
#include<iostream>
#include<string>
#include<sstream>
#include<list>
#include<glm/gtx/string_cast.hpp>

#include"Particle.h"
#include<glad/glad.h>
#include"GeneralSettings.h"

struct ClosestApproach {
public:
	Particle* first = NULL; //first particle of pair
	Particle* second = NULL; //second particle of pair

	glm::dvec3 r1_node = glm::dvec3(0.0, 0.0, 0.0);
	glm::dvec3 r2_node = glm::dvec3(0.0, 0.0, 0.0);

	glm::dvec3 r1 = glm::dvec3(0.0, 0.0, 0.0); //Position of first particle at closest approach
	glm::dvec3 r2 = glm::dvec3(0.0, 0.0, 0.0); //" second
	glm::dvec3 v1 = glm::dvec3(0.0, 0.0, 0.0); //Velocity of first particle at closest approach
	glm::dvec3 v2 = glm::dvec3(0.0, 0.0, 0.0); //Velocity of first particle at closest approach

#if GRAPHICAL and VISUAL_MODE
	//std::list<glm::dvec3> r_list; //List containing all OP-iterations for showing the arrows
	//GLfloat* arrowVAO = NULL;
	//GLfloat* sectionVAO = NULL;
#endif

	int possibleInteraction = 0; //0 = no interaction, 1 = influence, 2 = collision
	double d = 0; //Closest distance
	double w_sq = 0; //Square of the w vector (which is cross products of the two velocities)

	//Earliest passage times
	double deltaE_1 = 0;
	double deltaE_2 = 0;
	double t1 = 0;
	double t2 = 0;
	double reciDiff = 0;
	double delta = 0;

	//Collision time
	double colTime = INFINITY;

	std::string String(); //Returns string with node info

	void WriteVAO(GLfloat* arrows, GLfloat* section); //Writes VAO to given arrays so that the CA object can be rendered

	static void CalcClosestApproach(Particle* first, Particle* second, ClosestApproach* target, double GM, double M, bool side);

	static bool CompareColTime(ClosestApproach first, ClosestApproach second);
};

