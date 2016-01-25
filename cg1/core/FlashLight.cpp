/*
 * FlashLight.cpp
 *
 *  Created on: 24.01.2016
 *      Author: andreas
 */

#include "FlashLight.h"
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

namespace cg1 {

FlashLight::FlashLight():SceneObject("torch.obj",{"torch_DIFFUSE.png"}){
	setMaterialAttributes(100,glm::vec3(1,1,1));
	setShaderMode(SceneObject::DEFAULT);
	m_light = NULL;
}

FlashLight::~FlashLight() {

}

void FlashLight::turnOn()
{
	m_light->att_c3 = 0.03f;
}
void FlashLight::turnOff()
{
	m_light->att_c3 = 100.0f;
}
void FlashLight::setLightRef(Scene::Light* light)
{
	if(light == NULL)
		return;
	m_light = light;

	m_light->position = m_ModelMatrix[3];
	m_light->intensities = glm::vec3(1.6, 1.2, 0.4); //strong yellow light
	m_light->att_c1 = 0;
	m_light->att_c2 = 0;
	m_light->att_c3 = 0.02f;
	m_light->ambientCoefficient = 0; //no ambient light
	m_light->coneAngle = 20.0f;
	m_light->coneDirection = glm::vec3(0, 0, -1);
}
void FlashLight::lookAt(glm::vec3 P, glm::vec3 C)
{
	if(m_light == NULL)
		return;

	glm::mat4 viewTransform = glm::lookAt(P,C,glm::vec3(0,1,0));

	glm::vec3 lookDir = glm::normalize(C-P);

	m_ModelMatrix = glm::inverse(viewTransform);

	m_light->position = glm::vec4(P,1);
	m_light->coneDirection = lookDir;

}
void FlashLight::setTransformation(glm::vec3 T, glm::vec3 RAxis, float angle, glm::vec3 S)
{
	std::cout << "Dont use setTransformation on flashlights!" << std::endl;
}

} /* namespace cg1 */
