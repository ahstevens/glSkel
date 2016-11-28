#include "PhysicsSystem.h"

PhysicsSystem::PhysicsSystem()
	: m_pDynamicsWorld(NULL)
	, m_pCollisionConfiguration(NULL)
	, m_pDispatcher(NULL)
	, m_pBroadphase(NULL)
	, m_pSolver(NULL)
	, m_pDebugDrawer(NULL)
{
}

PhysicsSystem::~PhysicsSystem()
{
}

bool PhysicsSystem::init()
{
	m_pCollisionConfiguration = new btSoftBodyRigidBodyCollisionConfiguration();
	m_pDispatcher = new	btCollisionDispatcher(m_pCollisionConfiguration);

	worldAabbMin = btVector3(-1000, -1000, -1000);
	worldAabbMax = btVector3(1000, 1000, 1000);
	m_pBroadphase = new btAxisSweep3(worldAabbMin, worldAabbMax, 32766U);

	m_pSolver = new btSequentialImpulseConstraintSolver();

	m_pDynamicsWorld = new btSoftRigidDynamicsWorld(m_pDispatcher, m_pBroadphase, m_pSolver, m_pCollisionConfiguration);

	btSoftBodyWorldInfo &sbInfo = static_cast<btSoftRigidDynamicsWorld*>(m_pDynamicsWorld)->getWorldInfo();
	//sbInfo.m_gravity = btVector3(0.f, 0.f, 0.f);
	//sbInfo.m_gravity = btVector3(0.f, -9.8f, 0.f);
	sbInfo.m_gravity = btVector3(1.f, 3.f, -0.5f);
	sbInfo.m_dispatcher = m_pDispatcher;
	sbInfo.m_broadphase = m_pBroadphase;
	sbInfo.m_sparsesdf.Initialize();

	m_pDebugDrawer = new BulletDebugDrawer();
	m_pDebugDrawer->setDebugMode(btIDebugDraw::DBG_DrawWireframe);
	m_pDynamicsWorld->setDebugDrawer(m_pDebugDrawer);

	return true;
}

void PhysicsSystem::update()
{
	m_pDynamicsWorld->stepSimulation(1.f / 120.f, 10);
}

btDynamicsWorld * PhysicsSystem::getDynamicsWorld()
{
	return m_pDynamicsWorld;
}

BulletDebugDrawer * PhysicsSystem::getDebugDrawer()
{
	return m_pDebugDrawer;
}
