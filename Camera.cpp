#include "Camera.hpp"

namespace gps {
    
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget)
    {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraDirection = glm::normalize(cameraTarget - cameraPosition);
        this->cameraRightDirection = -glm::normalize(glm::cross(this->cameraDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
		this->cameraUpDirection = glm::normalize(glm::cross(glm::vec3(1.0f, 0.0f, 0.0f), this->cameraDirection));
    }
    
    glm::mat4 Camera::getViewMatrix()
    {
        return glm::lookAt(cameraPosition, cameraPosition + cameraDirection , cameraUpDirection);
    }

	glm::vec3 Camera::getPosition() {
		return cameraPosition;
	}

	glm::vec3 Camera::getCameraTarget() {
		return cameraTarget;
	}
    
    void Camera::move(MOVE_DIRECTION direction, float speed)
    {
        switch (direction) {
            case MOVE_FORWARD:
                cameraPosition += cameraDirection * speed;
                break;
                
            case MOVE_BACKWARD:
                cameraPosition -= cameraDirection * speed;
                break;
                
            case MOVE_RIGHT:
                cameraPosition -= cameraRightDirection * speed;
                break;
                
            case MOVE_LEFT:
                cameraPosition += cameraRightDirection * speed;
                break;

			case MOVE_UP:
				cameraPosition += cameraUpDirection * speed;
				break;

			case MOVE_DOWN:
				cameraPosition -= cameraUpDirection * speed;
				break;
        }
		if (cameraPosition.x > 9) cameraPosition.x = 9;
		if (cameraPosition.x < -9) cameraPosition.x = -9;
		if (cameraPosition.z > 9) cameraPosition.z = 9;
		if (cameraPosition.z < -9) cameraPosition.z = -9;
		if (cameraPosition.y > 20) cameraPosition.y = 20;
		if (cameraPosition.y < -1) cameraPosition.y = -1;
    }
    
    void Camera::rotate(float pitch, float yaw)
    {
		glm::mat4 rotationMatrix(1.0f);
		rotationMatrix = glm::rotate(rotationMatrix, pitch, this->cameraRightDirection);
		glm::vec4 res = glm::vec4(this->cameraDirection, 0.0f)*rotationMatrix;

		this->cameraDirection = glm::vec3(res.x, res.y, res.z);
		this->cameraUpDirection = glm::normalize(glm::cross(this->cameraDirection, this->cameraRightDirection));

		rotationMatrix = glm::mat4(1.0f);
		rotationMatrix = glm::rotate(rotationMatrix, yaw, this->cameraUpDirection);
		res = glm::vec4(this->cameraDirection, 0.0f)*rotationMatrix;

		this->cameraDirection = glm::vec3(res.x, res.y, res.z);
		this->cameraRightDirection = glm::normalize(glm::cross(this->cameraUpDirection, this->cameraDirection));
    }
    
}
