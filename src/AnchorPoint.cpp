#include "AnchorPoint.h"

AnchorPoint::AnchorPoint(glm::vec3 position, btDynamicsWorld* dynamicsWorld)
	: Object(position, glm::mat3())
	, m_bPhysicsInit(false)
	, m_pDynamicsWorld(dynamicsWorld)
	, m_pRigidBody(NULL)
{
	buildModel();
	initPhysics();
}


AnchorPoint::~AnchorPoint()
{
	if (m_pMesh)
		delete(m_pMesh);

	m_pDynamicsWorld->removeCollisionObject(m_pRigidBody);
	delete m_pRigidBody;
}

btRigidBody * AnchorPoint::getRigidBody()
{
	return m_pRigidBody;
}

void AnchorPoint::Draw(Shader s)
{
	glm::mat4 modelMat = glm::translate(glm::mat4(), m_vec3Position) * glm::mat4(m_mat3Rotation);
	m_pMesh->Draw(s, modelMat);
}

void AnchorPoint::initPhysics()
{
	
	btCollisionShape* collisionShape = new btEmptyShape();
	{
		btScalar mass(0.f);
		btVector3 localInertia(0.f, 0.f, 0.f);

		//using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState();
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, collisionShape, localInertia);
		m_pRigidBody = new btRigidBody(rbInfo);

		//add the body to the dynamics world
		m_pDynamicsWorld->addRigidBody(m_pRigidBody);
	}

	m_bPhysicsInit = true;
}

void AnchorPoint::buildModel()
{
	std::vector<glm::vec3> vertices;
	std::vector<unsigned int> indices;

	float halfWidth = 0.1f;
	float halfLength = 0.1f;
	float halfDepth = 0.1f;

	vertices.push_back(glm::vec3(-halfWidth, halfDepth, -halfLength));  // 0
	vertices.push_back(glm::vec3(halfWidth, halfDepth, -halfLength));   // 1
	vertices.push_back(glm::vec3(halfWidth, halfDepth, halfLength));    // 2
	vertices.push_back(glm::vec3(-halfWidth, halfDepth, halfLength));   // 3

	vertices.push_back(glm::vec3(halfWidth, -halfDepth, -halfLength));  // 4
	vertices.push_back(glm::vec3(-halfWidth, -halfDepth, -halfLength)); // 5
	vertices.push_back(glm::vec3(-halfWidth, -halfDepth, halfLength));  // 6
	vertices.push_back(glm::vec3(halfWidth, -halfDepth, halfLength));   // 7

	//top face
	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(0);
	indices.push_back(2);
	indices.push_back(3);

	//bottom face
	indices.push_back(4);
	indices.push_back(5);
	indices.push_back(6);
	indices.push_back(4);
	indices.push_back(6);
	indices.push_back(7);

	//left face
	indices.push_back(6);
	indices.push_back(5);
	indices.push_back(0);
	indices.push_back(6);
	indices.push_back(0);
	indices.push_back(3);

	//right face
	indices.push_back(4);
	indices.push_back(7);
	indices.push_back(2);
	indices.push_back(4);
	indices.push_back(2);
	indices.push_back(1);

	//front face
	indices.push_back(0);
	indices.push_back(5);
	indices.push_back(4);
	indices.push_back(0);
	indices.push_back(4);
	indices.push_back(1);

	//back face
	indices.push_back(2);
	indices.push_back(7);
	indices.push_back(6);
	indices.push_back(2);
	indices.push_back(6);
	indices.push_back(3);

	m_pMesh = new Mesh(vertices, indices, this->loadTextures());
}

std::vector<Texture> AnchorPoint::loadTextures()
{
	// Load textures
	Texture diffuseMap, specularMap;
	glGenTextures(1, &diffuseMap.id);
	glGenTextures(1, &specularMap.id);
	int width = 1, height = 1;
	unsigned char image[3] = { 0xFF, 0x00, 0x00 };

	// Diffuse map
	diffuseMap.type = "texture_diffuse";
	glBindTexture(GL_TEXTURE_2D, diffuseMap.id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, &image);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);

	// Specular map
	specularMap.type = "texture_specular";
	glBindTexture(GL_TEXTURE_2D, specularMap.id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, &image);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	std::vector<Texture> textures = { diffuseMap, specularMap };

	return textures;
}
