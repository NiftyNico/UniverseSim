//
// sueda
// October, 2014
//

#include "Camera.h"
#include "MatrixStack.h"
#include <iostream>

Camera::Camera(float windowSize) :
	aspect(1.0f),
	fovy(45.0f / 180.0f * M_PI),
	znear(0.1f),
	zfar(1000.0f),
	rotations(0.0, 0.0),
	translations(0.0f, 0.0f, -2.0f),
	rfactor(0.01f),
	tfactor(0.005f),
	sfactor(0.005f)
{
	Camera::windowSize = windowSize;
}

Camera::~Camera()
{
	
}

void Camera::mouseClicked(int x, int y, bool shift, bool ctrl, bool alt)
{
	mousePrev.x = x;
	mousePrev.y = y;
	if(shift) {
		state = Camera::TRANSLATE;
	} else if(ctrl) {
		state = Camera::SCALE;
	} else {
		state = Camera::ROTATE;
	}
}

void Camera::mouseMoved(int x, int y)
{
	glm::vec2 mouseCurr(x, y);
	glm::vec2 dv = mouseCurr - mousePrev;
	switch(state) {
		case Camera::ROTATE:
			rotations += rfactor * dv;
			break;
		case Camera::TRANSLATE:
			translations.x += tfactor * dv.x;
			translations.y -= tfactor * dv.y;
			break;
		case Camera::SCALE:
			translations.z *= (1.0f - sfactor * dv.y);
			break;
	}
	mousePrev.x = x;
	mousePrev.y = y;
}

void Camera::movement(char c)
{
	switch(c)
	{
		case 'w':
    		translations.z -= MOVE_SPEED;
			break;
		case 'a':
			translations.x -= MOVE_SPEED;
			break;
		case 's':
    		translations.z += MOVE_SPEED;
			break;
		case 'd':
			translations.x += MOVE_SPEED;
			break;
		case 'q':
			translations.y -= MOVE_SPEED;
			break;
		case 'e':
		    translations.y += MOVE_SPEED;
		    break;
	}
}

void Camera::passiveMouseMoved(int x, int y)
{
	glm::vec2 mouseCurr(x, y);
	glm::vec2 dv = mouseCurr - mousePrev;

	rotations += rfactor * dv;

	mousePrev.x = x;
	mousePrev.y = y;
}

void Camera::applyProjectionMatrix(MatrixStack *P) const
{
	P->perspective(fovy, aspect, znear, zfar);
}

void Camera::applyViewMatrix(MatrixStack *MV) const
{
	glm::mat4 viewMat(1.0f);

	viewMat *= glm::translate(translations);
	viewMat *= glm::rotate(-rotations.x, glm::vec3(0.0f, 1.0f, 0.0f));
	viewMat *= glm::rotate(-rotations.y, glm::vec3(1.0f, 0.0f, 0.0f));

	MV->multMatrix(glm::inverse(viewMat));
}

void Camera::calcNormal() {
	normal = glm::vec3(glm::rotate(-rotations.x, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(-rotations.y, glm::vec3(1.0f, 0.0f, 0.0f)) * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
}

bool Camera::inView(const glm::vec3 &pos, float radius) const {
	glm::vec3 dist = pos - translations;
	return (glm::dot(dist, normal) <= 0 && glm::dot(dist, dist) <= 500 * 500) || abs(glm::dot(dist, glm::normalize(normal))) <= radius;
}
