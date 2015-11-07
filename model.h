#ifndef MODEL_H
#define MODEL_H

#include <gl\glew.h>
#include <vector>

class Model
{
public:
	Model() {}
	~Model() {};

	void LoadModel(const char *fileName);

private:
	void LoadOBJ(const char *fileName);

	void calcNormals();

public:
	std::vector<GLfloat> vertices;
	std::vector<GLuint> faces;
	std::vector<GLuint> texcoords;
	std::vector<GLfloat> normals;

	std::vector<GLfloat> faceVerts;
	std::vector<GLfloat> faceNormals;

	double scale;
	double originX, originY, originZ;
};
#endif