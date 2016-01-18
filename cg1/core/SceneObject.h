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
	public:
		SceneObject();
		SceneObject(const std::string& mesh, std::initializer_list<std::string> textures);
		~SceneObject();

		void bindTexturesAndDrawMesh();
	};

}