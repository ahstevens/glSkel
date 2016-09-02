#pragma once
#include <glSkel/mesh.h>
#include <glSkel/shader.h>

#include <glSkel/Gabor.h>

class Slatissima
{
public:
	Slatissima(GLfloat length, GLfloat width, GLfloat thickness, Gabor &g);
	~Slatissima();

	void rotateX(float degrees);
	void rotateY(float degrees);
	void rotateZ(float degrees);
	void setOrientation(glm::quat orientation = glm::quat());
	glm::quat getOrientation();

	void Draw(Shader s);

private:
	Mesh* mesh;
	std::vector<GLuint> indices;

	GLfloat length, width, thickness;
	GLuint nVertsTall, nVertsWide;

	Gabor gabor;

	void buildStrip();

	std::vector<Texture> loadTextures();
};

