#pragma once

#include "cg1.h"
#include <glm/glm.hpp>

namespace cg1 {

    class Application;

    /**
     * Helper class for generic arc-balls.
     */
    class Arcball final
    {
    public:
        Arcball(int button) noexcept;

        bool handleMouse(int button, int action, const Application* sender) noexcept;
        glm::quat getWorldRotation(const glm::mat4& view) noexcept;

    private:
        /** Holds the button to use. */
        const int button_;
        /** Holds whether the arc-ball is currently rotated. */
        bool arcballOn_;
        /** holds the current arc-ball position in normalized device coordinates. */
        glm::vec3 currentScreen_;
        /** holds the last arc-ball position in normalized device coordinates. */
        glm::vec3 lastScreen_;
    };
}
