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
	
	static const std::string PATH_MESHES = "meshes";
	static const std::string PATH_TEXTURES = "textures";

	// this class manages one object in the scene. An object consists of a mesh and multiple textures.
	class SceneObject
	{
	private:
		std::unique_ptr<Mesh> m_pMesh;
		std::vector<std::unique_ptr<Texture> > m_Textures;

		glm::mat4 m_ModelMatrix;
	public:
		SceneObject();
		SceneObject(const std::string& mesh, std::initializer_list<std::string> textures);
		~SceneObject();

		void bindTexturesAndDrawMesh();

		// transformations for m_pModelMatrix
		void translate(glm::vec3 direction);
		// anlge is in radian
		void rotate(GLfloat angle, glm::vec3 axis = glm::vec3(0.0, 1.0, 0.0));
		void scale(glm::vec3 factors);

		glm::mat4 getModelMatrix();
	};

}