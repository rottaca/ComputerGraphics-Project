#include "Scene.h"
#include <glm/glm.hpp>
#include <type_traits>
#include "../gfx/GPUProgram.h"
#include "../core/Camera.h"
#include "../gfx/Texture.h"
#include "../gfx/Mesh.h"
#include <imgui.h>

#include <sstream>
#include <iostream>

namespace cg1 {

    /**
     *  Constructor.
     */
    Scene::Scene() :
        program_{ std::make_unique<GPUProgram>("ExampleProgram",
            std::initializer_list<std::string>{"sceneShader.vert", "sceneShader.frag"}) },
        modelMatrix_{ 1.0f },
        normalMatrix_{ 1.0f },
        VPMatrix_{ 1.0f },
		viewMatrix_{1.0f},
		currentTime_{0},
		obj_{new SceneObject("terrainSurface.obj",{"terrain-texture.tga"})}
    {
        matModelUniformLocation_ = glGetUniformLocation(program_->getProgramId(), "matModel");
        matNormalUniformLocation_ = glGetUniformLocation(program_->getProgramId(), "matNormal");
        matVPUniformLocation_ = glGetUniformLocation(program_->getProgramId(), "matVP");
        tex0UniformLocation_ = glGetUniformLocation(program_->getProgramId(), "tex");
        cameraPosUniformLocation_ = glGetUniformLocation(program_->getProgramId(), "camPos");

        // setup lights
        Light directionalLight;
        directionalLight.position = glm::vec4(50, 50, 50, 0); //w == 0 indications a directional light
        directionalLight.intensities = glm::vec3(0.4,0.3,0.1); //weak yellowish light
        directionalLight.ambientCoefficient = 0.06;

//        Light spotlight;
//        spotlight.position = glm::vec4(2,2,2,1);
//        spotlight.intensities = glm::vec3(2,2,2); //strong white light
//        spotlight.attenuation = 0.1f;
//        spotlight.ambientCoefficient = 0.0f; //no ambient light
//        spotlight.coneAngle = 30.0f;
//        spotlight.coneDirection = glm::vec3(-1,-1,-1);

//        gLights.push_back(spotlight);
        gLights.push_back(directionalLight);
    }

    /**
     *  Destructor.
     */
    Scene::~Scene(){
    	delete obj_;
    }

    /**
     *  Updates the current scene.
     *  @param camera the camera object.
     *  @param currentTime the current application time.
     *  @param elapsedTime the time elapsed during the last frame.
     */
    void Scene::updateScene(const CG1Camera& camera, double currentTime, double elapsedTime) noexcept
    {
        VPMatrix_ = camera.getProjMatrix() * camera.getViewMatrix();
        viewMatrix_ = camera.getViewMatrix();
        camPos_ = camera.getPosition();
        currentTime_ = currentTime;
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
            ImGui::End();
        }

        glUseProgram(program_->getProgramId());
        //////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Update general uniforms
        //////////////////////////////////////////////////////////////////////////////////////////////////////////
        glUniformMatrix4fv(matVPUniformLocation_, 1, GL_FALSE, reinterpret_cast<GLfloat*>(&VPMatrix_));
        glUniform3fv(cameraPosUniformLocation_, 1, reinterpret_cast<GLfloat*>(&camPos_));

        // Update Light
        updateLight();

        //////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Update Scene properties
        //////////////////////////////////////////////////////////////////////////////////////////////////////////

        glUniformMatrix4fv(matModelUniformLocation_, 1, GL_FALSE, reinterpret_cast<GLfloat*>(&modelMatrix_));
        normalMatrix_ = glm::mat4(glm::mat3(modelMatrix_));
        glUniformMatrix4fv(matNormalUniformLocation_, 1, GL_FALSE, reinterpret_cast<GLfloat*>(&normalMatrix_));

        //////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Render Scene
        //////////////////////////////////////////////////////////////////////////////////////////////////////////

        updateMaterial(1,glm::vec3(0.1,0.1,0.1));

        glUniform1i(tex0UniformLocation_,0);
        obj_->bindTexturesAndDrawMesh();

        glUseProgram(0);
    }

    /**
     *  Updates uniforms for light calculations in the shader.
     */
    void Scene::updateLight()
    {
    	glUniform1i(glGetUniformLocation(program_->getProgramId(), "numLights"), (int)gLights.size());

    	for(size_t i = 0; i < gLights.size(); ++i){
    		std::cout << "Light: " << i << std::endl;
    		glUniform4fv(glGetUniformLocation(program_->getProgramId(), getLightUniformName("position", i).c_str()), 1, reinterpret_cast<GLfloat*>(&gLights[i].position));
    		glUniform3fv(glGetUniformLocation(program_->getProgramId(), getLightUniformName("intensities", i).c_str()),1, reinterpret_cast<GLfloat*>(&gLights[i].intensities));
    		glUniform1f(glGetUniformLocation(program_->getProgramId(), getLightUniformName("attenuation", i).c_str()), gLights[i].attenuation);
    		glUniform1f(glGetUniformLocation(program_->getProgramId(), getLightUniformName("ambientCoefficient", i).c_str()), gLights[i].ambientCoefficient);
    		glUniform1f(glGetUniformLocation(program_->getProgramId(), getLightUniformName("coneAngle", i).c_str()), gLights[i].coneAngle);
    		glUniform3fv(glGetUniformLocation(program_->getProgramId(), getLightUniformName("coneDirection", i).c_str()),1, reinterpret_cast<GLfloat*>(&gLights[i].coneDirection));
    	}
    }

    std::string Scene::getLightUniformName(const char* propertyName, size_t lightIndex)
    {
        std::ostringstream ss;
        ss << "allLights[" << lightIndex << "]." << propertyName;
        std::cout << ss.str() << std::endl;
        return ss.str();
    }
    void Scene::updateMaterial(float shininess, glm::vec3 specularColor)
    {
    	glUniform1f(glGetUniformLocation(program_->getProgramId(),"material.shininess"),shininess);
    	glUniform3fv(glGetUniformLocation(program_->getProgramId(),"material.specularColor"),1, reinterpret_cast<GLfloat*>(&specularColor));
    }
}
