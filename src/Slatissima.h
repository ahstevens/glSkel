#pragma once
#include <glSkel/mesh.h>
#include <glSkel/shader.h>

#include <glSkel/Gabor.h>

#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/btBulletCollisionCommon.h>

class Slatissima
{
public:
	Slatissima(GLfloat length, GLfloat width, GLfloat thickness, std::vector<Gabor*> g, btDiscreteDynamicsWorld* dynamicsWorld);
	~Slatissima();

	void rotateX(float degrees);
	void rotateY(float degrees);
	void rotateZ(float degrees);
	void setOrientation(glm::quat orientation = glm::quat());
	glm::quat getOrientation();
	void setPosition(glm::vec3 pos);
	glm::vec3 getPosition();

	void drop(btVector3 pos);
	void bump(btVector3 dir);

	void Draw(Shader s);

private:
	Mesh* mesh;
	std::vector<GLuint> indices;

	GLfloat length, width, thickness;
	GLuint nVertsTall, nVertsWide;

	std::vector<Gabor*> gabors;

	void buildStrip();

	std::vector<Texture> loadTextures();

	btDiscreteDynamicsWorld* m_pDynamicsWorld;
	btRigidBody* m_pRigidBody;
};

