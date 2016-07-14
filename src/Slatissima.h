#pragma once
#include <glSkel/mesh.h>
#include <glSkel/shader.h>

class Slatissima
{
public:
	Slatissima(GLfloat length, GLfloat width, GLfloat thickness, GLfloat spinePadding, GLfloat wavinessMulti, GLuint nSegments);
	~Slatissima();

	void Draw(Shader s);

private:
	Mesh* mesh;
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;

	GLfloat length, width, thickness, spinePadding, wavinessMulti;
	GLuint nSegments;

	void buildModel();
	void calcSpineNormals();
	void calcEdgeNormals();
	glm::vec3 getNormalFromIndices(GLuint aInd1, GLuint aInd2, GLuint bInd1, GLuint bInd2);
	std::vector<Texture> loadTextures();
};

