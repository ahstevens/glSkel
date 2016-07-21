#pragma once
#include <glSkel/mesh.h>
#include <glSkel/shader.h>
#include <complex>

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
	GaussianKernel getGaussianKernel(glm::vec2 center, glm::vec2 spread, GLfloat angle, GLfloat amplitude);
	GLfloat gaussian(glm::vec2 pos, GaussianKernel k);
	std::complex<GLfloat> complexSinusoid(glm::vec2 pos, glm::vec2 spatialCentralFreq, GLfloat theta = 1.f);

	std::vector<Texture> loadTextures();
};

