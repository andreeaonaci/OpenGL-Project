#include "Camera.hpp"

bool autoRotateEnabled;

namespace gps {
    glm::vec3 aux;

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraFrontDirection = glm::normalize(cameraPosition - cameraTarget);
        this->cameraRightDirection = glm::normalize(glm::cross(cameraUp, cameraFrontDirection));
        this->cameraUpDirection = glm::cross(cameraFrontDirection, cameraRightDirection);
        aux = cameraFrontDirection;
        autoRotateEnabled = false;
    }

    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(cameraPosition, cameraPosition + cameraFrontDirection, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    void Camera::move(MOVE_DIRECTION direction, float speed) {
        switch (direction) {
        case MOVE_FORWARD:
            cameraPosition -= cameraFrontDirection * speed;
            break;

        case MOVE_BACKWARD:
            cameraPosition += cameraFrontDirection * speed;
            break;

        case MOVE_RIGHT:
            cameraPosition += glm::normalize(glm::cross(cameraUpDirection, cameraFrontDirection)) * speed;
            cameraTarget = cameraPosition + cameraFrontDirection;
            break;

        case MOVE_LEFT:
            cameraPosition -= glm::normalize(glm::cross(cameraUpDirection, cameraFrontDirection)) * speed;
            cameraTarget = cameraPosition + cameraFrontDirection;
            break;
        }
    }

    void Camera::rotate(float pitch, float yaw) {

        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFrontDirection = glm::normalize(direction);
    }

    void Camera::toggleAutoRotation() {
        autoRotateEnabled = !autoRotateEnabled;
    }

    glm::vec3 Camera::getCameraTarget()
    {
        return cameraTarget;
    }

    void Camera::rotateForPreview(float pitch, float yaw) {
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        cameraFrontDirection = glm::normalize(front);
    }

    void Camera::scenePreview(float elapsedTime, float rotationSpeed) {
        this->cameraPosition = glm::vec3(0.0f, 297.56f, 291.05f);

        float rotationAngle = elapsedTime * rotationSpeed;

        glm::mat4 r = glm::rotate(glm::mat4(1.0f), glm::radians(rotationAngle), glm::vec3(0, 1, 0));

        this->cameraPosition = glm::vec3(r * glm::vec4(this->cameraPosition, 1));
        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
    }
}