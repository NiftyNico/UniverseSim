//
// sueda
// October, 2014
//

#pragma  once
#ifndef __Camera__
#define __Camera__

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include <stdio.h>

#define ROTATE_BY 0.02f
#define SLOW_BY 0.05f
#define PI 3.141592654f
#define LOOK_SPEED 0.05f
#define MOVE_SPEED 0.05f

class MatrixStack;

class Camera
{
public:
	
	enum {
		ROTATE = 0,
		TRANSLATE,
		SCALE
	};
	
	Camera(float windowSize);
	virtual ~Camera();
	void setAspect(float a) { aspect = a; };
	void setRotationFactor(float f) { rfactor = f; };
	void setTranslationFactor(float f) { tfactor = f; };
	void setScaleFactor(float f) { sfactor = f; };
	void mouseClicked(int x, int y, bool shift, bool ctrl, bool alt);
	void mouseMoved(int x, int y);
	void applyProjectionMatrix(MatrixStack *P) const;
	void applyViewMatrix(MatrixStack *MV) const;
	void movement(char c);
	void passiveMouseMoved(int x, int y);
private:
	float aspect;
	float fovy;
	float znear;
	float zfar;
	glm::vec2 rotations;
	glm::vec3 translations;
	float scale;
	glm::vec2 mousePrev;
	int state;
	float rfactor;
	float tfactor;
	float sfactor;

	float windowSize;
	float xPos, yPos, zPos;
	float lastX, lastY;
	float xRot, yRot;
};

#endif
