#include "model.h"
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
using namespace std;

void Model::LoadModel(const char *fileName)
{
	string fileNameStr = fileName;
	string suffix = fileNameStr.substr(fileNameStr.rfind("."), fileNameStr.length() - fileNameStr.rfind("."));
	transform(suffix.begin(), suffix.end(), suffix.begin(), ::tolower);
	
	if (suffix == ".obj") LoadOBJ(fileName);
}

void Model::LoadOBJ(const char *fileName)
{
	ifstream input(fileName);
	string curLine;
	char lineType;
	GLfloat x, y, z;
	GLuint v0, v1, v2;
	GLuint vt;
	scale = 1.0;
	originX = 0.0, originY = 0.0, originZ = 0.0;

	double maxX = -1e30, maxY = -1e30, maxZ = -1e30;
	double minX = 1e30, minY = 1e30, minZ = 1e30;

	while (getline(input, curLine))
	{
		stringstream sin;
		sin << curLine;
		if (curLine.substr(0, 2) == "v ")
		{
			sin >> lineType >> x >> y >> z;
			vertices.push_back(x); vertices.push_back(y); vertices.push_back(z);
			maxX = __max(maxX, x); minX = __min(minX, x);
			maxY = __max(maxY, y); minY = __min(minY, y);
			maxZ = __max(maxZ, z); minZ = __min(minZ, z);
		}
		else if (curLine.substr(0, 2) == "vt")
		{
			curLine = curLine.substr(3, curLine.length() - 3);
			sin >> v0 >> v1;
			texcoords.push_back(v0); texcoords.push_back(v1); 
		}
		else if (curLine.substr(0, 2) == "f ")
		{
			if (curLine.find("/") != string::npos)
				sin >> lineType >> 
				v0 >> lineType >> vt >> 
				v1 >> lineType >> vt >> 
				v2 >> lineType >> vt;
			else
				sin >> lineType >> v0 >> v1 >> v2;
			faces.push_back(v0-1); faces.push_back(v1-1); faces.push_back(v2-1);
		}
	}

	originX = (maxX + minX) / 2;
	originY = (maxY + minY) / 2;
	originZ = (maxZ + minZ) / 2;
	scale = maxX - minX;
	scale = __max(scale, maxY - minY);
	scale = __max(scale, maxZ - minZ);
	scale /= 2.0;

	calcNormals();
}

void Model::calcNormals()
{
	vector<int> degrees(vertices.size() / 3, 0);
	normals.resize(vertices.size(), 0.0);
	for (unsigned i = 0; i < faces.size(); i+=3)
	{
		GLuint v0 = faces[i], v1 = faces[i + 1], v2 = faces[i + 2];
		++degrees[v0]; ++degrees[v1]; ++degrees[v2];
		GLfloat x0 = vertices[3 * v0], y0 = vertices[3 * v0 + 1], z0 = vertices[3 * v0 + 2];
		GLfloat x1 = vertices[3 * v1], y1 = vertices[3 * v1 + 1], z1 = vertices[3 * v1 + 2];
		GLfloat x2 = vertices[3 * v2], y2 = vertices[3 * v2 + 1], z2 = vertices[3 * v2 + 2];

		GLfloat vec0x = x1 - x0, vec0y = y1 - y0, vec0z = z1 - z0;
		GLfloat vec1x = x2 - x0, vec1y = y2 - y0, vec1z = z2 - z0;

		GLfloat nx = vec0y*vec1z - vec1y*vec0z, ny = vec0z*vec1x - vec1z*vec0x, nz = vec0x*vec1y - vec1x*vec0y;
		GLfloat len = sqrt(nx*nx + ny*ny + nz*nz);
		nx /= len; ny /= len; nz /= len;

		normals[3 * v0] += nx; normals[3 * v0 + 1] += ny; normals[3 * v0 + 2] += nz;
		normals[3 * v1] += nx; normals[3 * v1 + 1] += ny; normals[3 * v1 + 2] += nz;
		normals[3 * v2] += nx; normals[3 * v2 + 1] += ny; normals[3 * v2 + 2] += nz;
	}
	for (unsigned i = 0; i < vertices.size(); i += 3)
	{
		normals[i] /= degrees[i / 3]; 
		normals[i+1] /= degrees[i / 3];
		normals[i+2] /= degrees[i / 3];
	}
}