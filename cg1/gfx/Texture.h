#pragma once

#include "cg1.h"
#include <glm/glm.hpp>

namespace cg1 {

    /** Describes the format of a texture. */
    struct TextureDescriptor
    {
        TextureDescriptor(unsigned int btsPP, GLint intFmt, GLenum fmt, GLenum tp) noexcept : bytesPP_(btsPP), internalFormat_(intFmt), format_(fmt), type_(tp) {};

        /** Holds the bytes per pixel of the format. */
        unsigned int bytesPP_;
        /** Holds the internal format. */
        GLint internalFormat_;
        /** Holds the format. */
        GLenum format_;
        /** Holds the type. */
        GLenum type_;
    };

    /**
    * Helper class for loading an OpenGL texture from file.
    */
    class Texture final
    {
    public:
        Texture(const std::string& texFilename);
        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;
        Texture(Texture&&) noexcept;
        Texture& operator=(Texture&&) noexcept;
        ~Texture() noexcept;

        /** Returns the size of the texture. */
        glm::uvec2 getDimensions() const noexcept { return glm::uvec2(width_, height_); }
        /** Returns the OpenGL texture id. */
        GLuint getTextureId() const noexcept { return textureId_; }

    private:
        /** Holds the OpenGL texture id. */
        GLuint textureId_;
        /** Holds the texture descriptor. */
        TextureDescriptor descriptor_;

        /** Holds the width. */
        unsigned int width_;
        /** Holds the height. */
        unsigned int height_;
    };
}
