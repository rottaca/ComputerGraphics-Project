#pragma once

#include "cg1.h"
#include <glm/glm.hpp>
#include "core/Camera.h"
#include "core/FreeCamera.h"
#include "core/SceneObject.h"

namespace cg1 {

    class GPUProgram;
    class Camera;
    class Mesh;
    class Texture;

    // TODO: select the camera model for the application here. [1/13/2016 Sebastian Maisch]
    // also: if camera model is free, GUI should be disabled.
    using CG1Camera = Camera;
    constexpr bool DRAW_GUI = true;
    //using CG1Camera = FreeCamera;
    //constexpr bool DRAW_GUI = false;

    class Scene
    {
    public:
        Scene();
        ~Scene();

        void renderScene();
        void updateScene(const CG1Camera& camera, double currentTime, double elapsedTime) noexcept;

        typedef enum {
        	DEFAULT = 0,
			WATER = 1
        } tShaderMode;

        // Private member functions
    private:
        /* Updates uniforms for light calculations in the shader. */
        void updateLight();
        std::string getLightUniformName(const char* propertyName, size_t lightIndex);
        void updateMaterial(float shininess, glm::vec3 specularColor);

        void renderSceneObjects();

        void initShadowMapping();

        // Private member variables
    private:
        /** Holds the scenes GPU program. */
        std::unique_ptr<GPUProgram> program_;

        struct Light {
            glm::vec4 position;
            glm::vec3 intensities; //a.k.a. the color of the light
            float att_c1;
            float att_c2;
            float att_c3;
            float ambientCoefficient;
            float coneAngle;
            glm::vec3 coneDirection;
        };
        std::vector<Light> gLights;



        /////////////////////////////////////////////////////////////////////////////////////////////////////
        // Uniforms
        /////////////////////////////////////////////////////////////////////////////////////////////////////
        /** Holds uniform name for the model matrix. */
        GLint matModelUniformLocation_;
        /** Holds uniform name for the normal matrix. */
        GLint matNormalUniformLocation_;
        /** Holds uniform name for the view-projection matrix. */
        GLint matVPUniformLocation_;
        GLint cameraPosUniformLocation_;

        GLint tex0UniformLocation_;

        GLint shaderModeUniformLocation_;
        GLint timeUniformLocation_;


        /////////////////////////////////////////////////////////////////////////////////////////////////////
        //
        /////////////////////////////////////////////////////////////////////////////////////////////////////
        /** Holds the normal matrix. */
        glm::mat4 normalMatrix_;
        /** Holds the VP matrix. */
        glm::mat4 VPMatrix_;
        /* Holds the view matrix.*/
        glm::mat4 viewMatrix_;
        glm::vec3 camPos_;

        /////////////////////////////////////////////////////////////////////////////////////////////////////
        //
        /////////////////////////////////////////////////////////////////////////////////////////////////////
        float currentTime_;



        glm::mat4 modelMatrixTerrain_;
        SceneObject* objTerrain_;
        glm::mat4 modelMatrixWater_;
        SceneObject* objWater_;
        glm::mat4 modelMatrixStoneHenge_;
        SceneObject* objStoneHenge_;

        std::vector<SceneObject> m_sceneObjects;

        /////////////////////////////////////////////////////////////////////////////////////////////////////
        // Shadow Mapping
        // Source: http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-16-shadow-mapping/
        /////////////////////////////////////////////////////////////////////////////////////////////////////
        GLuint frameBufferId_;
        GLuint depthTextureId_;


    };
}
