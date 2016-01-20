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
	public:
		// diffuse material color and ambient color is given by the texture
		struct MaterialAttributes {
		public:
			MaterialAttributes() : shininess(1.0), specularColor(glm::vec3(1.0, 1.0, 1.0)) {}
			virtual ~MaterialAttributes(){}
			GLfloat shininess;
			glm::vec3 specularColor;
		};
		MaterialAttributes materialAttributes;

        typedef enum {
        	DEFAULT = 0,
			WATER = 1,
			DEFAULT_DEPTH = 4,
			WATER_DEPTH = 5
        } tShaderMode;

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

		void setMaterialAttributes(GLfloat p_shininess, glm::vec3 p_specularColor);
		void setSpecularColor(glm::vec3 p_specularColor);
		void setShininess(GLfloat p_shininess);
		MaterialAttributes getMaterialAttributes();
		GLfloat getShininess();
		glm::vec3 getSpecularColor();
		void setShaderMode(tShaderMode mode){m_shaderMode = mode;}
		tShaderMode getShaderMode(){return m_shaderMode;}

	private:
		std::unique_ptr<Mesh> m_pMesh;
		std::vector<std::unique_ptr<Texture> > m_Textures;

		glm::mat4 m_ModelMatrix;
		tShaderMode m_shaderMode;
	};

}
