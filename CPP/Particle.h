#pragma once

#include<cmath>
#include<iostream>
#include<string>
#include<glm/gtx/string_cast.hpp>
#include<glm/glm.hpp>
const double PI = 3.1415926535897932384626433832795;

struct Particle {
public:
	//Everything is in standard units unless specified otherwise
	double t_0; //Creation time
	double a; //Orbital radius
	double c; //Focal distance
	double s; //Particle radius
	double m; //Particle mass
	glm::dvec3 r_0; //Particle creation point
	glm::dvec3 L; //Angular momentum dvector
	glm::dvec3 eps; //Eccentricity dvector
	double omega; //Frequency

	static bool ComparePeriapsis(Particle first, Particle second); //Returns true if periapsis of first is smaller

	static bool SortedRangeOverlap(Particle small_periapsis, Particle large_periapsis); //Returns if range overlaps

	static glm::dmat3x3 KeplerRotationMatrix(double AP, double AN, double I); //Computes the Kepler rotation matrix

	static Particle ParticleFromKepler(double AP, double AN, double I, double TA, double a, double eps, double m, double s, double t_0, double GM); //Creates and returns particle with specified Kepler coordinates

	static Particle ParticleFromKepler_Eccentric(double AP, double AN, double I, double EA, double a, double eps, double m, double s, double t_0, double GM);

	static Particle ParticleFromCartesian(glm::dvec3 r, glm::dvec3 v, double m, double s, double t_0, double GM); //Creates and returns particle with specified Cartesian coordinates

	static Particle ParticleFromLER(glm::dvec3 L, glm::dvec3 eps, glm::dvec3 r_0, double m, double s, double t_0, double GM); //Creates particle with orbit defined by L and epslion dvectors

	static double EAFromMA(double MA, double eps, int iterations); //Calculates eccentric anomaly from mean anomaly numerically (no exact formula)

	//Equation B.1
	glm::dvec3 VelocityFromR(glm::dvec3 r, double GM); //Returns velocity, using particle data and r

	double Parameter_l(double GM); //Returns parameter l

	glm::dvec3 EpsFromVLR(glm::dvec3 v, glm::dvec3 r, double GM); //returns eccentricity dvector using v and r

	glm::dvec3 PositionFromTA(double TA, double GM); //Calculates position of particle given the true anomaly

	double TAFromPosition(glm::dvec3 r); //Calculates true anomaly of the particle based on the given position and its own particle data

	double SphereOfInfluence(double M); //Calculates sphere of influence

	double deltaE(glm::dvec3 r_CA, glm::dvec3 v_CA); //Returns difference in eccentric anomaly from particle to crossing

	double EccFromPosition(glm::dvec3 r); //Returns eccentric anomaly of the particle assuming it is on the given position

	std::string String(); //Returns string with particle info
};