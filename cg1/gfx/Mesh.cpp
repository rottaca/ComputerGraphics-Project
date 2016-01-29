#include "Mesh.h"

namespace cg1 {

    /**
     * Constructor, creates a mesh from file.
     * @param meshFilename the filename of the mesh file.
     */
    Mesh::Mesh(const std::string& meshFilename) :
        subMeshes_(),
        indices_(),
        vertices_(),
        vertexArray_(0),
        vertexBuffer_(0),
        indexBuffer_(0)
    {
        std::string fullFilename = config::resourceBasePath + meshFilename;
        // Load a Model from File
        Assimp::Importer loader;
        aiScene const * scene = loader.ReadFile(fullFilename,
            aiProcessPreset_TargetRealtime_MaxQuality |
            aiProcess_OptimizeGraph |
            aiProcess_FlipUVs);

        // Walk the Tree of Scene Nodes
        auto index = meshFilename.find_last_of("/");
        if (!scene) fprintf(stderr, "%s\n", loader.GetErrorString());
        else parse(meshFilename.substr(0, index), scene->mRootNode, scene);
    }

    /**
     *  Move constructor.
     */
    Mesh::Mesh(Mesh&& rhs) noexcept :
        subMeshes_(std::move(rhs.subMeshes_)),
        indices_(std::move(rhs.indices_)),
        vertices_(std::move(rhs.vertices_)),
        vertexArray_(std::move(rhs.vertexArray_)),
        vertexBuffer_(std::move(rhs.vertexBuffer_)),
        indexBuffer_(std::move(rhs.indexBuffer_))
    {
        rhs.vertexArray_ = 0;
        rhs.vertexBuffer_ = 0;
        rhs.indexBuffer_ = 0;
    }

    /**
     *  Move assignment operator.
     */
    Mesh& Mesh::operator=(Mesh&& rhs) noexcept
    {
        if (this != &rhs) {
            this->~Mesh();
            subMeshes_ = std::move(rhs.subMeshes_);
            indices_ = std::move(rhs.indices_);
            vertices_ = std::move(rhs.vertices_);
            vertexArray_ = std::move(rhs.vertexArray_);
            vertexBuffer_ = std::move(rhs.vertexBuffer_);
            indexBuffer_ = std::move(rhs.indexBuffer_);
            rhs.vertexArray_ = 0;
            rhs.vertexBuffer_ = 0;
            rhs.indexBuffer_ = 0;
        }
        return *this;
    }

    /** Destructor. */
    Mesh::~Mesh() noexcept
    {
        if (indexBuffer_ != 0) {
            glDeleteBuffers(1, &indexBuffer_);
            indexBuffer_ = 0;
        }

        if (vertexBuffer_ != 0) {
            glDeleteBuffers(1, &vertexBuffer_);
            vertexBuffer_ = 0;
        }

        if (vertexArray_ != 0) {
            glDeleteBuffers(1, &vertexArray_);
            vertexArray_ = 0;
        }
    }


    Mesh::Mesh(const std::string& path, const aiMesh* mesh, const aiScene* scene) :
        subMeshes_(),
        indices_(),
        vertices_(),
        vertexArray_(0),
        vertexBuffer_(0),
        indexBuffer_(0)
    {
        // Create Vertex Data from Mesh Node
        MeshVertex vertex;
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            if (mesh->mTextureCoords[0])
                vertex.textureCoordinate = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
            vertex.position = glm::vec4(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z, 1.0f);
            vertex.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
            vertices_.push_back(vertex);
        }

        // Create Mesh Indices for Indexed Drawing
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
            for (unsigned int j = 0; j < mesh->mFaces[i].mNumIndices; j++)
                indices_.push_back(mesh->mFaces[i].mIndices[j]);

		MeshVertex *v0, *v1, *v2;
		float r;
		for (unsigned int i = 0; i < indices_.size(); i+=3) {
			v0 = &vertices_.at(indices_.at(i));
			v1 = &vertices_.at(indices_.at(i+1));
			v2 = &vertices_.at(indices_.at(i+2));

			glm::vec3 deltaPos1 = glm::vec3(v1->position - v0->position);
			glm::vec3 deltaPos2 = glm::vec3(v2->position - v0->position);

			glm::vec2 deltaUV1 = glm::vec2(v1->textureCoordinate - v0->textureCoordinate);
			glm::vec2 deltaUV2 = glm::vec2(v2->textureCoordinate - v0->textureCoordinate);

			r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
			v0->tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y)*r;
			v1->tangent = v0->tangent;
			v2->tangent = v0->tangent;
		}

        // Bind a Vertex Array Object
        glGenVertexArrays(1, &vertexArray_);
        glBindVertexArray(vertexArray_);

        // Copy Vertex Buffer Data
        glGenBuffers(1, &vertexBuffer_);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer_);
        glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(MeshVertex), vertices_.data(), GL_STATIC_DRAW);

        // Copy Index Buffer Data
        glGenBuffers(1, &indexBuffer_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * sizeof(GLuint), indices_.data(), GL_STATIC_DRAW);

        // Set Shader Attributes
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), reinterpret_cast<GLvoid*>(offsetof(MeshVertex, position)));
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), reinterpret_cast<GLvoid*>(offsetof(MeshVertex, normal)));
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), reinterpret_cast<GLvoid*>(offsetof(MeshVertex, textureCoordinate)));
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), reinterpret_cast<GLvoid*>(offsetof(MeshVertex, tangent)));
        glEnableVertexAttribArray(0); // Vertex Positions
        glEnableVertexAttribArray(1); // Vertex Normals
        glEnableVertexAttribArray(2); // Vertex UVs
		glEnableVertexAttribArray(3); // tangent

        glBindVertexArray(0);
        glDeleteBuffers(1, &vertexBuffer_);
        glDeleteBuffers(1, &indexBuffer_);
    }

    /**
     *  Draws the current mesh without rendering its sub-meshes.
     */
    void Mesh::Draw() const
    {
        if (vertexArray_ != 0) {
            glBindVertexArray(vertexArray_);
            glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices_.size()), GL_UNSIGNED_INT, nullptr);
        }
    }

    /** Draws the whole hierarchy of this mesh with sub-meshes in one go. */
    void Mesh::DrawComplete() const
    {
        for (auto &i : subMeshes_) i->DrawComplete();
        Draw();
    }

    void Mesh::parse(const std::string& path, const aiNode* node, const aiScene* scene)
    {
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
            subMeshes_.emplace_back(std::move(std::make_unique<Mesh>(path, scene->mMeshes[node->mMeshes[i]], scene)));
        for (unsigned int i = 0; i < node->mNumChildren; i++)
            parse(path, node->mChildren[i], scene);
    }
}