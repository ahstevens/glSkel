#pragma once
#include <glSkel/Object.h>
#include <glSkel/BroadcastSystem.h>
#include <glSkel/Mesh.h>
#include <glSkel/Shader.h>

#include <glSkel/Gabor.h>

#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/BulletSoftBody/btSoftBody.h>
#include <bullet/BulletSoftBody/btSoftRigidDynamicsWorld.h>
#include <bullet/BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h>

class Slatissima : public Object, public BroadcastSystem::Listener
{
public:
	Slatissima(float solidThickness, glm::vec3 position, glm::quat orientation, btSoftRigidDynamicsWorld* dynamicsWorld);
	~Slatissima();

	void rotateX(float degrees);
	void rotateY(float degrees);
	void rotateZ(float degrees);

	void setDebugDrawFlags(int flags);
	int getDebugDrawFlags();
	void toggleDebugDrawFlag(int flag);

	void bump(btVector3 dir);
	
	void anchorToBody(btRigidBody* body);
	
	void update();

	void receiveEvent(Object* obj, const int event, void* data);

	void Draw(Shader s);

private:
	Mesh* mesh;
	std::vector<GLuint> indices;

	GLfloat length, width, edgeWaveAmplitude;
	GLuint nVertsTall;

	std::vector<Gabor*> gabors;

	void buildModel();
	void initPhysics();

	void generateGabors(float x);
	float calculateEnvelope(float currentRatio, float begin, float max1, float max2, float end);
	float getRandRatio();
	
	std::vector<Texture> loadTextures();

	void debugDraw();

	bool m_bPhysicsInit, m_bSolidMesh;;
	btSoftRigidDynamicsWorld* m_pDynamicsWorld;
	btSoftBody* m_pSoftBody;

	int m_debugDrawFlags;
};

