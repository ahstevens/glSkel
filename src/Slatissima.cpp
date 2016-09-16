#include "Slatissima.h"

#include <glSkel/GeometryStrip.h>

#include <algorithm>
#include <cmath>

#include <bullet/BulletSoftBody/btSoftRigidDynamicsWorld.h>
#include <bullet/BulletSoftBody/btSoftBodyHelpers.h>

const float gridSpacing = 0.5f; // cm, approx

Slatissima::Slatissima(GLfloat length_cm, GLfloat width_cm, GLfloat thickness_cm, std::vector<Gabor*> g, btDiscreteDynamicsWorld* dynamicsWorld)
{
	this->length = length_cm;
	this->width = width_cm; 
	this->thickness = thickness_cm;
	this->gabors = g;
	this->m_pDynamicsWorld = dynamicsWorld;
	this->nVertsTall = static_cast<GLuint>(length_cm / gridSpacing);
	this->nVertsWide = static_cast<GLuint>(width_cm / gridSpacing);
	this->buildStrip();

	this->initPhysics();
}


Slatissima::~Slatissima()
{
	if (mesh)
		delete(mesh);

	m_pDynamicsWorld->removeCollisionObject(m_pSoftBody);
	delete m_pSoftBody;
}

void Slatissima::rotateX(float degrees)
{
	glm::quat q = glm::quat(glm::vec3(glm::radians(degrees), 0.f, 0.f));
	this->mesh->addRotation(q);
}

void Slatissima::rotateY(float degrees)
{
	glm::quat q = glm::quat(glm::vec3(0.f, glm::radians(degrees), 0.f));
	this->mesh->addRotation(q);
}

void Slatissima::rotateZ(float degrees)
{
	glm::quat q = glm::quat(glm::vec3(0.f, 0.f, glm::radians(degrees)));
	this->mesh->addRotation(q);
}

void Slatissima::setOrientation(glm::quat orientation) { this->mesh->setRotation(orientation); }

glm::quat Slatissima::getOrientation()
{
	return mesh->getRotation();;
}

void Slatissima::setPosition(glm::vec3 pos)
{
	mesh->setPosition(pos);
}

glm::vec3 Slatissima::getPosition()
{
	return mesh->getPosition();
}

void Slatissima::bump(btVector3 dir)
{
	m_pSoftBody->activate();
	m_pSoftBody->addForce(dir);
}

void Slatissima::update()
{
	//btTransform trans;
	//m_pRigidBody->getMotionState()->getWorldTransform(trans);

	//const btVector3 o = trans.getOrigin();
	//const btQuaternion q = trans.getRotation();
	//this->mesh->setPosition(glm::vec3(o.getX(), o.getY(), o.getZ()));
	//this->mesh->setRotation(glm::quat(q.getW(), q.getX(), q.getY(), q.getZ()));
	btAlignedObjectArray<btSoftBody::Node> nodes = m_pSoftBody->m_nodes;
	std::vector<float> data_serialized;
	for (size_t i = 0; i < nodes.size(); ++i)
	{
		data_serialized.push_back(nodes[i].m_x.getX());
		data_serialized.push_back(nodes[i].m_x.getY());
		data_serialized.push_back(nodes[i].m_x.getZ());
		data_serialized.push_back(nodes[i].m_n.getX());
		data_serialized.push_back(nodes[i].m_n.getY());
		data_serialized.push_back(nodes[i].m_n.getZ());
		data_serialized.push_back(0.5f);
		data_serialized.push_back(0.5f);
	}
	//m_pSoftBody->m_faces[0].m_n[0]->
	this->mesh->updateMeshSerial(data_serialized);
}

void Slatissima::Draw(Shader s)
{
	mesh->Draw(s);
}

void Slatissima::initPhysics()
{
	std::vector<int> inds;
	std::vector<glm::vec3> verts;
	mesh->getIndexedVertices(inds, verts);

	btSoftBodyWorldInfo sbInfo;
	sbInfo.air_density = (btScalar)1.2;
	sbInfo.m_gravity.setValue(0, -9.81, 0);
	sbInfo.m_dispatcher = m_pDynamicsWorld->getDispatcher();
	sbInfo.m_sparsesdf.Reset();
	sbInfo.m_broadphase = m_pDynamicsWorld->getBroadphase();
	sbInfo.m_sparsesdf.Initialize();

	m_pSoftBody = btSoftBodyHelpers::CreateFromTriMesh(static_cast<btSoftRigidDynamicsWorld*>(m_pDynamicsWorld)->getWorldInfo()
		, (btScalar*)&verts[0]
		, &inds[0]
		, (int)mesh->getFaceCount()
	);
	btSoftBody::Material* pm = m_pSoftBody->appendMaterial();
	pm->m_kLST = 0.9;
	m_pSoftBody->m_cfg.piterations = 2;
	m_pSoftBody->m_cfg.kDF = 0.5;
	m_pSoftBody->m_cfg.collisions |= btSoftBody::fCollision::VF_SS;
	m_pSoftBody->generateClusters(2);
	m_pSoftBody->randomizeConstraints();
	m_pSoftBody->setTotalMass(300, true);

	static_cast<btSoftRigidDynamicsWorld*>(this->m_pDynamicsWorld)->addSoftBody(m_pSoftBody);
}

void Slatissima::buildStrip()
{
	std::vector<std::vector<glm::vec3>> vertices; // row major
	glm::vec3 tempVert;

	Gabor* gabor = new Gabor();
	gabor->setGaussianKernelCenter(glm::vec2(-width / 2.f, length / 2.f));
	gabor->setGaussianKernelSpread(glm::vec2(width / 4.f, length / 6.f));
	gabor->setGaussianKernelAngle(0.f);
	gabor->setGaussianKernelAmplitude(0.8f);
	gabor->setComplexSinusoidDistance(0.8f);
	gabor->setComplexSinusoidAngle(0.f);

	gabors.push_back(gabor);

	// CENTRAL BLADE VERTICES
	float centerBladeWidthPercent = 0.33f;
	float centerBladeWidth = width * centerBladeWidthPercent;
	for (GLuint row = 0; row < nVertsTall; ++row)
	{
		GLfloat heightRatio = static_cast<GLfloat>(row) / static_cast<GLfloat>(nVertsTall - 1);

		std::vector<glm::vec3> vecRow;

		for (GLuint col = 0; col < nVertsWide * centerBladeWidthPercent; ++col)
		{
			GLfloat widthRatio = static_cast<GLfloat>(col) / static_cast<GLfloat>(nVertsWide * centerBladeWidthPercent - 1);

			GLfloat displacement = -(centerBladeWidth / 2.f) + widthRatio * centerBladeWidth;
			GLfloat sineOffset = sin(heightRatio * glm::pi<GLfloat>());
			tempVert.x = sineOffset * displacement;
			//tempVert.x = displacement * 0.5f;
			tempVert.y = heightRatio * length;
						
			tempVert.z = 0.f;

			vecRow.push_back(tempVert);
		}

		vertices.push_back(vecRow);
	}

	GeometryStrip g(vertices);
	
	vertices.clear();

	float edgeWidthPercent = 0.5f;
	float edgeWidth = width * edgeWidthPercent;
	for (GLuint row = 0; row < nVertsTall; ++row)
	{
		std::vector<glm::vec3> vecRow;
		GLfloat heightRatio = static_cast<GLfloat>(row) / static_cast<GLfloat>(nVertsTall - 1);

		for (GLuint col = 0; col < nVertsWide * edgeWidthPercent; ++col)
		{
			GLfloat widthRatio = static_cast<GLfloat>(col) / static_cast<GLfloat>(nVertsWide * edgeWidthPercent - 1);

			tempVert.x = (widthRatio - 0.5f) * edgeWidth * sin(heightRatio * glm::pi<GLfloat>());
			//tempVert.x = (widthRatio - 0.5f) * width * 0.5f;
			tempVert.y = heightRatio * length;

			tempVert.z = 0.f;
			for(auto g : gabors)
				tempVert.z += g->get(glm::vec2(tempVert));

			vecRow.push_back(tempVert);
		}

		vertices.push_back(vecRow);
	}

	GeometryStrip g2(vertices);

	g.glueLeft(g2);

	gabor->setGaussianKernelCenter(glm::vec2(width / 2.f, length / 2.f));

	vertices.clear();
	for (GLuint row = 0; row < nVertsTall; ++row)
	{
		std::vector<glm::vec3> vecRow;
		GLfloat heightRatio = static_cast<GLfloat>(row) / static_cast<GLfloat>(nVertsTall - 1);

		for (GLuint col = 0; col < nVertsWide * edgeWidthPercent; ++col)
		{
			GLfloat widthRatio = static_cast<GLfloat>(col) / static_cast<GLfloat>(nVertsWide * edgeWidthPercent - 1);

			tempVert.x = (widthRatio - 0.5f) * edgeWidth * sin(heightRatio * glm::pi<GLfloat>());
			//tempVert.x = (widthRatio - 0.5f) * width * 0.5f;
			tempVert.y = heightRatio * length;

			tempVert.z = 0.f;
			for (auto g : gabors)
				tempVert.z += g->get(glm::vec2(tempVert));

			vecRow.push_back(tempVert);
		}

		vertices.push_back(vecRow);
	}

	GeometryStrip g3(vertices);

	g.glueRight(g3);

	std::cout << "Creating DCEL mesh from geometry strip that is " << g.getWidthVertexCount() << " verts wide and " << g.getHeightVertexCount() << " verts long" << std::endl;
	mesh = new Mesh(g.getVertices(), g.getIndices(), this->loadTextures());
}

std::vector<Texture> Slatissima::loadTextures()
{
	// Load textures
	Texture diffuseMap, specularMap;
	glGenTextures(1, &diffuseMap.id);
	glGenTextures(1, &specularMap.id);
	int width = 1, height = 1;
	unsigned char image[3] = { 0x55, 0xFF, 0x11 };

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