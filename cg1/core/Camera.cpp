#include "Camera.h"
#include <GLFW/glfw3.h>

#define GLM_SWIZZLE

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/quaternion.hpp>

namespace cg1 {

    /**
     *  Constructor.
     *  @param theFovY the field of view in y direction.
     *  @param theAspectRatio the screens aspect ratio.
     *  @param theScreenSize the screen size.
     *  @param theNearZ the near z plane
     *  @param theFarZ the far z plane
     *  @param theCamPos the cameras initial position.
     */
    Camera::Camera(float theFovY, float theAspectRatio, const glm::vec2& theScreenSize, float theNearZ,
        float theFarZ, const glm::vec3& theCamPos) noexcept :
        fovY_(theFovY),
        aspectRatio_(theAspectRatio),
        screenSize_(theScreenSize),
        nearZ_(theNearZ),
        farZ_(theFarZ),
        camPos_(theCamPos),
        camOrient_(1.0f, 0.0f, 0.0f, 0.0f),
        camUp_(0.0f, 1.0f, 0.0f),
        camArcball_(GLFW_MOUSE_BUTTON_1)
    {
        perspective_ = glm::perspective(glm::radians(fovY_), aspectRatio_, nearZ_, farZ_);
        view_ = glm::lookAt(camPos_, glm::vec3(0.0f), camUp_);
    }

    /**
     *  Updates the camera parameters using the internal arc-ball.
     */
    void Camera::updateCamera(const Application*) noexcept
    {
        glm::quat camOrientStep = camArcball_.getWorldRotation(view_);
        glm::mat3 matOrientStep{ glm::mat3_cast(camOrientStep) };
        camOrient_ = camOrientStep * camOrient_;
        glm::mat3 matOrient{ glm::mat3_cast(camOrient_) };
        
        camUp_ = matOrient[1];
        camPos_ = matOrientStep * camPos_;

        view_ = glm::lookAt(camPos_, glm::vec3(0.0f), camUp_);
    }

    /**
     *  Handles the mouse events for the camera.
     *  @param button the mouse button the event belongs to.
     *  @param action the mouse buttons action.
     *  @param sender the application to supply normalized screen coordinates.
     */
    bool Camera::handleMouse(int button, int action, float mouseWheelDelta, const Application* sender) noexcept
    {
        bool handled = camArcball_.handleMouse(button, action, sender);

        if (mouseWheelDelta != 0) {
            fovY_ -= mouseWheelDelta * 1.0f;
            fovY_ = glm::clamp(fovY_, 1.0f, 170.0f);
            perspective_ = glm::perspective(glm::radians(fovY_), aspectRatio_, nearZ_, farZ_);
            handled = true;
        }

        return handled;
    }
}
