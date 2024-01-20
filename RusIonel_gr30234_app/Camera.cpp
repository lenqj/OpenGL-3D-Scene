#include "Camera.hpp"

namespace gps {

	//Camera constructor
	Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
		//TODO
		this->cameraPosition = cameraPosition;
		this->cameraTarget = cameraTarget;
		this->cameraUpDirection = cameraUp;
		this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
		this->cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));

	}

	//return the view matrix, using the glm::lookAt() function
	glm::mat4 Camera::getViewMatrix() {
		//TODO

		return glm::lookAt(cameraPosition, cameraPosition + cameraFrontDirection, cameraUpDirection);
	}

	//update the camera internal parameters following a camera move event
	void Camera::move(MOVE_DIRECTION direction, float speed) {
		switch (direction) {
		case MOVE_FORWARD:
			cameraPosition += cameraFrontDirection * speed;
			break;

		case MOVE_BACKWARD:
			cameraPosition -= cameraFrontDirection * speed;
			break;

		case MOVE_RIGHT:
			cameraPosition += cameraRightDirection * speed;
			break;

		case MOVE_LEFT:
			cameraPosition -= cameraRightDirection * speed;
			break;
		case MOVE_UP:
			cameraPosition += cameraUpDirection * speed;
			break;

		case MOVE_DOWN:
			cameraPosition -= cameraUpDirection * speed;
			break;
		}
	}

	//update the camera internal parameters following a camera rotate event
	//yaw - camera rotation around the y axis
	//pitch - camera rotation around the x axis
	void Camera::rotate(float pitch, float yaw) {
		cameraFrontDirection = glm::normalize(glm::vec3(cos(yaw) * cos(pitch), sin(pitch), sin(yaw) * cos(pitch)));
		cameraRightDirection = glm::normalize(glm::cross(this->cameraFrontDirection, this->cameraUpDirection));
	}

}
