#pragma once
#include <glSkel/mesh.h>
#include <glSkel/shader.h>

struct GaussianKernel {
	glm::vec2 center;
	GLfloat amplitude;
	GLfloat a, b, c;
};

class Slatissima
{
public:
	Slatissima(GLfloat length, GLfloat width, GLfloat thickness, GLfloat spinePadding, GLfloat wavinessMulti);
	~Slatissima();

	void Draw(Shader s);

private:
	Mesh* mesh;
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;

	GLfloat length, width, thickness, spinePadding, wavinessMulti;
	GLuint nVertsTall, nVertsWide;

	void buildStrip();
	void calculateStripNormals(std::vector<Vertex> &v, GLuint nVertsWide, GLuint nVertsTall);
	std::vector<GLuint> getStripIndices(GLuint nVertsWide, GLuint nVertsTall);
	glm::vec3 getNormalFromIndices(std::vector<Vertex> &v, GLuint aInd1, GLuint aInd2, GLuint bInd1, GLuint bInd2);
	std::vector<Texture> loadTextures();
	GaussianKernel getGaussianKernel(glm::vec2 center, glm::vec2 spread, GLfloat angle, GLfloat amplitude);
	GLfloat gaussian(glm::vec2 pos, GaussianKernel k);
};

