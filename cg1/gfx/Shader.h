#pragma once

#include "cg1.h"

namespace cg1 {
    class GPUProgram;

    /**
     * Exception class for shader compiler errors.
     */
    class shader_compiler_error : public std::exception
    {
    public:
        shader_compiler_error(const std::string& shader, const std::string& errors) noexcept;
        shader_compiler_error(const shader_compiler_error&) noexcept;
        shader_compiler_error& operator=(const shader_compiler_error&) noexcept;
        shader_compiler_error(shader_compiler_error&&) noexcept;
        shader_compiler_error& operator=(shader_compiler_error&&) noexcept;
        virtual const char* what() const noexcept override;

    private:
        /** Holds the shader name. */
        std::vector<char> shr_;
        /** Holds the errors. */
        std::vector<char> errs_;
        /** holds the error message. */
        std::vector<char> myWhat_;
    };

    /**
     * Wrapper class for shader loading.
     */
    class Shader final
    {
    public:
        Shader(const std::string& shaderFilename);
        Shader(const Shader& orig) = delete;
        Shader& operator=(const Shader&) = delete;
        Shader(Shader&& orig) noexcept;
        Shader& operator=(Shader&& orig) noexcept;
        ~Shader() noexcept;

        /** Returns the OpenGL id of the shader. */
        GLuint getShaderId() const noexcept { return shader_; }
        void resetShader(GLuint newShader);
        GLuint recompileShader();

    private:
        /** Holds the shader file name. */
        std::string filename_;
        /** Holds the compiled shader. */
        GLuint shader_;
        /** Holds the shader type. */
        GLenum type_;
        /** Holds the shader type as a string. */
        std::string strType_;

        static GLuint compileShader(const std::string& filename, GLenum type, const std::string& strType);
        void unload() noexcept;
    };
}
