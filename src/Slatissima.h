#pragma once
#include <glSkel/mesh.h>
#include <glSkel/shader.h>

#include <glSkel/Gabor.h>

#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/BulletSoftBody/btSoftBody.h>
#include <bullet/BulletSoftBody/btSoftRigidDynamicsWorld.h>

class Slatissima
{
public:
	Slatissima(GLfloat length, GLfloat width, GLfloat edgeWaveAmplitude, float solidThickness, btSoftRigidDynamicsWorld* dynamicsWorld, btSoftBodyWorldInfo &btInfo);
	~Slatissima();

	void rotateX(float degrees);
	void rotateY(float degrees);
	void rotateZ(float degrees);
	void setOrientation(glm::quat orientation = glm::quat());
	glm::quat getOrientation();
	void setPosition(glm::vec3 pos);
	glm::vec3 getPosition();

	void bump(btVector3 dir);
	
	void anchorToBody(btRigidBody* body);
	
	void update();

	void Draw(Shader s);

private:
	Mesh* mesh;
	std::vector<GLuint> indices;

	GLfloat length, width, edgeWaveAmplitude;
	GLuint nVertsTall;

	std::vector<Gabor*> gabors;

	void initPhysics(btSoftBodyWorldInfo &sbInfo);
	void buildModel();

	void generateGabors(float x);
	float calculateEnvelope(float currentRatio, float begin, float max1, float max2, float end);
	float getRandRatio();
	
	std::vector<Texture> loadTextures();

	btSoftRigidDynamicsWorld* m_pDynamicsWorld;
	btSoftBody* m_pSoftBody;
};

