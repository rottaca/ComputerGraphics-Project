#pragma once

#include "cg1.h"
#include "core/Arcball.h"
#include <glm/gtc/quaternion.hpp>

namespace cg1 {

    /**
    * Represents a free moving camera.
    */
    class FreeCamera final
    {
    public:
        FreeCamera(float fovY, float aspectRatio, const glm::vec2& screenSize, float nearZ, float farZ,
            const glm::vec3& camPos) noexcept;

        bool handleMouse(int button, int action, float mouseWheelDelta, const Application* sender) noexcept;
        void updateCamera(const Application* sender) noexcept;

        /** Returns the cameras view matrix. */
        const glm::mat4& getViewMatrix() const noexcept { return view_; }
        /** Returns the cameras projection matrix. */
        const glm::mat4& getProjMatrix() const noexcept { return perspective_; }
        /** Returns the cameras position. */
        const glm::vec3& getPosition() const noexcept { return camPos_; }

    private:
        /** Holds the field of view in y direction. */
        float fovY_;
        /** Holds the aspect ratio. */
        float aspectRatio_;
        /** Holds the screen size. */
        glm::vec2 screenSize_;
        /** Holds the near z plane. */
        float nearZ_;
        /** Holds the far z plane. */
        float farZ_;
        /** Holds the perspective transform matrix. */
        glm::mat4 perspective_;
        /** Holds the current camera position. */
        glm::vec3 camPos_;
        /** Holds the current camera orientation. */
        glm::mat4 camOrient_;
        /** Holds the current up vector of the camera. */
        glm::vec3 camUp_;
        /** Holds the cameras view matrix. */
        glm::mat4 view_;
    };
}
