#include "SceneObject.h"
#include <glm/glm.hpp>
#include <type_traits>
#include "../gfx/GPUProgram.h"
#include "../core/Camera.h"
#include "../gfx/Texture.h"
#include "../gfx/Mesh.h"
#include <imgui.h>

namespace cg1{


SceneObject::SceneObject()
{
}


SceneObject::~SceneObject()
{
}

SceneObject::SceneObject(const std::string& mesh, std::initializer_list<std::string> textures) {
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

}
