#pragma once

#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h>
#include <bullet/BulletSoftBody/btSoftRigidDynamicsWorld.h>
#include <bullet/BulletSoftBody/btSoftBodyHelpers.h>

#include <glSkel/BulletDebugDrawer.h>

class PhysicsSystem
{
public:
	PhysicsSystem();
	virtual ~PhysicsSystem();

	bool init();

	void update();

	btDynamicsWorld* getDynamicsWorld();
	btSoftRigidDynamicsWorld* getSoftDynamicsWorld();
	BulletDebugDrawer* getDebugDrawer();

private:
	btDynamicsWorld* m_pDynamicsWorld;

	btDefaultCollisionConfiguration* m_pCollisionConfiguration;
	btCollisionDispatcher* m_pDispatcher;
	btBroadphaseInterface* m_pBroadphase;
	btSequentialImpulseConstraintSolver* m_pSolver;

	btVector3 worldAabbMin, worldAabbMax;

	BulletDebugDrawer* m_pDebugDrawer;
};

