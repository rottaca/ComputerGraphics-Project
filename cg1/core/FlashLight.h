/*
 * FlashLight.h
 *
 *  Created on: 24.01.2016
 *      Author: andreas
 */

#ifndef FOLDER_CG1_CORE_FLASHLIGHT_H_
#define FOLDER_CG1_CORE_FLASHLIGHT_H_
#include "SceneObject.h"
#include "../scenes/Scene.h"

namespace cg1 {

class FlashLight : public SceneObject{
public:
	FlashLight();
	virtual ~FlashLight();

	tObjectType getType(){return SceneObject::tObjectType::TYPE_FLASH_LIGHT;}

	void setLightRef(Scene::Light* light);
	void lookAt(glm::vec3 P, glm::vec3 C);

	void setTransformation(glm::vec3 T, glm::vec3 RAxis, float angle, glm::vec3 S);

	void turnOn();
	void turnOff();
private:
	Scene::Light* m_light;
};

} /* namespace cg1 */

#endif /* FOLDER_CG1_CORE_FLASHLIGHT_H_ */
