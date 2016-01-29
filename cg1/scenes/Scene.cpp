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

#include "core/SceneObject.h"
#include "core/FlashLight.h"

#define printOpenGLError() printOglError(__FILE__, __LINE__)
#define COMMA ,

int printOglError(const char *file, int line)
{

    GLenum glErr;
    int    retCode = 0;

    glErr = glGetError();
    if (glErr != GL_NO_ERROR)
    {
        printf("glError in file %s @ line %d: %d\n",
			     file, line, glErr);
        retCode = 1;
    }
    return retCode;
}


namespace cg1 {

    /**
     *  Constructor.
     */
    Scene::Scene() :
		program_{ std::make_unique<GPUProgram>("DefaultProgram",
				std::initializer_list<std::string>{"sceneShader.vert", "sceneShader.frag"}) },
        VPMatrix_{ 1.0f },
		viewMatrix_{1.0f},
		currentTime_{0},
		shadowMapSize_{1024},
		depthTextureSlot{10},
		enableLighting_{true},
		enableShadowMapping_{true},
		enableBumpMapping_{true},
		enableWater_{true},
		enableFlashLights_{true},
		enableSmoothShadows_{false},
		waterMode_{0},
		lastUpdate_{0},
		lastFPS_{-1}
    {
    	m_sceneObjects.clear();
    	gLights.clear();

    	std::cout << "Loading Uniform Locations ..." << std::endl;
        matModelUniformLocation_ = glGetUniformLocation(program_->getProgramId(), "matModel");
        matNormalUniformLocation_ = glGetUniformLocation(program_->getProgramId(), "matNormal");
        matVPUniformLocation_ = glGetUniformLocation(program_->getProgramId(), "matVP");
        matVUniformLocation_ = glGetUniformLocation(program_->getProgramId(), "matV");
        tex0UniformLocation_ = glGetUniformLocation(program_->getProgramId(), "tex");
		tex1UniformLocation_ = glGetUniformLocation(program_->getProgramId(), "normalTex");
        shaderModeUniformLocation_ = glGetUniformLocation(program_->getProgramId(), "shaderMode");
        timeUniformLocation_ = glGetUniformLocation(program_->getProgramId(), "time");
        depthTextureArrayUniformLocation_ = glGetUniformLocation(program_->getProgramId(), "shadowTexArray");
        enableShadowMappingUniformLocation_ = glGetUniformLocation(program_->getProgramId(), "enableShadowMapping");
		enableBumpMappingUniformLocation_ = glGetUniformLocation(program_->getProgramId(), "enableBumpMapping");
		hasBumpMapUniformLocation_ = glGetUniformLocation(program_->getProgramId(), "hasBumpMap");
        enableLightingUniformLocation_ = glGetUniformLocation(program_->getProgramId(), "enableLighting");
        waterModeUniformLocation_ = glGetUniformLocation(program_->getProgramId(), "waterMode");
        materialSpecularColUniformLocation_ = glGetUniformLocation(program_->getProgramId(), "material.specularColor");
        materialShininessUniformLocation_ = glGetUniformLocation(program_->getProgramId(), "material.shininess");
        enableSmoothShadowsUniformLocation_ = glGetUniformLocation(program_->getProgramId(), "enableSmoothShadows");

    	std::cout << "Creating Scene Objects ..." << std::endl;
#define ADD_SCENE_OBJECT(OBJ_FILE, TEX_LIST,T,R_AXIS,R_ANGLE,S, SHININESS, SPEC_COLOR, SHADER_MODE, HAS_BUMP_MAP) {\
			SceneObject* obj = new SceneObject(OBJ_FILE,TEX_LIST); \
			obj->setTransformation(T,R_AXIS,R_ANGLE,S);\
			obj->setMaterialAttributes(SHININESS,SPEC_COLOR);\
			obj->setShaderMode(SHADER_MODE);\
			obj->setBumpMappingStatus(HAS_BUMP_MAP);\
			m_sceneObjects.push_back(obj);\
    	}
#define ADD_FLASHLIGHT(P,LOOK_AT) {\
			FlashLight* obj = new FlashLight(); \
			m_sceneObjects.push_back(obj);\
			Light* flashLight = new Light();\
			obj->setLightRef(flashLight);\
			obj->lookAt(P,LOOK_AT);\
			gLights.push_back(flashLight);\
    	}

        // Setup scene
        ADD_SCENE_OBJECT("terrainSurface.obj",{"terrain_DIFFUSE.jpg" COMMA "terrain_NORMAL.jpg"},
        		glm::vec3(0,0,0),glm::vec3(0,1,0),glm::radians(0.0f),glm::vec3(1,1,1),
				100,glm::vec3(1,1,1),
				SceneObject::DEFAULT, 
				1);
        ADD_SCENE_OBJECT("waterSurface.obj",{"water_DIFFUSE.jpg" COMMA "water_NORMAL.jpg"},
        		glm::vec3(0, -2.2, 0),glm::vec3(0,1,0),glm::radians(0.0f),glm::vec3(1,1,1),
				100,glm::vec3(1,1,1),
				SceneObject::WATER,
				1);

        ADD_SCENE_OBJECT("Stonehengebed.obj",{"192.jpg" COMMA "192_norm.jpg"},
        		glm::vec3(0, 1, 0),glm::vec3(0,1,0),glm::radians(0.0f),glm::vec3(1,1,1),
				100,glm::vec3(1,1,1),
				SceneObject::DEFAULT,
				1);


        ADD_SCENE_OBJECT("Road-Blocker.obj",{"RoadBlocker_diffuse.png"},
        		glm::vec3(1,1,5),glm::vec3(0,1,0),glm::radians(-45.0f),glm::vec3(0.2,0.2,0.2),
				100,glm::vec3(1,1,1),
				SceneObject::DEFAULT,
				0);
        ADD_SCENE_OBJECT("Road-Blocker.obj",{"RoadBlocker_diffuse.png"},
        		glm::vec3(0,1,5),glm::vec3(0,1,0),glm::radians(70.0f),glm::vec3(0.2,0.2,0.2),
				100,glm::vec3(1,1,1),
				SceneObject::DEFAULT,
				0);
        ADD_SCENE_OBJECT("Road-Blocker.obj",{"RoadBlocker_diffuse.png"},
        		glm::vec3(-1,1,5),glm::vec3(0,1,0),glm::radians(20.0f),glm::vec3(0.2,0.2,0.2),
				100,glm::vec3(1,1,1),
				SceneObject::DEFAULT,
				0);


    	std::cout << "Creating Lights ..." << std::endl;
        // setup lights
        // Sun
        Light* directionalLight = new Light();
        directionalLight->position = glm::vec4(10, 5, 5, 0); //w == 0 indications a directional light
        //directionalLight->intensities = glm::vec3(255,184,19)/255.0f; //weak yellowish light
        directionalLight->intensities = glm::vec3(1,1,1); //white light
        directionalLight->ambientCoefficient = 0.1;
        directionalLight->att_c1 = 0;
        directionalLight->att_c2 = 0;
        directionalLight->att_c3 = 0;
        gLights.push_back(directionalLight);

        // Moon
        Light* directionalLight2 = new Light();
        directionalLight2->position = glm::vec4(-10, -5, 5, 0); //w == 0 indications a directional light
        directionalLight2->intensities = glm::vec3(192,192,240)/255*0.2; //white light
        directionalLight2->ambientCoefficient = 0.1;
        directionalLight2->att_c1 = 0;
        directionalLight2->att_c2 = 0;
        directionalLight2->att_c3 = 0;
        gLights.push_back(directionalLight2);

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
//		spotlight.position = glm::vec4(0, 7, -10, 1);
//		spotlight.intensities = glm::vec3(2, 2, 2); //strong white light
//		spotlight.att_c1 = 0;
//		spotlight.att_c2 = 0;
//		spotlight.att_c3 = 0.1f;
//		spotlight.ambientCoefficient = 0; //no ambient light
//		spotlight.coneAngle = 20.0f;
//		spotlight.coneDirection = glm::vec3(0, -0.5, 1);
//		gLights.push_back(spotlight);


        //ADD_FLASHLIGHT(glm::rotateY(glm::vec3(7,2.5,0),glm::radians(-30.0f)), glm::vec3(0,1.5,0));
        ADD_FLASHLIGHT(glm::rotateY(glm::vec3(7,3,0),glm::radians(45.0f)), glm::vec3(0,1.5,0));
        ADD_FLASHLIGHT(glm::rotateY(glm::vec3(7,2.5,0),glm::radians(140.0f)), glm::vec3(0,1.5,0));
        ADD_FLASHLIGHT(glm::rotateY(glm::vec3(7,1.20,0),glm::radians(270.0f)), glm::vec3(0,2.25,0));

        std::cout << "Sceneobjects (meshes): " << m_sceneObjects.size() << std::endl;
        std::cout << "Light Sources: " << gLights.size() << std::endl;


        getLightUniformLocations();
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

    	for(int i = 0; i < gLights.size();i++){
    		delete gLights.at(i);
    	}
    	gLights.clear();
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
        lastUpdate_ = currentTime_;
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
            std::stringstream ss;
            if(lastFPS_ > 0)
            	lastFPS_ = lastFPS_*0.99 + 1/(currentTime_-lastUpdate_)*0.01;
            else
            	lastFPS_ = 1/(currentTime_-lastUpdate_);

            ss << "Framerate: " << (int)lastFPS_ << " FPS";
            ImGui::Text(ss.str().c_str());
            ImGui::Text("General Settings");
            ImGui::Checkbox("Enable Phong Lighting",&enableLighting_);
			ImGui::Checkbox("Enable Bumpmapping", &enableBumpMapping_);
            ImGui::Checkbox("Enable Shadow-Mapping",&enableShadowMapping_);
            ImGui::Checkbox("Enable Smooth shadows",&enableSmoothShadows_);
            ImGui::Checkbox("Enable Flashlights",&enableFlashLights_);
            ImGui::Text("Water Settings");
            ImGui::Checkbox("Enable Water",&enableWater_);
            ImGui::RadioButton("Small waves",&waterMode_,0);
            ImGui::RadioButton("Middle waves",&waterMode_,1);
            ImGui::RadioButton("High waves",&waterMode_,2);
            ImGui::End();
        }

        glUseProgram(program_->getProgramId());
        printOpenGLError();
        //////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Update general uniforms
        //////////////////////////////////////////////////////////////////////////////////////////////////////////
        glUniformMatrix4fv(matVUniformLocation_, 1, GL_FALSE, reinterpret_cast<GLfloat*>(&viewMatrix_));
        printOpenGLError();

        // Rotate sun and moon
		for(int i = 0; i < 2; i++){
			gLights.at(i)->position = glm::rotateZ(gLights.at(i)->position,(currentTime_-lastUpdate_)*0.1f);
		}

        enableLighting(enableLighting_);
        if(enableLighting_){

			for(int i = 0; i < m_sceneObjects.size();i++){
				if(m_sceneObjects.at(i)->getType() != SceneObject::tObjectType::TYPE_FLASH_LIGHT)
					continue;

				if(enableFlashLights_)
					((FlashLight*) m_sceneObjects.at(i))->turnOn();
				else
					((FlashLight*) m_sceneObjects.at(i))->turnOff();
			}
        }
		// Update Light
		updateLight(viewMatrix_);

        //////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Update Scene properties
        //////////////////////////////////////////////////////////////////////////////////////////////////////////
        glUniform1f(timeUniformLocation_,currentTime_);
        glUniform1i(waterModeUniformLocation_,waterMode_);
        glUniform1i(tex0UniformLocation_,0);
		glUniform1i(tex1UniformLocation_, 1);


        //////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Render Scene
        //////////////////////////////////////////////////////////////////////////////////////////////////////////
    	glUniform1i(depthTextureArrayUniformLocation_,depthTextureSlot);

		enableBumpMapping(enableBumpMapping_);
        enableShadowMapping(enableShadowMapping_, enableSmoothShadows_);
        if(enableShadowMapping_){
        	// Shadow mapping: Render to depth buffer
        	renderDepthImage();
        }

        renderRealImage();

        glUseProgram(0);
    }

    /**
     *  Updates uniforms for light calculations in the shader.
     */
    void Scene::updateLight(glm::mat4 viewMat)
    {
    	glUniform1i(numLightsUniformLocation_, (int)gLights.size());

    	for(size_t i = 0; i < gLights.size(); ++i){
    		glm::vec4 posViewSpace = viewMat*gLights[i]->position;
    		glm::vec3 dirViewSpace = glm::mat3(viewMat)*gLights[i]->coneDirection;
    		glUniform4fv(gLights[i]->positionLoc, 1, reinterpret_cast<GLfloat*>(&posViewSpace));
    		printOpenGLError();
    		glUniform3fv(gLights[i]->intensitiesLoc ,1, reinterpret_cast<GLfloat*>(&gLights[i]->intensities));
    		printOpenGLError();
    		glUniform1f(gLights[i]->att_c1Loc, gLights[i]->att_c1);
    		printOpenGLError();
    		glUniform1f(gLights[i]->att_c2Loc, gLights[i]->att_c2);
    		printOpenGLError();
    		glUniform1f(gLights[i]->att_c3Loc, gLights[i]->att_c3);
    		printOpenGLError();
    		glUniform1f(gLights[i]->ambientCoefficientLoc, gLights[i]->ambientCoefficient);
    		printOpenGLError();
    		glUniform1f(gLights[i]->coneAngleLoc, gLights[i]->coneAngle);
    		printOpenGLError();
    		glUniform3fv(gLights[i]->coneDirectionLoc,1, reinterpret_cast<GLfloat*>(&dirViewSpace));
    		printOpenGLError();
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
    	glUniform1f(materialShininessUniformLocation_,shininess);
		printOpenGLError();
    	glUniform3fv(materialSpecularColUniformLocation_,1, reinterpret_cast<GLfloat*>(&specularColor));
		printOpenGLError();
    }

    void Scene::renderSceneObjects(bool onlyDepth)
    {
    	for(int i = 0; i < m_sceneObjects.size();i++){
//    		std::cout << "render mesh " << i << std::endl;
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

    		if(!enableWater_){
    			switch (mode) {
				case SceneObject::tShaderMode::WATER:
				case SceneObject::tShaderMode::WATER_DEPTH:
					mode = SceneObject::tShaderMode::DEFAULT;
					break;
				default:break;
				}
    		}

			glUniform1i(hasBumpMapUniformLocation_, so->getBumpMappingStatus());
            glUniform1i(shaderModeUniformLocation_, mode);
    		printOpenGLError();
            glm::mat4 mm = so->getModelMatrix();
            glUniformMatrix4fv(matModelUniformLocation_, 1, GL_FALSE, reinterpret_cast<GLfloat*>(&mm));
    		printOpenGLError();
            glm::mat4 normalMatrix = glm::mat4(glm::mat3(mm));
            glUniformMatrix4fv(matNormalUniformLocation_, 1, GL_FALSE, reinterpret_cast<GLfloat*>(&normalMatrix));
    		printOpenGLError();
            updateMaterial(so->getShininess(),so->getSpecularColor());
            so->bindTexturesAndDrawMesh();
    	}
    }
    void Scene::initShadowMapping()
    {
    	std::cout << "Initialize shadow mapping ..." << std::endl;

    	std::cout << "Generate shadow map textures ...";
    	GLuint fbId;
    	// Generate texture array
		glGenTextures(1, &depthTextureArrayId_);
        printOpenGLError();
        glActiveTexture(GL_TEXTURE0 + depthTextureSlot);
        printOpenGLError();
		glBindTexture(GL_TEXTURE_2D_ARRAY, depthTextureArrayId_);
        printOpenGLError();
		glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH_COMPONENT24, shadowMapSize_, shadowMapSize_, gLights.size());
        printOpenGLError();
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		printOpenGLError();
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		printOpenGLError();
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		printOpenGLError();
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		printOpenGLError();
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_NONE);
		printOpenGLError();
        std::cout << "Done." << std::endl;

    	std::cout << "Generate Framebuffers ...";
    	for(int i = 0; i < gLights.size(); i++){
			// Setup buffer to render depth image
			glGenFramebuffers(1, &fbId);
	        printOpenGLError();

			// Attach a depth buffer (our texture) to the framebuffer
			glBindFramebuffer(GL_FRAMEBUFFER, fbId);
	        printOpenGLError();
			glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTextureArrayId_, 0, i);		// Attach texture i of texture array to framebuffer
	        printOpenGLError();
			glDrawBuffer(GL_NONE); // No color buffer is drawn to.
	        printOpenGLError();
			glReadBuffer(GL_NONE);
	        printOpenGLError();

			frameBufferId_.push_back(fbId);

			if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
				std::cout << "Framebuffer not complete because of Error!" << std::endl;
				return exit(-1);
			}
    	}
        std::cout << "Done." << std::endl;

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
        printOpenGLError();

		std::cout << "shadowMapping initialized: " << frameBufferId_.size() << " Buffers created!" << std::endl;
    }

    void Scene::renderDepthImage()
    {
		for(int i = 0; i < gLights.size(); i++){

			glm::mat4 depthVPMatrix = calculateDepthVPMat(i);
			std::ostringstream ss;
			ss << "matDepthVP[" << i << "]";

			glUniformMatrix4fv(matVPUniformLocation_, 1, GL_FALSE, reinterpret_cast<GLfloat*>(&depthVPMatrix));

			// Switch buffer to depth buffer
			glBindFramebuffer(GL_FRAMEBUFFER, frameBufferId_.at(i));
	        printOpenGLError();
			glViewport(0,0, shadowMapSize_, shadowMapSize_);
	        printOpenGLError();
			glClear(GL_DEPTH_BUFFER_BIT);
	        printOpenGLError();
			// Render scene into that buffer
			renderSceneObjects(true);
		}

    }
    void Scene::renderRealImage()
    {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
        printOpenGLError();
    	glViewport(0,0, config::windowWidth, config::windowHeight);
        printOpenGLError();
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        printOpenGLError();
		glActiveTexture(GL_TEXTURE0 + depthTextureSlot);
        printOpenGLError();
		glBindTexture(GL_TEXTURE_2D_ARRAY, depthTextureArrayId_);
        printOpenGLError();

        glUniformMatrix4fv(matVPUniformLocation_, 1, GL_FALSE, reinterpret_cast<GLfloat*>(&VPMatrix_));

        if(enableShadowMapping_){
			for(int i = 0; i < gLights.size(); i++){
				glm::mat4 depthVPMatrix = calculateDepthVPMat(i);
				glm::mat4 biasMatrix(
				 0.5, 0.0, 0.0, 0.0,
				 0.0, 0.5, 0.0, 0.0,
				 0.0, 0.0, 0.5, 0.0,
				 0.5, 0.5, 0.5, 1.0
				 );

				 glm::mat4 depthBiasMVP = biasMatrix*depthVPMatrix;

				std::ostringstream ss;
				ss << "matDepthVP[" << i << "]";

				glUniformMatrix4fv(glGetUniformLocation(program_->getProgramId(), ss.str().c_str()), 1, GL_FALSE, reinterpret_cast<GLfloat*>(&depthBiasMVP));
			}
        }
        renderSceneObjects();

    }
    void Scene::enableShadowMapping(bool enable, bool smooth)
    {
        glUniform1i(enableShadowMappingUniformLocation_, enable?1:0);
        glUniform1i(enableSmoothShadowsUniformLocation_, smooth?1:0);
		printOpenGLError();
        enableShadowMapping_ = enable;
    }

	void Scene::enableBumpMapping(bool enable)
	{
		glUniform1i(enableBumpMappingUniformLocation_, enable ? 1 : 0);
		printOpenGLError();
	}

    void Scene::enableLighting(bool enable)
    {
        glUniform1i(enableLightingUniformLocation_, enable?1:0);
		printOpenGLError();
        enableLighting_ = enable;
    }
    glm::mat4 Scene::calculateDepthVPMat(int lightIdx)
    {
    	glm::mat4 depthPMatrix = glm::mat4(1.0f);
		glm::vec3 lookAt = glm::vec3(0, 0, 0);
		// Directional
		if (gLights.at(lightIdx)->position.w == 0){
			depthPMatrix = glm::ortho<float>(-20, 20, -20, 20, -10, 30);
		}
		// Spot light
		else if (gLights.at(lightIdx)->coneAngle < 180) {
			depthPMatrix = glm::perspective<float>(gLights.at(lightIdx)->coneAngle,1,0.1f,30);
			lookAt = glm::vec3(gLights.at(lightIdx)->position) + gLights.at(lightIdx)->coneDirection;
		}
		// Point light
		else {
			std::cout << "invalid light source" << std::endl;
		}

		glm::mat4 depthVMatrix = glm::lookAt(glm::vec3(gLights.at(lightIdx)->position), lookAt, glm::vec3(0, 1, 0));
		glm::mat4 depthVPMatrix = depthPMatrix * depthVMatrix;
		return depthVPMatrix;

    }
    void Scene::getLightUniformLocations()
    {
    	numLightsUniformLocation_ = glGetUniformLocation(program_->getProgramId(), "numLights");
    	for(size_t i = 0; i < gLights.size(); ++i){
    		Light* l = gLights.at(i);
    		l->positionLoc = glGetUniformLocation(program_->getProgramId(), getLightUniformName("position", i).c_str());
    		l->intensitiesLoc = glGetUniformLocation(program_->getProgramId(), getLightUniformName("intensities", i).c_str());
    		l->att_c1Loc = glGetUniformLocation(program_->getProgramId(), getLightUniformName("att_c1", i).c_str());
    		l->att_c2Loc = glGetUniformLocation(program_->getProgramId(), getLightUniformName("att_c2", i).c_str());
    		l->att_c3Loc = glGetUniformLocation(program_->getProgramId(), getLightUniformName("att_c3", i).c_str());
    		l->ambientCoefficientLoc = glGetUniformLocation(program_->getProgramId(), getLightUniformName("ambientCoefficient", i).c_str());
    		l->coneAngleLoc = glGetUniformLocation(program_->getProgramId(), getLightUniformName("coneAngle", i).c_str());
    		l->coneDirectionLoc = glGetUniformLocation(program_->getProgramId(), getLightUniformName("coneDirection", i).c_str());
    	}
    }
}
