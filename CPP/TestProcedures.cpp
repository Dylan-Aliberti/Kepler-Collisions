#include"TestProcedures.h"
#include"Delimiters.h"

//Setting up the seed / random number generator
std::chrono::system_clock::duration timeDuration = std::chrono::system_clock::now() - std::chrono::system_clock::time_point();
unsigned int seed = (unsigned int)timeDuration.count();
std::default_random_engine rndGen(seed);
std::uniform_real_distribution<double> UniDistr(0.0f, 1.0f);
auto Random01 = std::bind(UniDistr, rndGen);

void Procedures::ProcedureA(std::ofstream* fw) {
	SimParameters pam;
	pam.max_I = 1e-3;
	pam.max_eps = 1e-3;
	pam.s = pam.max_a * 1e-5;
	for (int i = 0; i < NUM_SIMS; i++)
	{
		pam.N = (unsigned int) std::pow(10, 5.0 + Random01());
		Simulation sim;
		sim.fw = fw;
		sim.SetParameters(pam);
		sim.PairTest();
		if (i < NUM_SIMS - 1) {
			*fw << SIM_DLMT;
		}
		if ((i % MESSAGE_INTERVAL) == 0) {
			std::cout << "[Procedure A] " << i+1 << "/" << NUM_SIMS << " done\n";
		}
	}
}

void Procedures::ProcedureB(std::ofstream* fw) {
	SimParameters pam;
	pam.max_I = 1e-3;
	pam.max_eps = 1e-3;
	pam.s = pam.max_a * 1e-5;
	for (int i = 0; i < NUM_SIMS; i++)
	{
		pam.N = (unsigned int)std::pow(10, 5.0 * Random01());
		std::cout << "[Procedure B] Next simulation: N = " << pam.N << ".\n";
		Simulation sim;
		sim.fw = fw;
		sim.SetParameters(pam);
		sim.Initialize();
		std::cout << "[Procedure B] Initialization complete.\n";
		sim.RunSim();
		if (i < NUM_SIMS - 1) {
			*fw << SIM_DLMT;
		}
		if ((i % MESSAGE_INTERVAL) == 0) {
			std::cout << "[Procedure B] " << i+1 << "/" << NUM_SIMS << " done\n";
		}
	}
}

void Procedures::ProcedureC(std::ofstream* fw) {
	SimParameters pam;
	pam.max_I = 1e-3;
	pam.N = 1e4;
	pam.s = pam.max_a * 1e-5;
	for (int i = 0; i < NUM_SIMS; i++)
	{
		pam.max_eps = std::pow(10, -5.0 * Random01());
		Simulation sim;
		sim.fw = fw;
		sim.SetParameters(pam);
		sim.PairTest();
		if (i < NUM_SIMS - 1) {
			*fw << SIM_DLMT;
		}
		if ((i % MESSAGE_INTERVAL) == 0) {
			std::cout << "[Procedure C] " << i+1 << "/" << NUM_SIMS << " done\n";
		}
	}
}

void Procedures::ProcedureD(std::ofstream* fw) {
	SimParameters pam;
	pam.max_eps = 1e-3;
	pam.N = 1e4;
	pam.s = pam.max_a * 1e-5;
	for (int i = 0; i < NUM_SIMS; i++)
	{
		pam.max_I = 0.5 * PI * std::pow(10, -7 * Random01()); //Note: before it was -5.0 * Random01()
		Simulation sim;
		sim.fw = fw;
		sim.SetParameters(pam);
		sim.PairTest();
		if (i < NUM_SIMS - 1) {
			*fw << SIM_DLMT;
		}
		if ((i % MESSAGE_INTERVAL) == 0) {
			std::cout << "[Procedure D] " << i + 1 << "/" << NUM_SIMS << " done\n";
		}
	}
}

void Procedures::ProcedureE(std::ofstream* fw) {
	SimParameters pam;
	pam.max_I = 1e-3;
	pam.N = 1e4;
	pam.max_eps = 1e-3;
	for (int i = 0; i < NUM_SIMS; i++)
	{
		pam.s = std::pow(10, -1.0 -5.0 * Random01());
		Simulation sim;
		sim.fw = fw;
		sim.SetParameters(pam);
		sim.PairTest();
		if (i < NUM_SIMS - 1) {
			*fw << SIM_DLMT;
		}
		if ((i % MESSAGE_INTERVAL) == 0) {
			std::cout << "[Procedure E] " << i + 1 << "/" << NUM_SIMS << " done\n";
		}
	}
}

void Procedures::ProcedureF(std::ofstream* fw) {
	SimParameters pam;
	pam.max_I = 1e-3;
	pam.max_eps = 1e-3;
	pam.s = pam.max_a * 1e-3; //increased s
	for (int i = 0; i < NUM_SIMS; i++)
	{
		pam.N = (unsigned int)std::pow(10, 4.0 * Random01());
		std::cout << "[Procedure F] Next simulation: N = " << pam.N << ".\n";
		Simulation sim;
		sim.fw = fw;
		sim.SetParameters(pam);
		sim.Initialize();
		std::cout << "[Procedure F] Initialization complete.\n";
		sim.RunSim();
		if (i < NUM_SIMS - 1) {
			*fw << SIM_DLMT;
		}
		if ((i % MESSAGE_INTERVAL) == 0) {
			std::cout << "[Procedure F] " << i + 1 << "/" << NUM_SIMS << " done\n";
		}
	}
}