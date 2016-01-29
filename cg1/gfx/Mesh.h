#pragma once

#include "cg1.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <map>
#include "Texture.h"

namespace cg1 {

    /** The vertex object used in these examples. */
    struct MeshVertex {
        /** The vertex position. */
        glm::vec4 position;
        /** The vertex normal. */
        glm::vec3 normal;
        /** The vertex texture coordinate. */
        glm::vec2 textureCoordinate;

		glm::vec3 tangent;
    };

    /**
    * Helper class for loading an OpenGL texture from file.
    */
    class Mesh final
    {
    public:
        explicit Mesh(const std::string& meshFilename);
        Mesh(const std::string& path, const aiMesh* mesh, const aiScene* scene);
        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;
        Mesh(Mesh&&) noexcept;
        Mesh& operator=(Mesh&&) noexcept;
        ~Mesh() noexcept;

        /**
         *  Accessor to the meshes sub-meshes. This can be used to render more complicated meshes (with multiple sets
         *  of texture coordinates).
         */
        std::vector<std::unique_ptr<Mesh>>& GetSubMeshes() { return subMeshes_; }
        /** Const accessor to the meshes sub-meshes. */
        const std::vector<std::unique_ptr<Mesh>>& GetSubMeshes() const { return subMeshes_; }

        void Draw() const;
        void DrawComplete() const;

    private:

        void parse(const std::string& path, const aiNode* node, const aiScene* scene);

        /** Holds all the meshes sub-meshes. */
        std::vector<std::unique_ptr<Mesh>> subMeshes_;
        /** Holds the meshes indices. */
        std::vector<GLuint> indices_;
        /** Holds the meshes vertices. */
        std::vector<MeshVertex> vertices_;

        /** Holds the OpenGL vertex array object. */
        GLuint vertexArray_;
        /** Holds the OpenGL vertex buffer. */
        GLuint vertexBuffer_;
        /** Holds the OpenGL index buffer. */
        GLuint indexBuffer_;
    };
}
