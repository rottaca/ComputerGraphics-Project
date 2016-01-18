#include "FreeCamera.h"
#include <GLFW/glfw3.h>

#define GLM_SWIZZLE

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include "Application.h"

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
    FreeCamera::FreeCamera(float theFovY, float theAspectRatio, const glm::vec2& theScreenSize, float theNearZ,
        float theFarZ, const glm::vec3& theCamPos) noexcept :
        fovY_(theFovY),
        aspectRatio_(theAspectRatio),
        screenSize_(theScreenSize),
        nearZ_(theNearZ),
        farZ_(theFarZ),
        camPos_(theCamPos),
        camOrient_(),
        camUp_(0.0f, 1.0f, 0.0f)
    {
        glm::scale(camOrient_, glm::vec3(1.0f, -1.0f, 1.0f));
        perspective_ = glm::perspective(glm::radians(fovY_), aspectRatio_, nearZ_, farZ_);
        view_ = glm::lookAt(camPos_, camPos_ - glm::vec3(0.0f, 0.0f, 1.0f), camUp_);
    }

    bool FreeCamera::handleMouse(int button, int action, float mouseWheelDelta, const Application* sender) noexcept
    {
        glm::vec2 mouseVel = sender->getMousePosition() - (screenSize_ * 0.5f);
        const float speed = 0.0005f;
        static float pitch, yaw;

        yaw = mouseVel.x * speed;
        pitch = mouseVel.y * speed;

        pitch = glm::clamp(pitch, -glm::half_pi<float>(), glm::half_pi<float>());

        if (yaw < -glm::two_pi<float>()) yaw += glm::two_pi<float>();
        if (yaw > glm::two_pi<float>()) yaw -= glm::two_pi<float>();

        camOrient_ = glm::eulerAngleXY(pitch, yaw);
        return true;
    }

    /**
     *  Updates the camera parameters using the internal arc-ball.
     */
    void FreeCamera::updateCamera(const Application* sender) noexcept
    {
        glm::vec3 camMove{ 0.0f };
        if (sender->isKeyPressed(GLFW_KEY_W)) camMove += glm::vec3(0.0f, 0.0f, 0.04f);
        if (sender->isKeyPressed(GLFW_KEY_A)) camMove += glm::vec3(0.04f, 0.0f, 0.0f);
        if (sender->isKeyPressed(GLFW_KEY_S)) camMove -= glm::vec3(0.0f, 0.0f, 0.04f);
        if (sender->isKeyPressed(GLFW_KEY_D)) camMove -= glm::vec3(0.04f, 0.0f, 0.0f);
        // TODO: More keys for y-movement? [1/13/2016 Sebastian Maisch]

        camPos_ += glm::vec3(glm::inverse(camOrient_) * glm::vec4(camMove, 1.0f));
        view_ = camOrient_* glm::translate(camPos_);
    }
}
