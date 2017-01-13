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
	Slatissima(float solidThickness, glm::vec3 position, glm::mat3 orientation, btSoftRigidDynamicsWorld* dynamicsWorld);
	Slatissima(float solidThickness, float length, float width, float edgeWaveAmplitude, glm::vec3 position, glm::mat3 orientation, btSoftRigidDynamicsWorld* dynamicsWorld);
	~Slatissima();

	void setDebugDrawFlags(int flags);
	int getDebugDrawFlags();
	void toggleDebugDrawFlag(int flag);

	float getLength();
	float getWidth();

	void bump(btVector3 dir);
	
	void pinToBody(float lengthRatio
		, glm::vec3 kernel
		, btRigidBody* body
		, float influence = 1.f
		, bool disableCollisionsWithBody = false
		, bool convergeAnchors = false
		, bool anchorInPlace = true
		); 
	void pinToBody(float lengthRatio
		, float kernelX
		, float kernelY
		, float kernelZ
		, btRigidBody* body
		, float influence = 1.f
		, bool disableCollisionsWithBody = false
		, bool convergeAnchors = false
		, bool anchorInPlace = true
		);
	void pinToBody(glm::vec3 worldPos
		, glm::vec3 kernelSize
		, btRigidBody* body
		, float influence = 1.f
		, bool disableCollisionsWithBody = false
		, bool convergeAnchors = false
		, bool anchorInPlace = true
		);
	void pinToBody(float worldX, float worldY, float worldZ
		, float kernelX
		, float kernelY
		, float kernelZ
		, btRigidBody* body
		, float influence = 1.f
		, bool disableCollisionsWithBody = false
		, bool convergeAnchors = false
		, bool anchorInPlace = true
		);
	
	void update();

	void receiveEvent(Object* obj, const int event, void* data);

	void Draw(Shader s);

private:
	Mesh* mesh;
	std::vector<GLuint> indices;

	GLfloat m_fLength, m_fWidth, m_fEdgeWaveAmplitude;
	GLuint nVertsTall;

	std::vector<Gabor*> gabors;

	void buildModel();
	void initPhysics();

	void generateGabors(float x);
	float calculateEnvelope(float currentRatio, float begin, float max1, float max2, float end);
	float getRandRatio();
	
	std::vector<Texture> loadTextures();

	void debugDraw();

	bool saveAsObj(std::string name = "untitled_model");

	bool m_bPhysicsInit, m_bSolidMesh;
	btSoftRigidDynamicsWorld* m_pDynamicsWorld;
	btSoftBody* m_pSoftBody;

	int m_debugDrawFlags;
};

