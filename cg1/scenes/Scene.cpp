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

#include <glm/gtc/matrix_transform.hpp>
#include "glm/ext.hpp"

namespace cg1 {

    /**
     *  Constructor.
     */
    Scene::Scene() :
		program_{ std::make_unique<GPUProgram>("DefaultProgram",
				std::initializer_list<std::string>{"sceneShader.vert", "sceneShader.frag"}) },
        VPMatrix_{ 1.0f },
		viewMatrix_{1.0f},
		currentTime_{0}
    {
        matModelUniformLocation_ = glGetUniformLocation(program_->getProgramId(), "matModel");
        matNormalUniformLocation_ = glGetUniformLocation(program_->getProgramId(), "matNormal");
        matVPUniformLocation_ = glGetUniformLocation(program_->getProgramId(), "matVP");
        tex0UniformLocation_ = glGetUniformLocation(program_->getProgramId(), "tex");
        cameraPosUniformLocation_ = glGetUniformLocation(program_->getProgramId(), "camPos");
        shaderModeUniformLocation_ = glGetUniformLocation(program_->getProgramId(), "shaderMode");
        timeUniformLocation_ = glGetUniformLocation(program_->getProgramId(), "time");
        depthTextureUniformLocation_ = glGetUniformLocation(program_->getProgramId(), "depthTex");
        matVPDepthUniformLocation_ = glGetUniformLocation(program_->getProgramId(), "matDepthVP");

        // Setup scene
        SceneObject* objTerrain = new SceneObject("terrainSurface.obj",{"terrain_DIFFUSE.jpg"});
        objTerrain->translate(glm::vec3(0, 0, 0));
        objTerrain->setMaterialAttributes(100,glm::vec3(1,1,1));
        objTerrain->setShaderMode(SceneObject::DEFAULT);
        SceneObject* objWater = new SceneObject("waterSurface.obj",{"water_DIFFUSE.jpg"});
        objWater->translate(glm::vec3(0, -2.2, 0));
        objWater->setMaterialAttributes(0.5,glm::vec3(1,1,1));
        objWater->setShaderMode(SceneObject::WATER);
        SceneObject* objStoneHenge = new SceneObject("Stonehengebed.obj",{"Stonehenge_texture.png"});
        objStoneHenge->translate(glm::vec3(0, 3, 0));
        objStoneHenge->setMaterialAttributes(100,glm::vec3(1,1,1));
        objStoneHenge->setShaderMode(SceneObject::DEFAULT);

        objPlane_ = new SceneObject("Plane.obj",{});

        m_sceneObjects.push_back(objTerrain);
        m_sceneObjects.push_back(objWater);
        m_sceneObjects.push_back(objStoneHenge);



        // setup lights
        Light directionalLight;
        directionalLight.position = glm::vec4(10, 10, 0, 0); //w == 0 indications a directional light
        directionalLight.intensities = glm::vec3(0.4,0.3,0.1); //weak yellowish light
        //directionalLight.intensities = glm::vec3(0.2,0.2,0.2); //white light
        directionalLight.ambientCoefficient = 0.1;
        directionalLight.att_c1 = 0;
        directionalLight.att_c2 = 0;
        directionalLight.att_c3 = 0;
        gLights.push_back(directionalLight);
//
//        Light pointLight;
//		pointLight.position = glm::vec4(0, 5, 0, 1);
//		pointLight.intensities = glm::vec3(0.8, 0.6, 0.2); //weak yellowish light
//		pointLight.ambientCoefficient = 0;
//		pointLight.coneAngle = 360;
//		pointLight.att_c1 = 0;
//		pointLight.att_c2 = 0;
//		pointLight.att_c3 = 0.1f;
//		gLights.push_back(pointLight);
//
//		Light spotlight;
//		spotlight.position = glm::vec4(0, 7, 0, 1);
//		spotlight.intensities = glm::vec3(2, 2, 2); //strong white light
//		spotlight.att_c1 = 0;
//		spotlight.att_c2 = 0;
//		spotlight.att_c3 = 0.1f;
//		spotlight.ambientCoefficient = 0; //no ambient light
//		spotlight.coneAngle = 20.0f;
//		spotlight.coneDirection = glm::vec3(0, -1, 0);
//		gLights.push_back(spotlight);

		initShadowMapping();
    }

    /**
     *  Destructor.
     */
    Scene::~Scene(){

    	for(int i = 0; i < m_sceneObjects.size();i++){
    		delete m_sceneObjects.at(i);
    	}
    	m_sceneObjects.clear();
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
        glUniform3fv(cameraPosUniformLocation_, 1, reinterpret_cast<GLfloat*>(&camPos_));

        // Update Light
        updateLight();

        //////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Update Scene properties
        //////////////////////////////////////////////////////////////////////////////////////////////////////////
        glUniform1f(timeUniformLocation_,currentTime_);

        //////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Render Scene
        //////////////////////////////////////////////////////////////////////////////////////////////////////////

        glUniform1i(tex0UniformLocation_,0);
        glUniform1i(depthTextureUniformLocation_,1);

        // Shadow mapping: Render to depth buffer
        renderDepthImage();

        renderRealImage();

        glUseProgram(0);
    }

    /**
     *  Updates uniforms for light calculations in the shader.
     */
    void Scene::updateLight()
    {
    	glUniform1i(glGetUniformLocation(program_->getProgramId(), "numLights"), (int)gLights.size());

    	for(size_t i = 0; i < gLights.size(); ++i){
    		glUniform4fv(glGetUniformLocation(program_->getProgramId(), getLightUniformName("position", i).c_str()), 1, reinterpret_cast<GLfloat*>(&gLights[i].position));
    		glUniform3fv(glGetUniformLocation(program_->getProgramId(), getLightUniformName("intensities", i).c_str()),1, reinterpret_cast<GLfloat*>(&gLights[i].intensities));
    		glUniform1f(glGetUniformLocation(program_->getProgramId(), getLightUniformName("att_c1", i).c_str()), gLights[i].att_c1);
    		glUniform1f(glGetUniformLocation(program_->getProgramId(), getLightUniformName("att_c2", i).c_str()), gLights[i].att_c2);
    		glUniform1f(glGetUniformLocation(program_->getProgramId(), getLightUniformName("att_c3", i).c_str()), gLights[i].att_c3);
    		glUniform1f(glGetUniformLocation(program_->getProgramId(), getLightUniformName("ambientCoefficient", i).c_str()), gLights[i].ambientCoefficient);
    		glUniform1f(glGetUniformLocation(program_->getProgramId(), getLightUniformName("coneAngle", i).c_str()), gLights[i].coneAngle);
    		glUniform3fv(glGetUniformLocation(program_->getProgramId(), getLightUniformName("coneDirection", i).c_str()),1, reinterpret_cast<GLfloat*>(&gLights[i].coneDirection));
    	}
    }

    std::string Scene::getLightUniformName(const char* propertyName, size_t lightIndex)
    {
        std::ostringstream ss;
        ss << "allLights[" << lightIndex << "]." << propertyName;
        return ss.str();
    }
    void Scene::updateMaterial(float shininess, glm::vec3 specularColor)
    {
    	glUniform1f(glGetUniformLocation(program_->getProgramId(),"material.shininess"),shininess);
    	glUniform3fv(glGetUniformLocation(program_->getProgramId(),"material.specularColor"),1, reinterpret_cast<GLfloat*>(&specularColor));
    }

    void Scene::renderSceneObjects(bool onlyDepth)
    {
    	for(int i = 0; i < m_sceneObjects.size();i++){
    		SceneObject* so = m_sceneObjects.at(i);

    		SceneObject::tShaderMode mode = so->getShaderMode();
    		if(onlyDepth){
    			switch (mode) {
				case SceneObject::tShaderMode::DEFAULT:
					mode = SceneObject::tShaderMode::DEFAULT_DEPTH;
					break;
				case SceneObject::tShaderMode::WATER:
					mode = SceneObject::tShaderMode::WATER_DEPTH;
					break;
				default:break;
				}
    		}
            glUniform1i(shaderModeUniformLocation_, mode);
            glm::mat4 mm = so->getModelMatrix();
            glUniformMatrix4fv(matModelUniformLocation_, 1, GL_FALSE, reinterpret_cast<GLfloat*>(&mm));
            glm::mat4 normalMatrix = glm::mat4(glm::mat3(mm));
            glUniformMatrix4fv(matNormalUniformLocation_, 1, GL_FALSE, reinterpret_cast<GLfloat*>(&normalMatrix));
            updateMaterial(so->getShininess(),so->getSpecularColor());
            so->bindTexturesAndDrawMesh();
    	}
    }
    void Scene::initShadowMapping()
    {
    	GLuint fbId, dtId;
    	for(int i = 0; i < gLights.size(); i++){
			// Setup buffer to render depth image
			glGenFramebuffers(1, &fbId);
			glGenTextures(1, &dtId);

			// Depth texture. Texture that contains the rendered depth image
			glBindTexture(GL_TEXTURE_2D, dtId);
			glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT24, config::windowWidth, config::windowHeight, 0,GL_DEPTH_COMPONENT, GL_FLOAT, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);


			// Attach a depth buffer (our texture) to the framebuffer
			glBindFramebuffer(GL_FRAMEBUFFER, fbId);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, dtId, 0);
			glDrawBuffer(GL_NONE); // No color buffer is drawn to.
			glReadBuffer(GL_NONE);

			frameBufferId_.push_back(fbId);
			depthTextureId_.push_back(dtId);

			if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
				std::cout << "Framebuffer not complete because of Error!" << std::endl;
				return exit(-1);
			}
    	}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		std::cout << "shadowMapping initialized: " << frameBufferId_.size() << " Buffers created!" << std::endl;
    }

    void Scene::renderDepthImage()
    {
    	// Calculate light view projection matrix
    	glm::mat4 depthPMatrix = glm::mat4(1.0f);

    	if(gLights.at(0).position.w == 0)
    		depthPMatrix = glm::ortho<float>(-10,10,-10,10,-10,20);
    	else
    	{
    		// TODO
			std::cout << "invalid light source" << std::endl;
    	}

    	glm::mat4 depthVMatrix = glm::lookAt(glm::vec3(gLights.at(0).position), glm::vec3(0,0,0), glm::vec3(0,1,0));
    	glm::mat4 depthVPMatrix = depthPMatrix*depthVMatrix;

        glUniformMatrix4fv(matVPUniformLocation_, 1, GL_FALSE, reinterpret_cast<GLfloat*>(&depthVPMatrix));

    	// Switch buffer to depth buffer
		glBindFramebuffer(GL_FRAMEBUFFER, frameBufferId_.at(0));
    	glViewport(0,0, config::windowWidth, config::windowHeight);
		glClear(GL_DEPTH_BUFFER_BIT);
		// Render scene into that buffer
        renderSceneObjects(true);
    }
    void Scene::renderRealImage()
    {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
    	glViewport(0,0, config::windowWidth, config::windowHeight);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, depthTextureId_.at(0));

        glUniformMatrix4fv(matVPUniformLocation_, 1, GL_FALSE, reinterpret_cast<GLfloat*>(&VPMatrix_));
		glm::mat4 depthPMatrix = glm::mat4(1.0f);

		if (gLights.at(0).position.w == 0)
			depthPMatrix = glm::ortho<float>(-10, 10, -10, 10, -10, 20);
		else {
			// TODO
			std::cout << "invalid light source" << std::endl;
		}

		glm::mat4 depthVMatrix = glm::lookAt(glm::vec3(gLights.at(0).position),	glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		glm::mat4 depthVPMatrix = depthPMatrix * depthVMatrix;
		glm::mat4 biasMatrix(
		 0.5, 0.0, 0.0, 0.0,
		 0.0, 0.5, 0.0, 0.0,
		 0.0, 0.0, 0.5, 0.0,
		 0.5, 0.5, 0.5, 1.0
		 );

		 glm::mat4 depthBiasMVP = biasMatrix*depthVPMatrix;

        glUniformMatrix4fv(matVPDepthUniformLocation_, 1, GL_FALSE, reinterpret_cast<GLfloat*>(&depthBiasMVP));

        renderSceneObjects();

    }
}
