#ifndef CAMERA_CLASS_H
#define CAMERA_CLASS_H

#include<string>
#include<algorithm>

#include<glm/gtx/string_cast.hpp>

#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<glm/gtx/rotate_vector.hpp>
#include<glm/gtx/vector_angle.hpp>

#include"shaderClass.h"
#include"GeneralSettings.h"

class Camera {
public:
	glm::vec3 Position;
	glm::vec3 Orientation = glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec3 Up = glm::vec3(0.0f, 0.0f, 1.0f);

	//If this is true the camera won't jump to the mouse position and instead sets this variable false
	bool firstClick = true;
	bool orthoClick = true;
	bool showValuesClick = true;
	bool camLookDownClick = true;
	bool camLookRightClick = true;
	bool camLookDiagonalClick = true;

	bool ortho = false;
	float currentFOV = 1.0f; //radians
	float currentLengthPerPixel = 0.005f; //length units per pixel
	float nearPlane = 0.0001f;
	float farPlane = 1e5f;
	float speedMultSensitivity = 200.0f; //Multiplied by currentLengthPerPixel to obtain speed multiplier

	int width;
	int height;

	float baseSpeed = 0.1f; //Length units per second
	float speed = baseSpeed;
	float sensitivity = 100.0f;
	float scrollFOVSensitivity = 0.01f; //radians per scrolled pixel
	float scrollLengthSensitivity = 0.0001f; //Length units per scrolled pixel
	float speedMultiplier = 1.0f;
	float rotMultiplier = 1.0f;

	glm::mat4 currentMatrix = glm::mat4(1.0f);

	Camera(int width, int height, glm::vec3 position);

	void UpdateMatrix();
	void MatrixToUniform(Shader& shader, const char* uniform);
	void Inputs(GLFWwindow* window, float deltaTime);
	void OnScroll(float offset);
};

#endif