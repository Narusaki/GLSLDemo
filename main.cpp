///////////////////////////////////////////////////////////////////////
//
// triangles.cpp
//
///////////////////////////////////////////////////////////////////////

#define _USE_MATH_DEFINES
#include <iostream>
#include "vgl.h"
#include "LoadShaders.h"
#include "model.h"
#include <string>

#include <cmath>
#include "trackball.h"

using namespace std;

enum VAO_IDs { Model1, NumVAOs };
enum Buffer_IDs { Vertices1, Faces1, Normals1, FaceVerts1, FaceNormals1, NumBuffers };
enum Attrib_IDs { vPosition = 0, vNormal = 1 };
GLuint VAOs[NumVAOs];
GLuint Buffers[NumBuffers];
const GLuint NumVertices = 6;

int winHandle = -1;
GLint mvMatrixLoc, projMatrixLoc, scaleMatrixLoc;
GLint meshColorLoc;
GLint lightPosLoc;
GLfloat LightPos[] = { 0.0, 0.0, 3.0 };
GLfloat zTheta = 0.0;
GLfloat projMatrix[16], scaleMatrix[16];

bool dkey = false;
bool showWire = false;
GLfloat meshColor[3] = { 1.0, 1.0, 1.0 };
GLfloat wireColor[3] = { 0.0, 0.0, 0.0 };
const double DEPTHEPS = 0.001;

bool skey = false;
bool smooth = true;

// input model
Model model;
TrackBall *trackball;

int mouseButton, mouseState;

void constructPerspectiveMatrix(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar, GLfloat *m)
{
	GLfloat height = tan(fovy / 180.0 * M_PI / 2.0) * zNear * 2.0;
	GLfloat width = height * aspect;

	memset(m, 0, sizeof(GLfloat) * 16);
	m[0] = 2.0 * zNear / width;
	m[5] = 2.0 * zNear / height;
	m[10] = (zFar + zNear) / (zNear - zFar);
	m[11] = -1.0;
	m[14] = 2.0 * zFar * zNear / (zNear - zFar);
}

void SetForModel(GLuint modelIndex, Model &model)
{
	glBindVertexArray(VAOs[modelIndex]);

	glBindBuffer(GL_ARRAY_BUFFER, Buffers[modelIndex * 5 + 0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * model.vertices.size(),
		(GLfloat*)(&model.vertices[0]), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Buffers[modelIndex * 5 + 1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * model.faces.size(),
		(GLuint*)(&model.faces[0]), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[modelIndex * 5 + 2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * model.normals.size(),
		(GLfloat*)(&model.normals[0]), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[modelIndex * 5 + 3]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * model.faceVerts.size(),
		(GLfloat*)(&model.faceVerts[0]), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[modelIndex * 5 + 4]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * model.faceNormals.size(),
		(GLfloat*)(&model.faceNormals[0]), GL_STATIC_DRAW);

	// default is smooth rendering
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[modelIndex * 5 + 0]);
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(vPosition);

	glBindBuffer(GL_ARRAY_BUFFER, Buffers[modelIndex * 5 + 2]);
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(vNormal);
}

void init(void)
{
	ShaderInfo shaders[] = {
		{ GL_VERTEX_SHADER, "triangles.vert" },
		{ GL_FRAGMENT_SHADER, "triangles.frag" },
		{ GL_NONE, NULL }
	};
	GLuint program = LoadShaders(shaders);
	glUseProgram(program);


	// Settings for model1
	glGenVertexArrays(NumVAOs, VAOs);
	glGenBuffers(NumBuffers, Buffers);

	SetForModel(0, model);

	glClearColor(1.0, 1.0, 1.0, 1.0);

	trackball = new TrackBall(640, 480, 45.0);
	for (int i = 0; i < 16; ++i)
		projMatrix[i] = (i / 4 == i % 4 ? 1.0 : 0.0);

	scaleMatrix[0] = 1.0 / model.scale;
	scaleMatrix[5] = 1.0 / model.scale;
	scaleMatrix[10] = 1.0 / model.scale;
	scaleMatrix[12] = -model.originX / model.scale;
	scaleMatrix[13] = -model.originY / model.scale;
	scaleMatrix[14] = -model.originZ / model.scale;
	scaleMatrix[15] = 1.0;

	mvMatrixLoc = glGetUniformLocation(program, "mvMatrix");
	projMatrixLoc = glGetUniformLocation(program, "projMatrix");
	scaleMatrixLoc = glGetUniformLocation(program, "scaleMatrix");
	trackball->mvMatrix[14] -= 2.0;
	glUniformMatrix4fv(mvMatrixLoc, 1, GL_FALSE, trackball->mvMatrix);
	trackball->mvMatrix[14] += 2.0;
	glUniformMatrix4fv(projMatrixLoc, 1, GL_FALSE, projMatrix);
	glUniformMatrix4fv(scaleMatrixLoc, 1, GL_FALSE, scaleMatrix);

	// set light parameters

	GLfloat ambientLight[] = { 0.1, 0.1, 0.1 };
	GLfloat diffuseLight[] = { 0.6, 0.6, 0.6 };
	GLfloat specularLight[] = { 0.2, 0.2, 0.2 };
	GLfloat shininess = 1.0;
	GLfloat strength = 0.00001;

	meshColorLoc = glGetUniformLocation(program, "meshColor");
	GLint ambientLoc = glGetUniformLocation(program, "Ambient");
	GLint diffuseLoc = glGetUniformLocation(program, "Diffuse");
	GLint specularLoc = glGetUniformLocation(program, "Specular");
	lightPosLoc = glGetUniformLocation(program, "LightPosition");
	GLint shininessLoc = glGetUniformLocation(program, "Shininess");
	GLint strengthLoc = glGetUniformLocation(program, "Strength");
	GLint cAttenuation = glGetUniformLocation(program, "ConstantAttenuation");
	GLint bAttenuation = glGetUniformLocation(program, "LinearAttenuation");
	GLint aAttenuation = glGetUniformLocation(program, "QuadraticAttenuation");

	glUniform3fv(ambientLoc, 1, ambientLight);
	glUniform3fv(diffuseLoc, 1, diffuseLight);
	glUniform3fv(specularLoc, 1, specularLight);
	glUniform3fv(lightPosLoc, 1, LightPos);
	glUniform1f(shininessLoc, shininess);
	glUniform1f(strengthLoc, strength);

	glUniform1f(cAttenuation, 1.0); glUniform1f(bAttenuation, 0.0005); glUniform1f(aAttenuation, 0.0005);

	glEnable(GL_DEPTH_TEST);

}
//---------------------------------------------------------------------
//
// display
//
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindVertexArray(VAOs[Model1]);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glUniform3fv(meshColorLoc, 1, meshColor);
	glDepthRange(DEPTHEPS, 1.0);
	if (smooth)
		glDrawElements(GL_TRIANGLES, model.faces.size(), GL_UNSIGNED_INT, NULL);
	else
		glDrawArrays(GL_TRIANGLES, 0, model.faceVerts.size() / 3);

	if (showWire)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glUniform3fv(meshColorLoc, 1, wireColor);
		glDepthRange(0.0, 1.0 - DEPTHEPS);
		if (smooth)
			glDrawElements(GL_TRIANGLES, model.faces.size(), GL_UNSIGNED_INT, NULL);
		else
			glDrawArrays(GL_TRIANGLES, 0, model.faceVerts.size() / 3);
	}
	glutSwapBuffers();
	glutPostRedisplay();
}

void reshapeFunc(int width, int height)
{
	constructPerspectiveMatrix(45.0, (GLfloat)width / (GLfloat)height, 0.1, 1000.0, projMatrix);
	glUniformMatrix4fv(projMatrixLoc, 1, GL_FALSE, projMatrix);
	glViewport(0, 0, width, height);
	trackball->Resize(width, height, 45.0);
}

void keyboardFunc(unsigned char key, int x, int y)
{
	switch (key)
	{
	case VK_ESCAPE: 
		glutDestroyWindow(winHandle);
		exit(0);
		break;
	case 'd':
	case 'D':
		if (!dkey)
		{
			dkey = true;
			showWire = !showWire;
		}
		break;
	case 's':
	case 'S':
		if (!skey)
		{
			skey = true;
			smooth = !smooth;

			glBindVertexArray(VAOs[Model1]);
			if (smooth)
			{
				glBindBuffer(GL_ARRAY_BUFFER, Buffers[Model1 * 5 + 0]);
				glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
				glEnableVertexAttribArray(vPosition);

				glBindBuffer(GL_ARRAY_BUFFER, Buffers[Model1 * 5 + 2]);
				glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
				glEnableVertexAttribArray(vNormal);
			}
			else
			{
				glBindBuffer(GL_ARRAY_BUFFER, Buffers[Model1 * 5 + 3]);
				glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
				glEnableVertexAttribArray(vPosition);

				glBindBuffer(GL_ARRAY_BUFFER, Buffers[Model1 * 5 + 4]);
				glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
				glEnableVertexAttribArray(vNormal);
			}
		}
		break;
	default:
		break;
	}
	glutPostRedisplay();
}

void keyboardUpFunc(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'd':
	case 'D':
		if (dkey) dkey = false;
		break;

	case 's':
	case 'S':
		if (skey) skey = false;
		break;
	}
}

void mouseFunc(int button, int state, int x, int y)
{
	mouseButton = button;
	mouseState = state;
	if (mouseButton == GLUT_LEFT_BUTTON && mouseState == GLUT_DOWN)
		trackball->MousePress(Vector2D(x, y), Vector3D());
	else if ((mouseButton == GLUT_RIGHT_BUTTON || mouseButton == GLUT_MIDDLE_BUTTON) && mouseState == GLUT_DOWN)
	{
		int viewport[16];
		glGetIntegerv(GL_VIEWPORT, viewport);
		GLfloat z;
		glReadPixels(x, viewport[3] - y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &z);
		if (z == 1.0)
		{
			mouseButton = -1; return;
		}
		double mvMatrix[16], projMatrixd[16];
		memset(mvMatrix, 0, 16 * sizeof(double));
		mvMatrix[0] = 1.0; mvMatrix[5] = 1.0; mvMatrix[10] = 1.0; mvMatrix[15] = 1.0;
		for (int i = 0; i < 16; ++i) projMatrixd[i] = projMatrix[i];
		if (mouseButton == GLUT_RIGHT_BUTTON) mvMatrix[14] = -2.0;
		Vector3D obj;
		gluUnProject(x, viewport[3] - y, z, mvMatrix, projMatrixd, viewport, &obj.x, &obj.y, &obj.z);
		trackball->MousePress(Vector2D(x, y), obj);
	}
		
}

void motionFunc(int x, int y)
{
	if (mouseButton == GLUT_LEFT_BUTTON && mouseState == GLUT_DOWN)
		trackball->MouseMoveRotate(Vector2D(x, y));
	else if (mouseButton == GLUT_RIGHT_BUTTON && mouseState == GLUT_DOWN)
	{
		GLfloat scaleFactor = trackball->MouseMoveScale(Vector2D(x, y));
		LightPos[0] *= scaleFactor;
		LightPos[1] *= scaleFactor;
		LightPos[2] *= scaleFactor;
		glUniform3fv(lightPosLoc, 1, LightPos);
	}
	else if (mouseButton == GLUT_MIDDLE_BUTTON && mouseState == GLUT_DOWN)
		trackball->MouseMoveTranslate(Vector2D(x, y));

	trackball->mvMatrix[14] -= 2.0;
	glUniformMatrix4fv(mvMatrixLoc, 1, GL_FALSE, trackball->mvMatrix);
	trackball->mvMatrix[14] += 2.0;

	glutPostRedisplay();
}

//---------------------------------------------------------------------
//
// main
//
int main(int argc, char** argv)
{
	if (argc < 2)
	{
		cout << "USAGE: [.exe] [inputModel1]" << endl;
		return -1;
	}

	model.LoadModel(argv[1]);

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(640, 480);
	glutInitContextVersion(4, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	winHandle = glutCreateWindow("GLSL example by Narusaki");
	glewExperimental = GL_TRUE;
	if (glewInit()) {
		cerr << "Unable to initialize GLEW ... exiting" << endl;
		exit(EXIT_FAILURE);
	}
	init();
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboardFunc);
	glutKeyboardUpFunc(keyboardUpFunc);
	glutReshapeFunc(reshapeFunc);
	glutMouseFunc(mouseFunc);
	glutMotionFunc(motionFunc);
	
	glutMainLoop();
}