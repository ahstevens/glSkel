#pragma once
#include <glSkel/mesh.h>
#include <glSkel/shader.h>

class Cube
{
public:
	Cube();
	~Cube();

	std::vector<glm::vec3> positions;
	std::vector<GLfloat> angles;

	void Draw(Shader s);

private:
	Mesh* mesh;
};

