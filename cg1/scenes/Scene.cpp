#include "Scene.h"
#include <glm/glm.hpp>
#include <type_traits>
#include "../gfx/GPUProgram.h"
#include "../core/Camera.h"
#include "../gfx/Texture.h"
#include "../gfx/Mesh.h"
#include <imgui.h>

namespace cg1 {

    /**
     *  Constructor.
     */
    Scene::Scene() :
        program_{ std::make_unique<GPUProgram>("ExampleProgram",
            std::initializer_list<std::string>{"example.vert", "example.frag"}) },
        mesh_{ std::make_unique<Mesh>("example.obj") },
        diffuseTex_{ std::make_unique<Texture>("example_d.tga") },
        heightTex_{ std::make_unique<Texture>("example_h.tga") },
        matModelUniformLocation_{ -1 },
        matNormalUniformLocation_{ -1 },
        matMVPUniformLocation_{ -1 },
        texDiffuseUniformLocation_{ -1 },
        texHeightUniformLocation_{ -1 },
        vecHeightColorUniformLocation_{ -1 },
        modelMatrix_{ 1.0f },
        normalMatrix_{ 1.0f },
        MVPMatrix_{ 1.0f },
        heightColor{ 0.0f, 1.0f, 0.0f }
    {
        matModelUniformLocation_ = glGetUniformLocation(program_->getProgramId(), "matModel");
        matNormalUniformLocation_ = glGetUniformLocation(program_->getProgramId(), "matNormal");
        matMVPUniformLocation_ = glGetUniformLocation(program_->getProgramId(), "matMVP");
        texDiffuseUniformLocation_ = glGetUniformLocation(program_->getProgramId(), "diffuseTex");
        texHeightUniformLocation_ = glGetUniformLocation(program_->getProgramId(), "heightTex");
        vecHeightColorUniformLocation_ = glGetUniformLocation(program_->getProgramId(), "heightColor");
    }

    /**
     *  Destructor.
     */
    Scene::~Scene() = default;

    /**
     *  Updates the current scene.
     *  @param camera the camera object.
     *  @param currentTime the current application time.
     *  @param elapsedTime the time elapsed during the last frame.
     */
    void Scene::updateScene(const CG1Camera& camera, double currentTime, double elapsedTime) noexcept
    {
        normalMatrix_ = glm::mat4(glm::mat3(modelMatrix_));
        MVPMatrix_ = camera.getProjMatrix() * camera.getViewMatrix() * modelMatrix_;
    }

    /**
     *  Renders the scene.
     */
    void Scene::renderScene()
    {
        if (DRAW_GUI) {
            ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiSetCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(500, 200), ImGuiSetCond_FirstUseEver);
            ImGui::Begin("Render Parameters");
            ImGui::Text("Hello World");
            ImGui::ColorEdit3("Height Color (Example)", reinterpret_cast<float*>(&heightColor));
            ImGui::End();
        }

        glUseProgram(program_->getProgramId());
        glUniformMatrix4fv(matModelUniformLocation_, 1, GL_FALSE, reinterpret_cast<GLfloat*>(&modelMatrix_));
        glUniformMatrix4fv(matNormalUniformLocation_, 1, GL_FALSE, reinterpret_cast<GLfloat*>(&normalMatrix_));
        glUniformMatrix4fv(matMVPUniformLocation_, 1, GL_FALSE, reinterpret_cast<GLfloat*>(&MVPMatrix_));
        glActiveTexture(GL_TEXTURE0 + 0);
        glBindTexture(GL_TEXTURE_2D, diffuseTex_->getTextureId());
        glUniform1i(texDiffuseUniformLocation_, 0);
        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(GL_TEXTURE_2D, heightTex_->getTextureId());
        glUniform1i(texHeightUniformLocation_, 1);
        glUniform3f(vecHeightColorUniformLocation_, heightColor.x, heightColor.y, heightColor.z);

        mesh_->DrawComplete();
        glUseProgram(0);
    }
}
