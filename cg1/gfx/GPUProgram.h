#pragma once

#include "cg1.h"

namespace cg1 {

    class Shader;

    /**
     * Complete GPU program with multiple Shader objects working together.
     */
    class GPUProgram final
    {
    public:
        GPUProgram(const std::string& programName, std::initializer_list<std::string> shaderNames);
        GPUProgram(const GPUProgram& orig) = delete;
        GPUProgram& operator=(const GPUProgram&) = delete;
        GPUProgram(GPUProgram&&) noexcept;
        GPUProgram& operator=(GPUProgram&&) noexcept;
        ~GPUProgram() noexcept;

        void recompileProgram();
        /** Returns the OpenGL program id. */
        GLuint getProgramId() const noexcept { return program_; }

    private:
        using ShaderList = std::vector<std::unique_ptr<Shader>>;

        /** Holds the program name. */
        std::string programName_;
        /** Holds the shader names. */
        std::vector<std::string> shaderNames_;
        /** Holds the program. */
        GLuint program_;
        /** Holds a list of shaders used internally. */
        ShaderList shaders_;

        void unload() noexcept;
        template<typename T, typename SHAcc> static GLuint linkNewProgram(const std::string& name,
            const std::vector<T>& shaders, SHAcc shaderAccessor);
        static void releaseShaders(const std::vector<GLuint>& shaders) noexcept;
    };
}
