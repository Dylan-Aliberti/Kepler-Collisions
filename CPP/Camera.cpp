#include"Camera.h"

template <typename t>
t clamp2(t x, t min, t max)
{
	if (x < min) x = min;
	if (x > max) x = max;
	return x;
}

Camera::Camera(int width, int height, glm::vec3 position) {
	Camera::width = width;
	Camera::height = height;
	Position = position;
}

void Camera::UpdateMatrix() {
	//Initializes matrices since otherwise they will be the null matrix
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);
	//Makes camera look in the right direction from the right position
	view = glm::lookAt(Position, Position + Orientation, Up);

	if (ortho) {
		//Orthogonal view
		float viewWidth = width * this->currentLengthPerPixel;
		float viewHeight = height * this->currentLengthPerPixel;
		projection = glm::ortho(-viewWidth / 2, viewWidth / 2, -viewHeight / 2, viewHeight / 2, nearPlane, farPlane);
	}
	else {
		//Perspective view
		float aspect = (float)width / (float)height;
		projection = glm::perspective(this->currentFOV, aspect, nearPlane, farPlane);
	}

	this->currentMatrix = projection * view;
}

void Camera::MatrixToUniform(Shader& shader, const char* uniform) {
	//Export the camera matrix to the Vertex Shader
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, uniform), 1, GL_FALSE, glm::value_ptr(this->currentMatrix));
}

void Camera::Inputs(GLFWwindow* window, float deltaTime) {
	glm::vec3 forward = glm::normalize(Orientation - glm::dot(Orientation, Up) * Up);
	glm::vec3 right = glm::normalize(glm::cross(Orientation, Up) - glm::dot(glm::cross(Orientation, Up), Up));
	if (glfwGetKey(window, CAM_FORWARD) == GLFW_PRESS) {
		Position += deltaTime * speed * speedMultiplier * forward;
	}
	if (glfwGetKey(window, CAM_LEFT) == GLFW_PRESS) {
		Position += deltaTime * speed * speedMultiplier * -right;
	}
	if (glfwGetKey(window, CAM_BACKWARD) == GLFW_PRESS) {
		Position += deltaTime * speed * speedMultiplier * -forward;
	}
	if (glfwGetKey(window, CAM_RIGHT) == GLFW_PRESS) {
		Position += deltaTime * speed * speedMultiplier * right;
	}
	if (glfwGetKey(window, CAM_UP) == GLFW_PRESS) {
		Position += deltaTime * speed * speedMultiplier * Up;
	}
	if (glfwGetKey(window, CAM_DOWN) == GLFW_PRESS) {
		Position += deltaTime * speed * speedMultiplier * -Up;
	}
	if (glfwGetKey(window, CAM_BOOST) == GLFW_PRESS) {
		speed = baseSpeed * 10;
	}
	else if (glfwGetKey(window, CAM_BOOST) == GLFW_RELEASE) {
		speed = baseSpeed;
	}

	//Projection switch
	if (glfwGetKey(window, CAM_ORTHO) == GLFW_PRESS) {
		if (orthoClick) {
			this->ortho = !this->ortho;
			orthoClick = false;
			//Update multipliers
			if (ortho) {
				speedMultiplier = currentLengthPerPixel * speedMultSensitivity;
				rotMultiplier = 1.0f;
			}
			else {
				speedMultiplier = 1.0f;
				rotMultiplier = currentFOV;
			}
		}
	}
	else if (glfwGetKey(window, CAM_ORTHO) == GLFW_RELEASE) {
		orthoClick = true;
	}

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

		if (firstClick) {
			glfwSetCursorPos(window, (width / 2), (height / 2));
			firstClick = false;
		}

		double mouseX;
		double mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);

		float rotx = sensitivity * rotMultiplier * (float)(mouseY - (height / 2)) / height;
		float rotY = sensitivity * rotMultiplier * (float)(mouseX - (width / 2)) / width;

		//Calculates upcoming vertical change in the Orientation
		glm::vec3 newOrientation = glm::rotate(Orientation, glm::radians(-rotx), glm::normalize(glm::cross(Orientation, Up)));

		if (glm::dot(glm::normalize(Orientation - glm::vec3(0, 0, Orientation.z)), newOrientation) > 0.01) {
			Orientation = newOrientation;
		}

		Orientation = glm::rotate(Orientation, glm::radians(-rotY), Up);
		
		//Reset mouse position to the middle (while still hidden)
		glfwSetCursorPos(window, (width / 2), (height / 2));
	}
	else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		firstClick = true;
	}


	//Show current camera values
	if (glfwGetKey(window, CAM_INFO) == GLFW_PRESS) {
		if (showValuesClick) {
			showValuesClick = false;
			std::cout << "Current LPP: " << currentLengthPerPixel << "\n";
			std::cout << "Window size: (" << width << ", " << height << ")\n";
			std::cout << "World-space window size: (" << width * currentLengthPerPixel << ", " << height * currentLengthPerPixel << ")\n";
			std::cout << "Position: " << glm::to_string(Position) << "\n";
		}
	}
	else if (glfwGetKey(window, CAM_INFO) == GLFW_RELEASE) {
		showValuesClick = true;
	}

	//Orthogonal looking angles
	if (glfwGetKey(window, CAM_LOOK_DOWN) == GLFW_PRESS) {
		if (camLookDownClick) {
			camLookDownClick = false;
			Orientation = glm::normalize(glm::vec3(0, 1e-3, -1));
			ortho = true;
			Position = glm::vec3(0, 0, 10);

		}
	}
	else if (glfwGetKey(window, CAM_LOOK_DOWN) == GLFW_RELEASE) {
		camLookDownClick = true;
	}

	if (glfwGetKey(window, CAM_LOOK_RIGHT) == GLFW_PRESS) {
		if (camLookRightClick) {
			camLookRightClick = false;
			Orientation = glm::vec3(-1, 0, 0);
			ortho = true;
			Position = glm::vec3(10, 0, 0);
		}
	}
	else if (glfwGetKey(window, CAM_LOOK_RIGHT) == GLFW_RELEASE) {
		camLookRightClick = true;
	}

	if (glfwGetKey(window, CAM_LOOK_DIAGONAL) == GLFW_PRESS) {
		if (camLookRightClick) {
			camLookRightClick = false;
			Orientation = glm::vec3(-0.5*std::sqrt(2), 0, -0.5*std::sqrt(2));
			ortho = true;
			Position = glm::vec3(10, 0, 10);
		}
	}
	else if (glfwGetKey(window, CAM_LOOK_DIAGONAL) == GLFW_RELEASE) {
		camLookRightClick = true;
	}
}

void Camera::OnScroll(float offset) {
	if (this->ortho) {
		this->currentLengthPerPixel += offset * this->scrollLengthSensitivity;
		if (this->currentLengthPerPixel < 0.0f) {
			this->currentLengthPerPixel = 0.0f;
		}
		speedMultiplier = currentLengthPerPixel * speedMultSensitivity;
		rotMultiplier = 1.0f;
	}
	else {
		this->currentFOV = clamp2(this->currentFOV + offset * this->scrollFOVSensitivity, 0.0f, 3.14f);
		speedMultiplier = 1.0f;
		rotMultiplier = currentFOV;
	}
}