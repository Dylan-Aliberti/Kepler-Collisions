#include<iostream>
#include<iterator>
#include<string>

#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

#include"shaderClass.h"
#include"VAO.h"
#include"VBO.h"
#include"EBO.h"
#include"Camera.h"

#include"Simulation.h"
#include"TestProcedures.h"
#include"GeneralSettings.h"

#if GRAPHICAL
int width = 1024;
int height = 1024;

bool orthoView = false;
bool viewAllOrbits = false;
bool scrnshotMode = false;

bool viewAllOrbitsClick = false;
bool scrnClick = false;

//Time travel stuff
double timeTravelSpeed = 1.0; //sim seconds per real-life second
bool jumpToCCA = false;
double AnimationStartTime = 0.0;
double AnimationEndTime = 0.0;
double AnimationPlaySpeed = 1.0; //ratio per RL seconds
double AnimationParameter = 0.0; //on a scale from 0 to 1 how far is the animation

//CA Navigation (visual mode)
bool leftArrowClick = false;
bool rightArrowClick = false;
bool onlyCCAClick = false;
bool CCAClick = false;

//Simulation control
bool stepClick = false;

//RL time and frames
int currentFPScount = 0;
double latestTime = 0;
double currentTime = 0;
double deltaTime = 1;

//render flags
bool renderCCA = true;
bool renderOrbits = false;
bool renderParticles = true;

GLfloat centralBodyColor_default[] = { 1, 1, 0 };
GLfloat centralBodyColor_scrnshotMode[] = { 0, 0, 1 };
GLfloat particleColor[] = { 1, 0, 0 };
GLfloat orbitColor_primary_default[] = { 0, 0, 1 };
GLfloat orbitColor_secondary_default[] = { 0, 1, 0 };
GLfloat orbitColor_primary_scrnshotMode[] = { 0, 0, 0 };
GLfloat orbitColor_secondary_scrnshotMode[] = { 0, 0, 0 };

GLfloat centralBody[] = { 0, 0, 0 };
GLfloat origin[] = { 0, 0, 0 }; //origin
GLfloat* lineArray;

//For visualizing Closest Approach
GLfloat CA_arrows[12];
GLfloat CA_section[22]; //mel, r0, deltaE, and that for both particles

Camera* cam;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	(*cam).width = width;
	(*cam).height = height;
}

void ControllerInput(GLFWwindow* window, Simulation* sim) {
	//Next step in simulation
	if (glfwGetKey(window, SIM_STEP) == GLFW_PRESS) {
		if (!stepClick) {
			stepClick = true;
			sim->Step();
#if VISUAL_MODE
			renderCCA = false;
#endif
		}
	}
	else if (glfwGetKey(window, SIM_STEP) == GLFW_RELEASE) {
		stepClick = false;
	}

	//Freely moving through time
	if (glfwGetKey(window, SIM_TIME_FORWARD) == GLFW_PRESS) {
		double firstTime = sim->ViewTime;
		sim->ViewTime += timeTravelSpeed * deltaTime;
		std::cout << "viewing time: " << sim->ViewTime << "\n";
	}
	if (glfwGetKey(window, SIM_TIME_BACKWARD) == GLFW_PRESS) {
		sim->ViewTime -= timeTravelSpeed * deltaTime;
		std::cout << "viewing time: " << sim->ViewTime << "\n";
	}

	//Jumping to time of collision (if it exists)
	if (glfwGetKey(window, SIM_TIME_JUMP) == GLFW_PRESS) {
		if (sim->CollisionList.empty() == false) {
#if VISUAL_MODE
			if (std::abs(sim->CurrentClosestApproach.colTime) < INFINITY) {
#else
			if (std::abs(sim->CollisionList.front().time) < INFINITY) {
#endif
				jumpToCCA = true;
				AnimationStartTime = sim->ViewTime;
#if VISUAL_MODE
				AnimationEndTime = sim->CurrentClosestApproach.colTime;
#else
				AnimationEndTime = sim->CollisionList.front().time;
#endif
				AnimationParameter = 0.0;
			}
		}
	}

#if VISUAL_MODE
	//Inspect CA of earliest collision
	if (glfwGetKey(window, SIM_SHOW_CCA) == GLFW_PRESS) {
		if (!CCAClick) {
			CCAClick = true;
			if (sim->CollisionList.empty() == false) {
				sim->CurrentClosestApproach = sim->CollisionList.front().CA;
				std::cout << "viewing time: " << sim->ViewTime << "\n";
				std::cout << "Currently viewing CA: \n" << sim->CurrentClosestApproach.String() << "\n";
			}
		}
	}else if(glfwGetKey(window, SIM_SHOW_CCA) == GLFW_RELEASE) {
		CCAClick = false;
	}

	//Navigating CA objects
	if (glfwGetKey(window, SIM_SHOW_PREVIOUS_CA) == GLFW_PRESS) {
		if (!leftArrowClick) {
			leftArrowClick = true;
			if (sim->ColIterator != sim->CollisionList.begin()) {
				sim->ColIterator--;
				sim->CurrentClosestApproach = sim->ColIterator->CA;
				std::cout << "Currently viewing CA: \n" << sim->CurrentClosestApproach.String() << "\n";
			}
		}
	}
	else if (glfwGetKey(window, SIM_SHOW_PREVIOUS_CA) == GLFW_RELEASE) {
		leftArrowClick = false;
	}

	if (glfwGetKey(window, SIM_SHOW_NEXT_CA) == GLFW_PRESS) {
		if (!rightArrowClick) {
			rightArrowClick = true;
			if (sim->CollisionList.empty() == false) {
				if (sim->ColIterator != --sim->CollisionList.end()) {
					sim->ColIterator++;
					sim->CurrentClosestApproach = sim->ColIterator->CA;
					std::cout << "Currently viewing CA: \n" << sim->CurrentClosestApproach.String() << "\n";
				}
			}
		}
	}
	else if (glfwGetKey(window, SIM_SHOW_NEXT_CA) == GLFW_RELEASE) {
		rightArrowClick = false;
	}

	//Switch view for looking only at colliding particles or at everything
	if (glfwGetKey(window, SIM_SHOW_ONLY_CCA) == GLFW_PRESS) {
		if (!onlyCCAClick) {
			onlyCCAClick = true;
			switch (sim->focusCol) {
			case 0:
				sim->focusCol = 1;
				break;
			case 1:
				sim->focusCol = 0;
				break;
			case 2:
				sim->focusCol = 1;
				renderCCA = true;
			}
			sim->UpdateKeplerVertices();
		}
	}
	else if (glfwGetKey(window, SIM_SHOW_ONLY_CCA) == GLFW_RELEASE) {
		onlyCCAClick = false;
	}
#endif

	if (glfwGetKey(window, SIM_VIEW_ALL_ORBITS) == GLFW_PRESS) {
		if (!viewAllOrbitsClick) {
			viewAllOrbitsClick = true;
			viewAllOrbits = !viewAllOrbits;
		}
	}
	else if (glfwGetKey(window, SIM_VIEW_ALL_ORBITS) == GLFW_RELEASE) {
		viewAllOrbitsClick = false;
	}

	//Screenshot
	if (glfwGetKey(window, SIM_SCHEMATIC) == GLFW_PRESS) {
		if (!scrnClick) {
			scrnClick = true;
			scrnshotMode = !scrnshotMode;
		}
	}
	else if (glfwGetKey(window, SIM_SCHEMATIC) == GLFW_RELEASE) {
		scrnClick = false;
	}
}

double Lerp(double a, double b, double t)
{
	return a + t * (b - a);
}

double SinusLerp(double a, double b, double t) {
	return a + std::sin(PI * t / 2) * (b - a);
}

double SqrtLerp(double a, double b, double t) {
	return a + std::sqrt(-std::pow(t, 2) + 2 * t) * (b - a);
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	(*cam).OnScroll((float) yoffset);
}

#endif

std::ofstream fw; //File to stream simulation data to
//std::ofstream fw_B;
//std::ofstream fw_C;
//std::ofstream fw_D;
//std::ofstream fw_E;
//std::ofstream fw_F;

int main() {
#if GRAPHICAL
	//Initialize GLFW
	glfwInit();

	//Tell GLFW we are using OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//Tell GLFW we only use the core profile (only modern functions)
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//Create window
	GLFWwindow* window = glfwCreateWindow(width, height, "BEP Collisions", NULL, NULL);
	//error check if window fails to create
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	//Introduce the window to the current context
	glfwMakeContextCurrent(window);

	//Load GLAD so it configures OpenGL
	gladLoadGL();

	//Specify viewport of OpenGL in window
	glViewport(0, 0, width, height);

	//Graphic settings
	glEnable(GL_DEPTH_TEST);
	glLineWidth(5);
	glEnable(GL_LINE_SMOOTH);
	glPointSize(20);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	Shader orbitDrawer("orbit.vert", "orbit.frag", "orbit.geo");
	Shader centralBodyDrawer("centralBody.vert", "centralBody.frag");
	Shader compassDrawer("compass.vert", "compass.frag", "compass.geo");
	Shader particleDrawer("particle.vert", "particle.frag");
#if VISUAL_MODE
	Shader arrowDrawer("arrow.vert", "line.frag", "arrow.geo");
	Shader sectionDrawer("section.vert", "section.frag", "section.geo");
#endif

	VAO bodyVAO;
	bodyVAO.Bind();
	VBO bodyVBO(centralBody, sizeof(centralBody));
	bodyVAO.LinkAttrib(bodyVBO, 0, 3, GL_FLOAT, 3 * sizeof(float), (void*)0);
	bodyVAO.Unbind();
	bodyVBO.Unbind();

	VAO gridVAO;
	gridVAO.Bind();
	VBO gridVBO(origin, sizeof(origin));
	gridVAO.LinkAttrib(gridVBO, 0, 3, GL_FLOAT, 3 * sizeof(float), (void*)0);
	gridVAO.Unbind();
	gridVBO.Unbind();

	Camera camera(width, height, glm::vec3(-3.0f, 0.0f, 2.0f));
	camera.Orientation = glm::vec3(1 / sqrtf(3), 0.0f, -0.5f);
	cam = &camera;
	glfwSetScrollCallback(window, scrollCallback);
#endif

	//Open file to write simulation data to
	fw.open("C:\\Active Projects\\simData.txt");
	fw.precision(16);
	//fw_B.open("C:\\Active Projects\\simData_B4_part2.txt");
	//fw_B.precision(16);
	//fw_C.open("C:\\Active Projects\\simData_C2.txt");
	//fw_C.precision(16);
	//fw_D.open("C:\\Active Projects\\simData_D3.txt");
	//fw_D.precision(16);
	//fw_E.open("C:\\Active Projects\\simData_E2.txt");
	//fw_E.precision(16);
	//fw_F.open("C:\\Active Projects\\simData_F1.txt");
	//fw_F.precision(16);

#if GRAPHICAL
	//Start simulation
	Simulation sim;
	sim.max_I = 1e-3;
	sim.max_eps = 0.8;
	sim.N = 1e3;
	sim.fw = &fw;
	sim.Initialize();
#else
	//Test procedures
	//Procedures::ProcedureC(&fw_C);
	//Procedures::ProcedureD(&fw_D);
	//Procedures::ProcedureE(&fw_E);
	//Procedures::ProcedureF(&fw_F);
	//Procedures::ProcedureB(&fw_B);
#endif

#if GRAPHICAL
	//Set up graphic stuff for viewing simulation
	VAO particleVAO;
	particleVAO.Bind();
	//Generates Vertex Buffer Object and links it to vertices
	VBO particleVBO(sim.KeplerVertices, sim.ParticleList.size() * 12 * sizeof(float));
	//Links VBO to VAO
	particleVAO.LinkAttrib(particleVBO, 0, 1, GL_FLOAT, 12 * sizeof(float), (void*)0); //mass
	particleVAO.LinkAttrib(particleVBO, 1, 3, GL_FLOAT, 12 * sizeof(float), (void*)(1 * sizeof(float))); //eps
	particleVAO.LinkAttrib(particleVBO, 2, 3, GL_FLOAT, 12 * sizeof(float), (void*)(4 * sizeof(float))); //L
	particleVAO.LinkAttrib(particleVBO, 3, 3, GL_FLOAT, 12 * sizeof(float), (void*)(7 * sizeof(float))); //r_0
	particleVAO.LinkAttrib(particleVBO, 4, 1, GL_FLOAT, 12 * sizeof(float), (void*)(10 * sizeof(float))); //t_0
	particleVAO.LinkAttrib(particleVBO, 5, 1, GL_FLOAT, 12 * sizeof(float), (void*)(11 * sizeof(float))); //phase shift
	//Unbind all to prevent accidentally modifying them
	particleVAO.Unbind();
	particleVBO.Unbind();

#if VISUAL_MODE
	sim.CurrentClosestApproach.WriteVAO(CA_arrows, CA_section);

	VAO arrowVAO;
	arrowVAO.Bind();
	VBO arrowVBO(CA_arrows, 12 * sizeof(float));
	arrowVAO.LinkAttrib(arrowVBO, 0, 3, GL_FLOAT, 3 * sizeof(float), (void*)0);
	arrowVAO.Unbind();
	arrowVBO.Unbind();

	VAO sectionVAO;
	sectionVAO.Bind();
	VBO sectionVBO(CA_section, 22 * sizeof(float));
	sectionVAO.LinkAttrib(sectionVBO, 0, 1, GL_FLOAT, 11 * sizeof(float), (void*)0); //mass
	sectionVAO.LinkAttrib(sectionVBO, 1, 3, GL_FLOAT, 11 * sizeof(float), (void*)(1 * sizeof(float))); //eps
	sectionVAO.LinkAttrib(sectionVBO, 2, 3, GL_FLOAT, 11 * sizeof(float), (void*)(4 * sizeof(float))); //L
	sectionVAO.LinkAttrib(sectionVBO, 3, 3, GL_FLOAT, 11 * sizeof(float), (void*)(7 * sizeof(float))); //r_0
	sectionVAO.LinkAttrib(sectionVBO, 4, 1, GL_FLOAT, 11 * sizeof(float), (void*)(10 * sizeof(float))); //deltaE
#endif

	//Set value of GM in shaders that use it
	glUniform1f(glGetUniformLocation(orbitDrawer.ID, "GM"), sim.GM);
#if VISUAL_MODE
	glUniform1f(glGetUniformLocation(sectionDrawer.ID, "GM"), sim.GM);
#endif
	glUniform1f(glGetUniformLocation(particleDrawer.ID, "GM"), sim.GM);

	//Main while loop
	while (!glfwWindowShouldClose(window)) {
		//Set delta time
		currentTime = clock() / (double) CLOCKS_PER_SEC; //seconds
		deltaTime = currentTime - latestTime; //delta time in seconds
		latestTime = currentTime;

		//White background
		if (scrnshotMode) {
			glClearColor(1, 1, 1, 1);
		}
		else {
			glClearColor(0, 0, 0, 1);
		}
		//Clean the back buffer and depth buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//Tell OpenGL which Shader Program we want to use

		//Get inputs for camera control
		camera.Inputs(window, (float) deltaTime);
		camera.UpdateMatrix();
		ControllerInput(window, &sim);
		if ((!viewAllOrbits) && (sim.focusCol == 0)) {
			renderOrbits = false;
		}
		else {
			renderOrbits = true;
		}

		//sim.Step();

		//Update VBOs and render flags
#if VISUAL_MODE
		sim.CurrentClosestApproach.WriteVAO(CA_arrows, CA_section);
		if (sim.CollisionList.empty() == false) {
			arrowVBO.Update(CA_arrows, 12 * sizeof(float));
			sectionVBO.Update(CA_section, 22 * sizeof(float));
		}
		else {
			renderCCA = false;
		}
#endif
		sim.UpdateKeplerVertices();
		particleVBO.Update(sim.KeplerVertices, sim.ParticleList.size() * 12 * sizeof(float));

		//No z-test zone
		glDisable(GL_DEPTH_TEST);

#if VISUAL_MODE
		if (renderCCA && renderOrbits) {
			//Draw orbit sections
			sectionDrawer.Activate();
			camera.MatrixToUniform(sectionDrawer, "camMatrix");
			glUniform1f(glGetUniformLocation(sectionDrawer.ID, "GM"), sim.GM);
			sectionVAO.Bind();
			glDrawArrays(GL_POINTS, 0, (GLsizei)2);
			sectionVAO.Unbind();
		}
#endif

		//Set Z-test back
		glEnable(GL_DEPTH_TEST);

		//Draw compass
		if (!scrnshotMode) {
			compassDrawer.Activate();
			glUniform1f(glGetUniformLocation(compassDrawer.ID, "scale"), 1);
			camera.MatrixToUniform(compassDrawer, "camMatrix");
			gridVAO.Bind();
			glDrawArrays(GL_POINTS, 0, 1);
			gridVAO.Unbind();
		}

		//Draw particles
		if (renderParticles) {
			particleDrawer.Activate();
			particleVAO.Bind();
			camera.MatrixToUniform(particleDrawer, "camMatrix");
			glUniform3fv(glGetUniformLocation(particleDrawer.ID, "color"), 1, particleColor);
			glUniform1f(glGetUniformLocation(particleDrawer.ID, "GM"), sim.GM);
			glDrawArrays(GL_POINTS, 0, (GLsizei)sim.ParticleList.size());
			particleVAO.Unbind();
		}

		if (renderOrbits) {
			//Draw orbits
			orbitDrawer.Activate();
			camera.MatrixToUniform(orbitDrawer, "camMatrix");
			glUniform3fv(glGetUniformLocation(orbitDrawer.ID, "camPos"), 1, glm::value_ptr(camera.Position));
			glUniform1f(glGetUniformLocation(orbitDrawer.ID, "GM"), sim.GM);
			if (scrnshotMode) {
				glUniform3fv(glGetUniformLocation(orbitDrawer.ID, "primaryColor"), 1, orbitColor_primary_scrnshotMode);
				glUniform3fv(glGetUniformLocation(orbitDrawer.ID, "secondaryColor"), 1, orbitColor_secondary_scrnshotMode);
			}
			else {
				glUniform3fv(glGetUniformLocation(orbitDrawer.ID, "primaryColor"), 1, orbitColor_primary_default);
				glUniform3fv(glGetUniformLocation(orbitDrawer.ID, "secondaryColor"), 1, orbitColor_secondary_default);
			}
			particleVAO.Bind();
			glDrawArrays(GL_POINTS, 0, (GLsizei)sim.ParticleList.size());
			particleVAO.Unbind();
		}

		//No z-test zone
		glDisable(GL_DEPTH_TEST);

#if VISUAL_MODE
		if (renderCCA && renderOrbits) {
			//Draw closest approach arrows
			arrowDrawer.Activate();
			camera.MatrixToUniform(arrowDrawer, "camMatrix");
			glUniform3fv(glGetUniformLocation(arrowDrawer.ID, "camPos"), 1, glm::value_ptr(camera.Position));
			arrowVAO.Bind();
			glDrawArrays(GL_LINES, 0, (GLsizei)4);
			arrowVAO.Unbind();
		}
#endif

		//Draw central body
		centralBodyDrawer.Activate();
		camera.MatrixToUniform(centralBodyDrawer, "camMatrix");
		if (scrnshotMode) {
			glPointSize(20);
			glUniform3fv(glGetUniformLocation(centralBodyDrawer.ID, "color"), 1, centralBodyColor_scrnshotMode);
			glUniform1i(glGetUniformLocation(centralBodyDrawer.ID, "schematicMode"), true);
		}
		else {
			glPointSize(100);
			glUniform3fv(glGetUniformLocation(centralBodyDrawer.ID, "color"), 1, centralBodyColor_default);
			glUniform1i(glGetUniformLocation(centralBodyDrawer.ID, "schematicMode"), false);
		}
		bodyVAO.Bind();
		glDrawArrays(GL_POINTS, 0, 1);
		bodyVAO.Unbind();
		glPointSize(20);

		//Set Z-test back
		glEnable(GL_DEPTH_TEST);

		//Animation
		if (jumpToCCA) {

			if (AnimationParameter < 1.0f) {
				AnimationParameter += AnimationPlaySpeed * deltaTime;
				sim.ViewTime = SqrtLerp(AnimationStartTime, AnimationEndTime, AnimationParameter);
			}
		}

		glfwSwapBuffers(window);
		
		//Take care of all GLFW events
		glfwPollEvents();
		glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	}

#endif

	//fw.close();
	//fw_B.close();
	//fw_C.close();
	//fw_D.close();
	//fw_E.close();
	//fw_F.close();

#if GRAPHICAL
	//Delete all the objects we've created
	particleVAO.Delete();
	particleVBO.Delete();
	bodyVAO.Delete();
	bodyVBO.Delete();
	orbitDrawer.Delete();
	centralBodyDrawer.Delete();
#if VISUAL_MODE
	arrowDrawer.Delete();
	arrowVAO.Delete();
	arrowVBO.Delete();
	sectionDrawer.Delete();
	sectionVAO.Delete();
	sectionVBO.Delete();
#endif

	//Delete window before ending program
	glfwDestroyWindow(window);
	//Terminate GLFW before ending program
	glfwTerminate();
#endif
	return 0;
}