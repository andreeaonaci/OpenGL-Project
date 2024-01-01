// Camera.hpp

#ifndef Camera_hpp
#define Camera_hpp

#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include <GL/glew.h>

namespace gps {

    enum MOVE_DIRECTION { MOVE_FORWARD, MOVE_BACKWARD, MOVE_RIGHT, MOVE_LEFT };

    class Camera {

    public:
        // Camera constructor
        Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp);
        glm::mat4 getViewMatrix();
        void move(MOVE_DIRECTION direction, float speed);
        void rotate(float yaw, float pitch);
        void toggleAutoRotation();
        void scenePreview(float elapsedTime, float rotationSpeed);
        void rotateForPreview(float pitch, float yaw);
        glm::vec3 getCameraTarget();

    private:
        void updateCameraVectors();

        glm::vec3 cameraPosition;
        glm::vec3 cameraTarget;
        glm::vec3 cameraFrontDirection;
        glm::vec3 cameraRightDirection;
        glm::vec3 cameraUpDirection;
        glm::vec3 originalUp;
        glm::vec3 originalFront;

        float yaw;
        float pitch;

        bool firstMouse;
        float lastX;
        float lastY;
        float mouseSensitivity;
    };
}

#endif /* Camera_hpp */
