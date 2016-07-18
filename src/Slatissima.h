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
	GLuint nSpineVerts;

	void buildModel();
	void buildCenterBladeGeometry();
	void calcSpineNormals();
	void calcCenterBladeEdgeNormals();
	glm::vec3 getNormalFromIndices(std::vector<Vertex> &v, GLuint aInd1, GLuint aInd2, GLuint bInd1, GLuint bInd2);
	std::vector<Texture> loadTextures();
	
	void buildStrip(GLuint widthGranularity = 1);
};

