#version 430 core
layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;

uniform mat4 scaleMatrix;
uniform mat4 mvMatrix, projMatrix;
uniform mat3 normalTransferMatrix;

out vec3 position;
out vec3 normal;

void main()
{
	gl_Position = projMatrix * mvMatrix * scaleMatrix * vec4(vPosition, 1.0);
	position = vec3(mvMatrix * scaleMatrix * vec4(vPosition, 1.0));
	normal = normalize(normalTransferMatrix * vNormal);
}