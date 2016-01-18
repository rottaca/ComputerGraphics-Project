#include "Arcball.h"
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/quaternion.hpp>
#include <limits>
#include "Application.h"
#include <GLFW/glfw3.h>
#include <iostream>

#undef min
#undef max

namespace cg1 {

    /**
     *  Constructor.
     *  @param theButton the mouse button to use.
     */
    Arcball::Arcball(int theButton) noexcept :
        button_{ theButton },
        arcballOn_{ false },
        currentScreen_{ 0.0f },
        lastScreen_{ 0.0f }
    {
    }

    /**
     *  Handles the mouse input to the arc-ball.
     *  @param button the mouse button the event belongs to.
     *  @param action the mouse buttons action.
     *  @param sender the application to supply normalized screen coordinates.
     */
    bool Arcball::handleMouse(int theButton, int action, const Application* sender) noexcept
    {
        bool handled = false;
        if (button_ == theButton && action == GLFW_PRESS) {
            arcballOn_ = true;
            lastScreen_ = currentScreen_ = sender->getMousePositionNormalized();
            handled = true;
        } else if (arcballOn_ && sender->isMouseButtonPressed(button_)) {
            currentScreen_ = sender->getMousePositionNormalized();
            handled = true;
        } else if (!sender->isMouseButtonPressed(button_)) {
            handled = arcballOn_;
            arcballOn_ = false;
        }

        return handled;
    }

    /**
     *  Calculates the world rotation using a view matrix.
     *  @param view the view matrix.
     */
    glm::quat Arcball::getWorldRotation(const glm::mat4& view) noexcept
    {
        glm::quat result(1.0f, 0.0f, 0.0f, 0.0f);
        if (currentScreen_ != lastScreen_) {
            float angle = acos(glm::min(1.0f, glm::dot(lastScreen_, currentScreen_)));
            glm::vec3 camAxis = glm::cross(lastScreen_, currentScreen_);
            glm::vec3 worldAxis = glm::normalize(glm::vec3(glm::inverse(glm::mat3(view)) * camAxis));
            result = glm::angleAxis(-1.5f * angle, worldAxis);
            lastScreen_ = currentScreen_;
        }
        return result;
    }
}
