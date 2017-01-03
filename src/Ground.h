#pragma once
#include <glSkel/Object.h>
#include <glSkel/BroadcastSystem.h>
#include <glSkel/Mesh.h>
#include <glSkel/Shader.h>

#include <glSkel/Gabor.h>

#include <bullet/btBulletDynamicsCommon.h>

class Ground : public Object
{
public:
	Ground(float width, float length, float depth, btDynamicsWorld* dynamicsWorld);
	~Ground();

	btRigidBody* getRigidBody();

	void Draw(Shader s);

private:
	float m_fLength, m_fWidth, m_fDepth;
	
	Mesh* m_pMesh;
	
	void buildModel();
	void initPhysics();
	
	std::vector<Texture> loadTextures();

	bool m_bPhysicsInit;
	btDynamicsWorld* m_pDynamicsWorld;
	btRigidBody* m_pRigidBody;
};

