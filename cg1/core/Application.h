#pragma once

#include "cg1.h"
#include <glm/glm.hpp>
#include "scenes/Scene.h"

struct GLFWwindow;

namespace cg1 {

    class Scene;

    class Application final
    {
    public:
        Application(const std::string& applicationName);
        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;
        Application(Application&&) = delete;
        Application& operator=(Application&&) = delete;
        ~Application() noexcept;

        bool isRunning() const noexcept;
        void updateObjects();
        void render();

        /** Returns the current mouse position. */
        const glm::vec2& getMousePosition() const noexcept { return mousePosition_; }
        /** Return the current mouse position in normalized coordinates. */
        const glm::vec3& getMousePositionNormalized() const noexcept { return mousePositionNormalized_; }

        bool isMouseButtonPressed(int button) const noexcept;
        bool isKeyPressed(int key) const noexcept;

        static void glfwErrorCallback(int error, const char* description) noexcept;
        static void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) noexcept;
        static void glfwCursorCallback(GLFWwindow* window, double xpos, double ypos) noexcept;
        static void glfwScrollCallback(GLFWwindow* window, double xoffset, double yoffset) noexcept;
        static void glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) noexcept;
        static void glfwCharCallback(GLFWwindow* window, unsigned int codepoint) noexcept;

    private:
        void handleKeyboard(int key, int action, int mods) noexcept;
        void handleMouseMove(const glm::vec2& pos) noexcept;
        void handleMouseScroll(const glm::vec2& scrollOffset) noexcept;
        void handleMouseClick(int button, int action, int mods) noexcept;

        /** Holds the GLFW window. */
        GLFWwindow* window_;
        /** Holds the current window size. */
        glm::vec2 windowSize_;
        /** Holds the current frames timestamp. */
        double currentTime_;
        /** Holds the last frames timestamp. */
        double lastTime_;
        /** Holds the current mouse position. */
        glm::vec2 mousePosition_;
        /** Holds the current normalized mouse position. */
        glm::vec3 mousePositionNormalized_;
        /** Holds the (main) camera object. */
        CG1Camera camera_;
        /** Holds the current scene. */
        std::unique_ptr<Scene> scene_;
    };
}
