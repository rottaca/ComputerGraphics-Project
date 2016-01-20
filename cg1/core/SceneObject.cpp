#include "SceneObject.h"
#include <glm/glm.hpp>
#include <type_traits>
#include "../gfx/GPUProgram.h"
#include "../core/Camera.h"
#include "../gfx/Texture.h"
#include "../gfx/Mesh.h"
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>

namespace cg1{


SceneObject::SceneObject()
{
}


SceneObject::~SceneObject()
{
}

SceneObject::SceneObject(const std::string& mesh, std::initializer_list<std::string> textures) : m_ModelMatrix(glm::mat4(1.0)){
	m_pMesh = std::make_unique<Mesh>(PATH_MESHES + "/" + mesh);
	for (std::initializer_list<std::string>::iterator it = textures.begin(); it != textures.end(); ++it) {
		m_Textures.push_back(std::make_unique<Texture>(PATH_TEXTURES + "/" + *it));
	}
}

void SceneObject::bindTexturesAndDrawMesh() {
	int i = 0;
	for (std::vector<std::unique_ptr<Texture> >::iterator it = m_Textures.begin(); it != m_Textures.end(); ++it) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, (*it)->getTextureId());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		++i;
	}
	m_pMesh->DrawComplete();
}

void SceneObject::translate(glm::vec3 direction) {
	m_ModelMatrix = glm::translate(m_ModelMatrix, direction);
}

void SceneObject::rotate(GLfloat angle, glm::vec3 axis) {
	m_ModelMatrix = glm::rotate(m_ModelMatrix, angle, axis);
}

void SceneObject::scale(glm::vec3 factors) {
	m_ModelMatrix = glm::scale(m_ModelMatrix, factors);
}

glm::mat4 SceneObject::getModelMatrix() {
	return m_ModelMatrix;
}

void SceneObject::setMaterialAttributes(GLfloat p_shininess, glm::vec3 p_specularColor) {
	materialAttributes.shininess = p_shininess;
	materialAttributes.specularColor = p_specularColor;
}

void SceneObject::setSpecularColor(glm::vec3 p_specularColor) {
	materialAttributes.specularColor = p_specularColor;
}

void SceneObject::setShininess(GLfloat p_shininess) {
	materialAttributes.shininess = p_shininess;
}
GLfloat SceneObject::getShininess() {
	return materialAttributes.shininess;
}

SceneObject::MaterialAttributes SceneObject::getMaterialAttributes() {
	return materialAttributes;
}
glm::vec3 SceneObject::getSpecularColor() {
	return materialAttributes.specularColor;
}

}
