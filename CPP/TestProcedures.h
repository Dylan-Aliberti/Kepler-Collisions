#pragma once

#include<iostream>
#include<string>
#include<random>
#include<functional>
#include<chrono>

#include"Simulation.h"

#define NUM_SIMS 14
#define MESSAGE_INTERVAL 1 //Each time this number of simulations is finished the status will be printed in the console

static class Procedures {
public:
	//Varying N, pair test
	static void ProcedureA(std::ofstream* fw);

	//Varying N, run full simulation
	static void ProcedureB(std::ofstream* fw);

	//Varying eps, pair test
	static void ProcedureC(std::ofstream* fw);

	//Varying I, pair test
	static void ProcedureD(std::ofstream* fw);

	//Varying s, pair test
	static void ProcedureE(std::ofstream* fw);

	//Varying N, full simulation, with increased s
	static void ProcedureF(std::ofstream* fw);
};
