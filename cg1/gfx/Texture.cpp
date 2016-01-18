#define STB_IMAGE_IMPLEMENTATION

#include "Texture.h"
#include <iostream>

#undef min
#undef max

namespace cg1 {

    /**
     * Constructor, creates a texture from file.
     * @param texFilename the filename of the texture file.
     */
    Texture::Texture(const std::string& texFilename) :
        textureId_{ 0 },
        descriptor_{ 0, GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE },
        width_{ 0 },
        height_{ 0 }
    {
        std::string fullFilename = config::resourceBasePath + texFilename;
        int width = 0, height = 0, channels = 0;
        unsigned char * image = stbi_load(fullFilename.c_str(), &width, &height, &channels, 0);
        if (!image) std::cerr << "Failed to Load Texture (" << fullFilename << ")." << std::endl;

        // Set the Correct Channel Format
        switch (channels)
        {
        case 1:
            descriptor_.internalFormat_ = GL_R8;
            descriptor_.format_ = GL_RED;
            break;
        case 2:
            descriptor_.internalFormat_ = GL_RG8;
            descriptor_.format_ = GL_RG;
            break;
        case 3:
            descriptor_.internalFormat_ = GL_RGB8;
            descriptor_.format_ = GL_RGB;
            break;
        case 4:
            descriptor_.internalFormat_ = GL_RGBA8;
            descriptor_.format_ = GL_RGBA;
            break;
        }

        // Bind Texture and Set Filtering Levels
        glGenTextures(1, &textureId_);
        glBindTexture(GL_TEXTURE_2D, textureId_);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, descriptor_.internalFormat_, width, height, 0, descriptor_.format_, descriptor_.type_, image);

        // Release Image Pointer and Store the Texture
        stbi_image_free(image);
    }

    /**
     *  Move-constructor.
     *  @param rhs the object to copy.
     */
    Texture::Texture(Texture&& rhs) noexcept :
        textureId_{ std::move(rhs.textureId_) },
        descriptor_{ std::move(rhs.descriptor_) },
        width_{ std::move(rhs.width_) },
        height_{ std::move(rhs.height_) }
    {
        rhs.textureId_ = 0;
    }

    /**
     *  Move-assignment operator.
     *  @param rhs the object to assign.
     *  @return reference to this object.
     */
    Texture& Texture::operator=(Texture&& rhs) noexcept
    {
        if (this != &rhs) {
            this->~Texture();
            textureId_ = std::move(rhs.textureId_);
            descriptor_ = std::move(rhs.descriptor_);
            width_ = std::move(rhs.width_);
            height_ = std::move(rhs.height_);
            rhs.textureId_ = 0;
        }
        return *this;
    }

    /** Destructor. */
    Texture::~Texture() noexcept
    {
        if (textureId_ != 0) {
            glBindTexture(GL_TEXTURE_2D, 0);
            glDeleteTextures(1, &textureId_);
            textureId_ = 0;
        }
    }
}