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

using namespace std;

enum VAO_IDs { Model1, Model2, NumVAOs };
enum Buffer_IDs { Vertices1, Faces1, Normals1, Vertices2, Faces2, Normals2, NumBuffers };
enum Attrib_IDs { vPosition = 0, vNormal = 1 };
GLuint VAOs[NumVAOs];
GLuint Buffers[NumBuffers];
const GLuint NumVertices = 6;

int winHandle = -1;
GLint mvMatrixLoc, projMatrixLoc, scaleMatrixLoc, normalTransferMatrixLoc;
GLfloat zTheta = 0.0;
GLfloat mvMatrix[16], projMatrix[16], scaleMatrix[16];
GLfloat normalTransferMatrix[9];

// input model
Model model1, model2;

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

	glBindBuffer(GL_ARRAY_BUFFER, Buffers[modelIndex * 3 + 0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * model.vertices.size(),
		(GLfloat*)(&model.vertices[0]), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Buffers[modelIndex * 3 + 1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * model.faces.size(),
		(GLuint*)(&model.faces[0]), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[modelIndex * 3 + 2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * model.normals.size(),
		(GLfloat*)(&model.normals[0]), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, Buffers[modelIndex*3+0]);
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(vPosition);

	glBindBuffer(GL_ARRAY_BUFFER, Buffers[modelIndex * 3 + 2]);
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

	SetForModel(0, model1);
	if (!model2.vertices.empty()) SetForModel(1, model2);
	glBindVertexArray(VAOs[Model1]);

	glClearColor(1.0, 1.0, 1.0, 1.0);

	for (int i = 0; i < 16; ++i)
	{
		mvMatrix[i] = (i / 4 == i % 4 ? 1.0 : 0.0);
		projMatrix[i] = (i / 4 == i % 4 ? 1.0 : 0.0);
		if (i >= 9) continue;
		normalTransferMatrix[i] = (i / 3 == i % 3 ? 1.0 : 0.0);
	}

	scaleMatrix[0] = 1.0 / model1.scale;
	scaleMatrix[5] = 1.0 / model1.scale;
	scaleMatrix[10] = 1.0 / model1.scale;
	scaleMatrix[12] = -model1.originX / model1.scale;
	scaleMatrix[13] = -model1.originY / model1.scale;
	scaleMatrix[14] = -model1.originZ / model1.scale - 4.0;
	scaleMatrix[15] = 1.0;

	mvMatrixLoc = glGetUniformLocation(program, "mvMatrix");
	projMatrixLoc = glGetUniformLocation(program, "projMatrix");
	scaleMatrixLoc = glGetUniformLocation(program, "scaleMatrix");
	normalTransferMatrixLoc = glGetUniformLocation(program, "normalTransferMatrix");
	glUniformMatrix4fv(mvMatrixLoc, 1, GL_FALSE, mvMatrix);
	glUniformMatrix4fv(projMatrixLoc, 1, GL_FALSE, projMatrix);
	glUniformMatrix4fv(scaleMatrixLoc, 1, GL_FALSE, scaleMatrix);
	glUniformMatrix4fv(normalTransferMatrixLoc, 1, GL_FALSE, normalTransferMatrix);

	// set light parameters

	GLfloat ambientLight[] = { 0.1, 0.1, 0.1 };
	GLfloat diffuseLight[] = { 0.7, 0.7, 0.7 };
	GLfloat specularLight[] = { 0.3, 0.3, 0.3 };
	GLfloat LightPos[] = { 3.0, 3.0, 3.0 };
	GLfloat shininess = 16.0;
	GLfloat strength = 1.0;

	GLint ambientLoc = glGetUniformLocation(program, "Ambient");
	GLint diffuseLoc = glGetUniformLocation(program, "Diffuse");
	GLint specularLoc = glGetUniformLocation(program, "Specular");
	GLint lightPosLoc = glGetUniformLocation(program, "LightPosition");
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

	zTheta += 0.01;
	mvMatrix[0] = cos(zTheta); mvMatrix[1] = sin(zTheta);
	mvMatrix[4] = -sin(zTheta); mvMatrix[5] = cos(zTheta);

	normalTransferMatrix[0] = cos(zTheta); normalTransferMatrix[1] = sin(zTheta);
	normalTransferMatrix[3] = -sin(zTheta); normalTransferMatrix[4] = cos(zTheta);

	glUniformMatrix4fv(mvMatrixLoc, 1, GL_FALSE, mvMatrix);
	glUniformMatrix3fv(normalTransferMatrixLoc, 1, GL_FALSE, normalTransferMatrix);

	/*glBindVertexArray(VAOs[Model1]);*/
	/*glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);*/
	glDrawElements(GL_TRIANGLES, model1.faces.size(), GL_UNSIGNED_INT, NULL);
	glutSwapBuffers();
	glutPostRedisplay();
}

void reshapeFunc(int width, int height)
{
	constructPerspectiveMatrix(45.0, (GLfloat)width / (GLfloat)height, 0.1, 1000.0, projMatrix);
	glUniformMatrix4fv(projMatrixLoc, 1, GL_FALSE, projMatrix);
	glViewport(0, 0, width, height);
}

void keyboardFunc(unsigned char key, int x, int y)
{
	switch (key)
	{
	case VK_ESCAPE: 
		glutDestroyWindow(winHandle);
		exit(0);
		break;
	case '1':
		glBindVertexArray(VAOs[Model1]);
// 		glBindBuffer(GL_ARRAY_BUFFER, Buffers[Vertices1]);
// 		glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
// 		glEnableVertexAttribArray(vPosition);
// 
// 		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Buffers[Faces1]);
// 
// 		glBindBuffer(GL_ARRAY_BUFFER, Buffers[Normals1]);
// 		glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
// 		glEnableVertexAttribArray(vNormal);

		scaleMatrix[0] = 1.0 / model1.scale;
		scaleMatrix[5] = 1.0 / model1.scale;
		scaleMatrix[10] = 1.0 / model1.scale;
		scaleMatrix[12] = -model1.originX / model1.scale;
		scaleMatrix[13] = -model1.originY / model1.scale;
		scaleMatrix[14] = -model1.originZ / model1.scale - 4.0;
		scaleMatrix[15] = 1.0;
		glUniformMatrix4fv(scaleMatrixLoc, 1, GL_FALSE, scaleMatrix);
		break;
	case '2':
		if (model2.vertices.empty()) return;
		glBindVertexArray(VAOs[Model2]);
// 		glBindBuffer(GL_ARRAY_BUFFER, Buffers[Vertices2]);
// 		glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
// 		glEnableVertexAttribArray(vPosition);
// 
// 		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Buffers[Faces2]);
// 
// 		glBindBuffer(GL_ARRAY_BUFFER, Buffers[Normals2]);
// 		glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
// 		glEnableVertexAttribArray(vNormal);

		scaleMatrix[0] = 1.0 / model2.scale;
		scaleMatrix[5] = 1.0 / model2.scale;
		scaleMatrix[10] = 1.0 / model2.scale;
		scaleMatrix[12] = -model2.originX / model2.scale;
		scaleMatrix[13] = -model2.originY / model2.scale;
		scaleMatrix[14] = -model2.originZ / model2.scale - 4.0;
		scaleMatrix[15] = 1.0;
		glUniformMatrix4fv(scaleMatrixLoc, 1, GL_FALSE, scaleMatrix);
		break;
	default:
		break;
	}
	glutPostRedisplay();
}

void specialFunc(int key, int x, int y)
{
	int mod = glutGetModifiers();
	if (mod != GLUT_ACTIVE_SHIFT && mod != GLUT_ACTIVE_ALT && mod != GLUT_ACTIVE_CTRL)
	{
		glutAttachMenu(GLUT_RIGHT_BUTTON);
	}
	else
	{
		glutDetachMenu(GLUT_RIGHT_BUTTON);
	}
}

void specialUpFunc(int key, int x, int y)
{
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void mouseFunc(int button, int state, int x, int y)
{
	
}

void menu(int item)
{
	keyboardFunc(std::to_string(item).c_str()[0], 0, 0);
	/*glutPostRedisplay();*/
}

//---------------------------------------------------------------------
//
// main
//
int main(int argc, char** argv)
{
	if (argc < 2)
	{
		cout << "USAGE: [.exe] [inputModel1] [optional: inputModel2]" << endl;
		return -1;
	}

	model1.LoadModel(argv[1]);
	if (argc >= 3) model2.LoadModel(argv[2]);

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
	glutSpecialFunc(specialFunc);
	glutSpecialUpFunc(specialUpFunc);
	glutReshapeFunc(reshapeFunc);

	glutCreateMenu(menu);
	for (int i = 1; i < argc; ++i)
		glutAddMenuEntry(argv[i], i);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	
	glutMainLoop();
}