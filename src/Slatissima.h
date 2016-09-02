#pragma once
#include <glSkel/mesh.h>
#include <glSkel/shader.h>

#include <glSkel/Gabor.h>

class Slatissima
{
public:
	Slatissima(GLfloat length, GLfloat width, GLfloat thickness, GLfloat spinePadding, GLfloat wavinessMulti);
	~Slatissima();

	void rotateX(float degrees);
	void rotateY(float degrees);
	void rotateZ(float degrees);
	void resetOrientation();

	void Draw(Shader s);

private:
	Mesh* mesh;
	std::vector<GLuint> indices;

	GLfloat length, width, thickness, spinePadding, wavinessMulti;
	GLuint nVertsTall, nVertsWide;

	void buildStrip();

	std::vector<Texture> loadTextures();
};

