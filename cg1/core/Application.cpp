#include "Application.h"
#include <GLFW/glfw3.h>
#include "cg1.h"
#include <iostream>
#include "../scenes/Scene.h"
#include <imgui.h>
#include "imgui_impl_glfw_gl3.h"

namespace cg1 {

    /**
     *  Constructor, creates the cg1 application.
     *  @param applicationName the name of the application (will appear as the window title).
     */
    Application::Application(const std::string& applicationName) :
        window_{ nullptr },
        windowSize_{ static_cast<float>(config::windowWidth), static_cast<float>(config::windowHeight) },
        currentTime_{ 0.0 },
        lastTime_{ 0.0 },
        mousePosition_{ 0.0f, 0.0f },
        mousePositionNormalized_{ 0.0f, 0.0f, 0.0f },
        camera_{ 60.0f, windowSize_.x / windowSize_.y, windowSize_, 1.0f, 100.0f, glm::vec3(0.0f, 0.0f, -10.0f) }
    {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

        glfwSetErrorCallback(Application::glfwErrorCallback);
        window_ = glfwCreateWindow(static_cast<int>(windowSize_.x), static_cast<int>(windowSize_.y),
            applicationName.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(window_, this);

        ImGui_ImplGlfwGL3_Init(window_, false);

        glfwSetInputMode(window_, GLFW_STICKY_MOUSE_BUTTONS, 1);
        glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        glfwSetKeyCallback(window_, Application::glfwKeyCallback);
        glfwSetCursorPosCallback(window_, Application::glfwCursorCallback);
        glfwSetScrollCallback(window_, Application::glfwScrollCallback);
        glfwSetMouseButtonCallback(window_, Application::glfwMouseButtonCallback);
        glfwSetCharCallback(window_, Application::glfwCharCallback);

        // Check for Valid Context
        if (window_ == nullptr) {
            std::cerr << "Failed to Create OpenGL Context!" << std::endl;
            glfwTerminate();
            throw std::runtime_error("Failed to Create OpenGL Context!");
        }

        // Create Context and Load OpenGL Functions
        glfwMakeContextCurrent(window_);
        gladLoadGL();
        std::cerr << "OpenGL " << glGetString(GL_VERSION) << std::endl;
        currentTime_ = lastTime_ = glfwGetTime();

        glFrontFace(GL_CCW);
        glCullFace(GL_BACK);
        glEnable(GL_CULL_FACE);
        glDepthFunc(GL_LEQUAL);
        glEnable(GL_DEPTH_TEST);

        scene_ = std::make_unique<Scene>();
    }

    /**
     *  Destructor.
     */
    Application::~Application() noexcept
    {
        ImGui_ImplGlfwGL3_Shutdown();
        scene_.reset();
        if (window_) glfwDestroyWindow(window_);
        glfwTerminate();
    }

    /**
     *  Checks if the application is still running.
     *  @return whether the application is still running.
     */
    bool Application::isRunning() const noexcept
    {
        return glfwWindowShouldClose(window_) == false;
    }

    /**
     *  Updates application objects.
     */
    void Application::updateObjects()
    {
        currentTime_ = glfwGetTime();
        double elapsedTime = currentTime_ - lastTime_;
        glfwPollEvents();
        if (DRAW_GUI) ImGui_ImplGlfwGL3_NewFrame();
        camera_.updateCamera(this);

        scene_->updateScene(camera_, currentTime_, elapsedTime);
        lastTime_ = currentTime_;
    }

    void Application::render()
    {
        // clear back buffer
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClearDepth(1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        scene_->renderScene();
        if (DRAW_GUI) ImGui::Render();

        // Flip Buffers and Draw
        glfwSwapBuffers(window_);
    }

    /**
     *  Check if mouse button is currently pressed.
     *  @param button the button to check.
     *  @return whether the button is down or not.
     */
    bool Application::isMouseButtonPressed(int button) const noexcept
    {
        return glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    }

    bool Application::isKeyPressed(int key) const noexcept
    {
        return glfwGetKey(window_, key) == GLFW_PRESS;
    }

    /**
     *  Handles the keyboard events.
     *  @param key the pressed keys key code.
     *  @param action the key action.
     *  @param mods the applied key modifiers.
     */
    void Application::handleKeyboard(int key, int action, int mods) noexcept
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window_, true);
    }

    /**
     *  Handles mouse motion events.
     *  @param pos the mouse position.
     */
    void Application::handleMouseMove(const glm::vec2& pos) noexcept
    {
        mousePosition_ = pos;
        mousePositionNormalized_.x = (2.0f * mousePosition_.x - windowSize_.x) / windowSize_.x;
        mousePositionNormalized_.y = -(2.0f * mousePosition_.y - windowSize_.y) / windowSize_.y;
        mousePositionNormalized_.z = 0.0f;
        mousePositionNormalized_ = glm::clamp(mousePositionNormalized_, glm::vec3(-1.0f), glm::vec3(1.0f));

        float length_squared = glm::dot(mousePositionNormalized_, mousePositionNormalized_);
        if (length_squared <= 1.0f) mousePositionNormalized_.z = sqrtf(1.0f - length_squared);
        else mousePositionNormalized_ = glm::normalize(mousePositionNormalized_);

        camera_.handleMouse(-1, 0, 0.0f, this);
    }

    /**
     *  Handles mouse scroll events.
     *  @param scrollOffset the mouse scrolling offset.
     */
    void Application::handleMouseScroll(const glm::vec2& scrollOffset) noexcept
    {
        camera_.handleMouse(-1, 0, scrollOffset.y, this);
    }

    /**
     *  Handles mouse clicks events.
     *  @param button the mouse button the event belongs to.
     *  @param action the mouse buttons action.
     *  @param mods the applied modifiers.
     */
    void Application::handleMouseClick(int button, int action, int mods) noexcept
    {
        camera_.handleMouse(button, action, 0.0f, this);
    }

    /**
     *  Handles GLFW errors.
     *  @param error the error to handle.
     *  @param description the error description.
     */
    void Application::glfwErrorCallback(int error, const char* description) noexcept
    {
        std::cerr << "An GLFW error occurred (" << error << "): " << std::endl;
        std::cerr << description << std::endl;
    }

    /**
     *  Callback for GLFW key handling.
     *  @param window the window this event belongs to.
     *  @param key the pressed keys key code.
     *  @param scancode the platform specific scan code.
     *  @param action the key action.
     *  @param mods the applied key modifiers.
     */
    void Application::glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) noexcept
    {
        ImGui_ImplGlfwGL3_KeyCallback(window, key, scancode, action, mods);

        ImGuiIO& io = ImGui::GetIO();
        if (!io.WantCaptureKeyboard) {
            auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
            app->handleKeyboard(key, action, mods);
        }
    }

    /**
     *  Callback for GLFW mouse cursor handling.
     *  @param window the window this event belongs to.
     *  @param xpos the mouse cursors x position.
     *  @param ypos the mouse cursors y position.
     */
    void Application::glfwCursorCallback(GLFWwindow* window, double xpos, double ypos) noexcept
    {
        ImGuiIO& io = ImGui::GetIO();
        if (!io.WantCaptureMouse) {
            auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
            app->handleMouseMove(glm::vec2(static_cast<float>(xpos), static_cast<float>(ypos)));
        }
    }

    /**
     *  Callback for GLFW mouse scroll handling.
     *  @param window the window this event belongs to.
     *  @param xoffset the mouse scroll x offset.
     *  @param yoffset the mouse scroll y offset.
     */
    void Application::glfwScrollCallback(GLFWwindow* window, double xoffset, double yoffset) noexcept
    {
        ImGui_ImplGlfwGL3_ScrollCallback(window, xoffset, yoffset);

        ImGuiIO& io = ImGui::GetIO();
        if (!io.WantCaptureMouse) {
            auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
            app->handleMouseScroll(glm::vec2(static_cast<float>(xoffset), static_cast<float>(yoffset)));
        }
    }

    /**
     *  Callback for GLFW mouse clicks handling.
     *  @param window the window this event belongs to.
     *  @param button the mouse button the event belongs to.
     *  @param action the mouse buttons action.
     *  @param mods the applied modifiers.
     */
    void Application::glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) noexcept
    {
        ImGui_ImplGlfwGL3_MouseButtonCallback(window, button, action, mods);

        ImGuiIO& io = ImGui::GetIO();
        if (!io.WantCaptureMouse) {
            auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
            app->handleMouseClick(button, action, mods);
        }
    }

    void Application::glfwCharCallback(GLFWwindow* window, unsigned int codepoint) noexcept
    {
        ImGui_ImplGlfwGL3_CharCallback(window, codepoint);
    }
}
