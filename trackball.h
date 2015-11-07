#ifndef TRACKBALL_H
#define TRACKBALL_H

#include "Geometry.h"
#define FREEGLUT_STATIC
#include <GL/freeglut.h>

class TrackBall
{
public:
	TrackBall(int width, int height, float fovy);
	~TrackBall() {};

	void Resize(int width, int height, float fovy);

	void MouseMoveRotate(Vector2D v2d);
	void MouseMoveScale(Vector2D v2d);
	void MouseMoveTranslate(Vector2D v2d);

	void MousePress(Vector2D v2d, Vector3D v3d);

private:
	Vector3D mapToSphere(Vector2D v2d);
	void rotate(Vector3D axis, float angle);
	void scale(float scaleFactor);
	void translate(Vector3D shift);

	void rotateMatrix(Vector3D axis, float angle, float *m);
	void scaleMatrix(float scaleFactor, float *m);
	void multiply(float *left, float *right0, float *right1);

private:
	float width, height, fovy;
	Vector2D prevPoint2D;
	Vector3D prevPoint3D, zoomCenter;

public:
	float mvMatrix[16];
};

#endif