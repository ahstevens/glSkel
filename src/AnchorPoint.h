#pragma once
#include <glSkel/Object.h>
#include <glSkel/Mesh.h>
#include <glSkel/Shader.h>

#include <bullet/btBulletDynamicsCommon.h>

class AnchorPoint : public Object
{
public:
	AnchorPoint(glm::vec3 position, btDynamicsWorld* dynamicsWorld);
	~AnchorPoint();

	btRigidBody* getRigidBody();

	void Draw(Shader s);

private:	
	Mesh* m_pMesh;
	
	void buildModel();
	void initPhysics();
	
	std::vector<Texture> loadTextures();

	bool m_bPhysicsInit;
	btDynamicsWorld* m_pDynamicsWorld;
	btRigidBody* m_pRigidBody;
};

