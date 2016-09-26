#include "Slatissima.h"

#include <glSkel/GeometryStrip.h>
#include <glSkel/BulletDebugDrawer.h>

#include <algorithm>
#include <cmath>

#include <bullet/BulletSoftBody/btSoftBodyHelpers.h>

const float lengthGridSpacing = 3.f; // cm, approx
const unsigned int center_nVertsWide = 3u;
const unsigned int edge_nVertsWide = 3u;
const float edgeCutoffPercent = 0.05f;

Slatissima::Slatissima(GLfloat length_cm, GLfloat width_cm, GLfloat edgeWaveAmplitude_cm, float solidThickness)
	: length(length_cm)
	, width(width_cm)
	, edgeWaveAmplitude(edgeWaveAmplitude_cm)
	, m_bPhysicsInit(false)
	, m_bSolidMesh(solidThickness > 0.f)
	, m_pDynamicsWorld(NULL)
	, m_pSoftBody(NULL)
{
	this->nVertsTall = static_cast<GLuint>(length_cm / lengthGridSpacing);

	this->buildModel();
	if(m_bSolidMesh) mesh->solidify(solidThickness);
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
	glm::mat4 trans = glm::translate(glm::mat4(1.f), mesh->getPosition());
	glm::mat4 rot = glm::mat4_cast(mesh->getRotation());
	glm::mat4 m = glm::inverse(trans * rot);
	glm::vec3 d = glm::vec3(m * glm::vec4(dir.getX(), dir.getY(), dir.getZ(), 0.f));
	m_pSoftBody->activate();
	m_pSoftBody->addForce(btVector3(d.x, d.y, d.z));
}

void Slatissima::anchorToBody(btRigidBody * body)
{
	for (int i = 0; i < center_nVertsWide; ++i)
	{
		//m_pSoftBody->setMass(i, 0.f);
		//m_pSoftBody->setMass(mesh->m_vOpposingVertPairs[i], 0.f);
		m_pSoftBody->appendAnchor(i, body);
		m_pSoftBody->appendAnchor(mesh->m_vOpposingVertPairs[i], body);
	}
}

void Slatissima::update()
{
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

	this->mesh->updateMeshSerial(data_serialized);

	if (m_pDynamicsWorld->getDebugDrawer() && (m_pDynamicsWorld->getDebugDrawer()->getDebugMode() & (btIDebugDraw::DBG_DrawWireframe)))
	{
		static_cast<BulletDebugDrawer*>(m_pDynamicsWorld->getDebugDrawer())->setTransform(m_pSoftBody->getWorldTransform());

		btSoftBodyHelpers::DrawFrame(m_pSoftBody, m_pDynamicsWorld->getDebugDrawer());
		btSoftBodyHelpers::Draw(m_pSoftBody, m_pDynamicsWorld->getDebugDrawer(), fDrawFlags::Nodes | fDrawFlags::Faces | fDrawFlags::Anchors | fDrawFlags::Contacts);
	}
}

void Slatissima::Draw(Shader s)
{
	mesh->Draw(s);
}

void Slatissima::initPhysics(btSoftRigidDynamicsWorld* dynamicsWorld)
{
	m_pDynamicsWorld = dynamicsWorld;

	btSoftBodyWorldInfo &sbInfo = m_pDynamicsWorld->getWorldInfo();

	std::vector<int> inds;
	std::vector<glm::vec3> verts;
	mesh->getIndexedVertices(inds, verts);
	m_pSoftBody = btSoftBodyHelpers::CreateFromTriMesh(sbInfo
		, (btScalar*)&verts[0]
		, &inds[0]
		, (int)mesh->getFaceCount()
		, true
	);

	btSoftBody::Material *supportLinkMat = new btSoftBody::Material();
	//m_pSoftBody->m_cfg.collisions |= btSoftBody::fCollision::VF_SS;

	m_pSoftBody->m_materials[0]->m_flags |= btSoftBody::fMaterial::DebugDraw;

	if (m_bSolidMesh)
	{
		supportLinkMat->m_kLST = 1.f;
		supportLinkMat->m_kAST = 1.f;
		supportLinkMat->m_kVST = 1.f;
		//supportLinkMat->m_flags |= btSoftBody::fMaterial::DebugDraw;

		for (auto p : mesh->m_vOpposingVertPairs)
			m_pSoftBody->appendLink(p.first, p.second, supportLinkMat);

		m_pSoftBody->generateBendingConstraints(2, supportLinkMat);
	}

	m_pSoftBody->randomizeConstraints();
	btQuaternion o(mesh->getRotation().x, mesh->getRotation().y, mesh->getRotation().z, mesh->getRotation().w);
	btVector3 pos(mesh->getPosition().x, mesh->getPosition().y, mesh->getPosition().z);
	btTransform trans(o, pos);
	m_pSoftBody->transform(trans);
	m_pSoftBody->setWorldTransform(trans);;
	m_pSoftBody->setTotalMass(10, true);
	
	this->m_pDynamicsWorld->addSoftBody(m_pSoftBody);

	sbInfo.m_sparsesdf.Reset();

	m_bPhysicsInit = true;
}

void Slatissima::buildModel()
{
	std::vector<std::vector<glm::vec3>> vertices; // row major
	glm::vec3 tempVert;
	
	// CENTRAL BLADE VERTICES
	float centerBladeWidthPercent = 0.25f;
	float centerBladeWidth = width * centerBladeWidthPercent;
	for (GLuint row = 0; row < nVertsTall; ++row)
	{
		GLfloat heightRatio = static_cast<GLfloat>(row) / static_cast<GLfloat>(nVertsTall - 1);

		std::vector<glm::vec3> vecRow;

		for (GLuint col = 0; col < center_nVertsWide; ++col)
		{
			GLfloat widthRatio = static_cast<GLfloat>(col) / static_cast<GLfloat>(center_nVertsWide - 1);

			GLfloat displacement = -(centerBladeWidth / 2.f) + widthRatio * centerBladeWidth;
			GLfloat sineOffset = sin((0.1f + 0.8f * heightRatio) * glm::pi<GLfloat>());
			tempVert.x = sineOffset * displacement;

			tempVert.y = heightRatio * length;
						
			tempVert.z = 0.f;

			vecRow.push_back(tempVert);
		}

		vertices.push_back(vecRow);
	}

	GeometryStrip g(vertices);
	
	vertices.clear();

	float edgeWidthPercent = 0.375f;
	float edgeWidth = width * edgeWidthPercent;
	
	generateGabors(-width / 2.f);

	for (GLuint row = 0; row < nVertsTall; ++row)
	{
		std::vector<glm::vec3> vecRow;
		GLfloat heightRatio = static_cast<GLfloat>(row) / static_cast<GLfloat>(nVertsTall - 1);

		for (GLuint col = 0; col < edge_nVertsWide; ++col)
		{
			GLfloat widthRatio = static_cast<GLfloat>(col) / static_cast<GLfloat>(edge_nVertsWide - 1);

			tempVert.x = (widthRatio - 0.5f) * edgeWidth * calculateEnvelope(heightRatio, 0.f, 0.1f, 0.9f, 1.f);
			
			tempVert.y = heightRatio * length;

			tempVert.z = 0.f;
			for(auto g : gabors)
				tempVert.z += g->get(glm::vec2(tempVert));

			tempVert.z *= calculateEnvelope(heightRatio, 0.05f, 0.1f, 0.9f, 0.95f);
			vecRow.push_back(tempVert);
		}

		vertices.push_back(vecRow);
	}

	GeometryStrip g2(vertices);

	g.glueLeft(g2);

	generateGabors(width / 2.f);

	vertices.clear();
	for (GLuint row = 0; row < nVertsTall; ++row)
	{
		std::vector<glm::vec3> vecRow;
		GLfloat heightRatio = static_cast<GLfloat>(row) / static_cast<GLfloat>(nVertsTall - 1);

		for (GLuint col = 0; col < edge_nVertsWide; ++col)
		{
			GLfloat widthRatio = static_cast<GLfloat>(col) / static_cast<GLfloat>(edge_nVertsWide - 1);

			tempVert.x = (widthRatio - 0.5f) * edgeWidth * calculateEnvelope(heightRatio, 0.f, 0.1f, 0.9f, 1.f);

			tempVert.y = heightRatio * length;

			tempVert.z = 0.f;
			for (auto g : gabors)
				tempVert.z += g->get(glm::vec2(tempVert));

			tempVert.z *= calculateEnvelope(heightRatio, 0.05f, 0.1f, 0.9f, 0.95f);
			vecRow.push_back(tempVert);
		}

		vertices.push_back(vecRow);
	}

	GeometryStrip g3(vertices);

	g.glueRight(g3);

	std::cout << "Creating DCEL mesh from geometry strip that is " << g.getWidthVertexCount() << " verts wide and " << g.getHeightVertexCount() << " verts long" << std::endl;
	mesh = new Mesh(g.getVertices(), g.getIndices(), this->loadTextures());
}

void Slatissima::generateGabors(float x)
{
	gabors.clear();

	unsigned int nLFWaves = 20u;
	unsigned int nHFWaves = 1000u;

	for (unsigned int i = 0; i < nLFWaves; ++i)
	{
		float y = length * (i / (nLFWaves - 1.f));
		//float y = length * getRandRatio();
		//float y = length / 2.f;
		Gabor *mainG = new Gabor();
		mainG->setGaussianKernelCenter(glm::vec2(x, y));
		mainG->setGaussianKernelSpread(glm::vec2(width / 5.f, length / nLFWaves));
		mainG->setGaussianKernelAmplitude(edgeWaveAmplitude * (0.75f + 0.25f * getRandRatio()));
		mainG->setComplexSinusoidDistance(length / (5.f + 10.f * getRandRatio()));

		gabors.push_back(mainG);
	}

	for (unsigned int i = 0; i < nHFWaves; ++i)
	{
		//float y = length * (i / (nHFWaves - 1.f));
		float y = length * getRandRatio();
		Gabor *g = new Gabor();
		g->setGaussianKernelCenter(glm::vec2(x, y));
		g->setGaussianKernelSpread(glm::vec2(width / 10.f, length / nHFWaves));
		g->setGaussianKernelAmplitude(edgeWaveAmplitude * 2.f);
		g->setComplexSinusoidDistance(0.5f + 5.5f * getRandRatio());

		gabors.push_back(g);
	}
}

float Slatissima::calculateEnvelope(float currentRatio, float beginRatio, float maxRatio1, float maxRatio2, float endRatio)
{
	if (currentRatio < beginRatio || currentRatio > endRatio)
		return 0.f;

	if (currentRatio > maxRatio1 && currentRatio < maxRatio2)
		return 1.f;
	
	if (currentRatio < maxRatio1)
	{
		float r = (maxRatio1 - currentRatio) / (maxRatio1 - beginRatio);
		return sin((1.f - r) * glm::half_pi<GLfloat>());
	}
	else // currentRatio > maxRatio2
	{
		float r = (maxRatio2 - currentRatio) / (maxRatio2 - endRatio);
		return sin((1.f - r) * glm::half_pi<GLfloat>());
	}
}

float Slatissima::getRandRatio()
{
	return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
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