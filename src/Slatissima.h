#pragma once
#include <glSkel/mesh.h>
#include <glSkel/shader.h>

class Slatissima
{
public:
	Slatissima(GLfloat length, GLfloat width, GLfloat thickness);
	~Slatissima();

	std::vector<glm::vec3> positions;
	std::vector<GLfloat> angles;

	void Draw(Shader s);

private:
	Mesh* mesh;

	void buildModel(GLfloat length, GLfloat width, GLfloat thickness);
	std::vector<Texture> loadTextures();
};

