#pragma once

#include "cg1.h"
#include <glm/glm.hpp>
#include "core/Camera.h"
#include "core/FreeCamera.h"

namespace cg1 {

    class GPUProgram;
    class Camera;
    class Mesh;
    class Texture;

    // TODO: select the camera model for the application here. [1/13/2016 Sebastian Maisch]
    // also: if camera model is free, GUI should be disabled.
    using CG1Camera = Camera;
    constexpr bool DRAW_GUI = true;
    // using CG1Camera = FreeCamera;
    // constexpr bool DRAW_GUI = false;    

    class Scene
    {
    public:
        Scene();
        ~Scene();

        void renderScene();
        void updateScene(const CG1Camera& camera, double currentTime, double elapsedTime) noexcept;

    private:
        /** Holds the scenes GPU program. */
        std::unique_ptr<GPUProgram> program_;
        /** Holds the mesh. */
        std::unique_ptr<Mesh> mesh_;
        /** Holds the diffuse texture. */
        std::unique_ptr<Texture> diffuseTex_;
        /** Holds the height texture. */
        std::unique_ptr<Texture> heightTex_;

        /** Holds uniform name for the model matrix. */
        GLint matModelUniformLocation_;
        /** Holds uniform name for the normal matrix. */
        GLint matNormalUniformLocation_;
        /** Holds uniform name for the model-view-projection matrix. */
        GLint matMVPUniformLocation_;
        /** Holds uniform name for the diffuse texture. */
        GLint texDiffuseUniformLocation_;
        /** Holds uniform name for the height texture. */
        GLint texHeightUniformLocation_;
        /** Holds uniform name for the height color. */
        GLint vecHeightColorUniformLocation_;

        /** Holds the model matrix. */
        glm::mat4 modelMatrix_;
        /** Holds the normal matrix. */
        glm::mat4 normalMatrix_;
        /** Holds the MVP matrix. */
        glm::mat4 MVPMatrix_;

        /** Holds the color value changed by the ant tweak bar. */
        glm::vec3 heightColor;
    };
}
