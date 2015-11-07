#include "trackball.h"
#include <cmath>
#include <iostream>

TrackBall::TrackBall(int width, int height, float fovy)
{
	this->width = width / 2.0;
	this->height = height / 2.0;
	this->fovy = fovy;
	memset(mvMatrix, 0, 16 * sizeof(float));
	mvMatrix[0] = 1.0; mvMatrix[5] = 1.0;
	mvMatrix[10] = 1.0; mvMatrix[15] = 1.0;
}

void TrackBall::Resize(int width, int height, float fovy)
{
	this->width = width / 2.0;
	this->height = height / 2.0;
	this->fovy = fovy;
}

void TrackBall::MouseMoveRotate(Vector2D v2d)
{
	Vector2D currentPoint2D = v2d;
	Vector3D currentPoint3D = mapToSphere(v2d);
	Vector3D axis = prevPoint3D ^ currentPoint3D;
	if (axis.length() < 1e-7) return;
	axis.normalize();
	float angle = acos(prevPoint3D * currentPoint3D);
	rotate(axis, 2.0 * angle);
	prevPoint2D = currentPoint2D;
	prevPoint3D = currentPoint3D;
}

void TrackBall::MouseMoveScale(Vector2D v2d)
{
	Vector2D currentPoint2D = v2d;
	float scaleFactor = exp(-(currentPoint2D - prevPoint2D).y / 100.0);
	scale(scaleFactor);
	prevPoint2D = currentPoint2D;
}

void TrackBall::MouseMoveTranslate(Vector2D v2d)
{
	Vector2D currentPoint2D = v2d;
	Vector2D shift2D = zoomCenter.z * (currentPoint2D - prevPoint2D) * tan(fovy / 180.0 * PI / 2.0) / height;
	shift2D.y = -shift2D.y;
	std::cout << zoomCenter << std::endl;
	translate(Vector3D(-shift2D.x, -shift2D.y, 0.0));
	prevPoint2D = currentPoint2D;
}

void TrackBall::MousePress(Vector2D v2d, Vector3D v3d)
{
	prevPoint2D = v2d;
	prevPoint3D = mapToSphere(v2d);
	zoomCenter = v3d;
}

Vector3D TrackBall::mapToSphere(Vector2D v2d)
{
	Vector3D v3d;
	v3d.x = (v2d.x - width) / width;
	v3d.y = (height - v2d.y) / height;
	float v3dLen = v3d.length();
	if (v3dLen < 1.0)
		v3d.z = sqrt(1.0 - v3dLen*v3dLen);
	else
		v3d /= v3dLen;
	return v3d;
}

void TrackBall::rotate(Vector3D axis, float angle)
{
	Vector3D shift;
	for (int i = 0; i < 3; ++i)
	{
		shift[i] = mvMatrix[12 + i];
		mvMatrix[12 + i] = 0.0;
	}
	float rm[16];
	rotateMatrix(axis, angle, rm);
	multiply(mvMatrix, rm, mvMatrix);
	for (int i = 0; i < 3; ++i)
		mvMatrix[12 + i] = shift[i];
}

void TrackBall::scale(float scaleFactor)
{
	for (int i = 0; i < 3; ++i)
		mvMatrix[12 + i] -= zoomCenter[i];
	float sm[16];
	scaleMatrix(scaleFactor, sm);
	multiply(mvMatrix, sm, mvMatrix);
	for (int i = 0; i < 3; ++i)
		mvMatrix[12 + i] += zoomCenter[i];
}

void TrackBall::translate(Vector3D shift)
{
	for (int i = 0; i < 3; ++i)
		mvMatrix[12 + i] += shift[i];
}

void TrackBall::rotateMatrix(Vector3D axis, float angle, float *m)
{
	float c = cos(angle), s = sin(angle);
	memset(m, 0, 16 * sizeof(float));
	m[0] = c + (1 - c)*axis.x*axis.x;
	m[1] = (1 - c)*axis.y*axis.x + s*axis.z;
	m[2] = (1 - c)*axis.z*axis.x - s*axis.y;
	m[4] = (1 - c)*axis.x*axis.y - s*axis.z;
	m[5] = c + (1 - c)*axis.y*axis.y;
	m[6] = (1 - c)*axis.z*axis.y + s*axis.x;
	m[8] = (1 - c)*axis.x*axis.z + s*axis.y;
	m[9] = (1 - c)*axis.y*axis.z - s*axis.x;
	m[10] = c + (1 - c)*axis.z*axis.z;
	m[15] = 1.0;
}

void TrackBall::scaleMatrix(float scaleFactor, float *m)
{
	memset(m, 0, 16 * sizeof(float));
	m[0] = scaleFactor; m[5] = scaleFactor; m[10] = scaleFactor;
	m[15] = 1.0;
}

void TrackBall::multiply(float *left, float *right0, float *right1)
{
	float res[16];
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			res[j * 4 + i] = 0.0;
			for (int k = 0; k < 4; ++k)
				res[j * 4 + i] += right0[k * 4 + i] * right1[j * 4 + k];
		}
	}
	for (int i = 0; i < 16; ++i) left[i] = res[i];
}