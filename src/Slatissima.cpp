#include "Slatissima.h"

#include <GLFW\glfw3.h>
#include <glSkel/GeometryStrip.h>
#include <glSkel/BulletDebugDrawer.h>

#include <algorithm>
#include <cmath>
#include <random>
#include <fstream>
#include <sys/stat.h> // stat()

#include <bullet/BulletSoftBody/btSoftBodyHelpers.h>

extern std::default_random_engine generator;

const float lengthGridSpacing = 1.5f; // cm, approx
const unsigned int center_nVertsWide = 3u;
const unsigned int edge_nVertsWide = 3u;
const float edgeCutoffPercent = 0.05f;

const float rayStrength = 0.01f;

const float L_AVG = 148.1f;        // avg length
const float L_STD = 55.26f;        // std. dev. length
const float W_AVG = 23.73f;        // avg width
const float W_STD = 6.933f;        // std. dev. width
const float P_AVG = 19.97f;        // avg periodicity
const float P_STD = 3.446f;        // std. dev. periodicity
const float LW_RATIO_AVG = 6.277f;
const float LW_RATIO_STD = 2.010f;
const float LP_RATIO_AVG = 7.738f;
const float LP_RATIO_STD = 2.389f;

struct Payload
{
	int index;
	float x0;
	float y0;
	float z0;
};

Slatissima::Slatissima(float solidThickness, glm::vec3 position, glm::mat3 orientation, btSoftRigidDynamicsWorld* dynamicsWorld)
	: Object(position, orientation)
	, m_bPhysicsInit(false)
	, m_bSolidMesh(solidThickness > 0.f)
	, m_pDynamicsWorld(dynamicsWorld)
	, m_pSoftBody(NULL)
	, m_debugDrawFlags(0)
{
	std::normal_distribution<float> length_dist(L_AVG, L_STD);
	std::normal_distribution<float> lwr_dist(LW_RATIO_AVG, LW_RATIO_STD);
	std::normal_distribution<float> lpr_dist(LP_RATIO_AVG, LP_RATIO_STD);
	std::normal_distribution<float> waveAmp_dist(LP_RATIO_AVG, 1.f);

	m_fLength = length_dist(generator);
	m_fWidth = m_fLength / lwr_dist(generator);
	m_fEdgeWaveAmplitude = waveAmp_dist(generator);
	
	std::cout << "Length: " << m_fLength << " | Width: " << m_fWidth << std::endl;

	this->nVertsTall = static_cast<GLuint>(m_fLength / lengthGridSpacing);

	this->buildModel();
	if (m_bSolidMesh) mesh->solidify(solidThickness);

	initPhysics();
}

Slatissima::Slatissima(float solidThickness, float length, float width, float edgeWaveAmplitude, glm::vec3 position, glm::mat3 orientation, btSoftRigidDynamicsWorld* dynamicsWorld)
	: Object(position, orientation)
	, m_bPhysicsInit(false)
	, m_bSolidMesh(solidThickness > 0.f)
	, m_pDynamicsWorld(dynamicsWorld)
	, m_pSoftBody(NULL)
	, m_debugDrawFlags(0)
{
	m_fLength = length;
	m_fWidth = width;
	m_fEdgeWaveAmplitude = edgeWaveAmplitude;

	std::cout << "Length: " << m_fLength << " | Width: " << m_fWidth << std::endl;

	this->nVertsTall = static_cast<GLuint>(m_fLength / lengthGridSpacing);

	this->buildModel();
	if (m_bSolidMesh) mesh->solidify(solidThickness);

	initPhysics();
}


Slatissima::~Slatissima()
{
	if (mesh)
		delete(mesh);

	m_pDynamicsWorld->removeCollisionObject(m_pSoftBody);
	delete m_pSoftBody;
}

void Slatissima::setDebugDrawFlags(int flags)
{
	m_debugDrawFlags = flags;
}

int Slatissima::getDebugDrawFlags()
{
	return m_debugDrawFlags;
}

void Slatissima::toggleDebugDrawFlag(int flag)
{
	m_debugDrawFlags ^= flag;
}

float Slatissima::getLength()
{
	return m_fLength;
}

float Slatissima::getWidth()
{
	return m_fWidth;
}

void Slatissima::bump(btVector3 dir)
{
	m_pSoftBody->activate();
	m_pSoftBody->addForce(dir);
}

void Slatissima::pinToBody(float lengthRatio, glm::vec3 kernel, btRigidBody * body, float influence, bool disableCollisionsWithBody, bool convergeAnchors, bool anchorInPlace)
{
	pinToBody(lengthRatio, kernel.x, kernel.y, kernel.z, body, influence, disableCollisionsWithBody, convergeAnchors, anchorInPlace);
}

void Slatissima::pinToBody(float lengthRatio, float kernelX, float kernelY, float kernelZ, btRigidBody * body, float influence, bool disableCollisionsWithBody, bool convergeAnchors, bool anchorInPlace)
{
	float targetY = lengthRatio * this->m_fLength;

	glm::vec3 worldPt = glm::vec3(glm::translate(glm::mat4(), m_vec3Position) * glm::mat4(m_mat3Rotation) * glm::vec4(0.f, targetY, 0.f, 1.f));

	pinToBody(worldPt.x, worldPt.y, worldPt.z, kernelX, kernelY, kernelZ, body, influence, disableCollisionsWithBody, convergeAnchors, anchorInPlace);
}

void Slatissima::pinToBody(glm::vec3 worldPos, glm::vec3 kernelSize, btRigidBody * body, float influence, bool disableCollisionsWithBody, bool convergeAnchors, bool anchorInPlace)
{
	pinToBody(worldPos.x, worldPos.y, worldPos.z, kernelSize.x, kernelSize.y, kernelSize.z, body, influence, disableCollisionsWithBody, convergeAnchors, anchorInPlace);
}

void Slatissima::pinToBody(float worldX, float worldY, float worldZ, float kernelX, float kernelY, float kernelZ, btRigidBody * body, float influence, bool disableCollisionsWithBody, bool convergeAnchors, bool anchorInPlace)
{
	glm::vec4 worldPt(worldX, worldY, worldZ, 1.f);

	glm::mat4 modelToWorldMat = glm::translate(glm::mat4(), m_vec3Position) * glm::mat4(m_mat3Rotation);
	glm::mat4 worldToModelMat = glm::inverse(modelToWorldMat);
	// transform world position into model space
	glm::vec4 modelPt = worldToModelMat * worldPt;
	
	std::vector<int> nodeIndices = mesh->getClosestVertexIndicesKernel(modelPt.x, modelPt.y, modelPt.z, kernelX, kernelY, kernelZ);

	// short circuit if no node indices found within kernel
	if (nodeIndices.size() < 1)
	{
		std::cerr << "pinToBody: No nodes found near model coordinate (" << modelPt.x << ", " << modelPt.y << ", " << modelPt.z << ") using kernel (" << kernelX << ", " << kernelY << ", " << kernelZ << ")" << std::endl;
		return;
	}

	// get centroid (model space)
	glm::vec3 centroidPt = mesh->getCentroidPosition(nodeIndices);
	
	// calc direction vec
	glm::vec3 centroidToBodyVec;
	if (!convergeAnchors)
	{
		glm::vec3 bodyOriginPt(body->getWorldTransform().getOrigin().getX(), body->getWorldTransform().getOrigin().getY(), body->getWorldTransform().getOrigin().getZ());
		bodyOriginPt = glm::vec3(worldToModelMat * glm::vec4(bodyOriginPt, 1.f)); // transform to model space
		centroidToBodyVec = bodyOriginPt - centroidPt;
	}

	for (auto const &i : nodeIndices)
	{
		glm::vec3 worldPivot;

		// if not converging anchors, project point along centroid vector and convert to world coords
		if (convergeAnchors)
		{
			if (anchorInPlace)
				worldPivot = glm::vec3(modelToWorldMat * glm::vec4(centroidPt, 1.f));
			else
				worldPivot = glm::vec3(modelToWorldMat * glm::vec4(centroidPt + centroidToBodyVec, 1.f));
		}
		else
		{
			if (anchorInPlace)
				worldPivot = glm::vec3(modelToWorldMat * glm::vec4(mesh->getPositionAtIndex(i), 1.f));
			else
				worldPivot = glm::vec3(modelToWorldMat * glm::vec4(mesh->getPositionAtIndex(i) + centroidToBodyVec, 1.f));
		}
		
		btVector3 localPivot = body->getWorldTransform().inverse() * btVector3(worldPivot.x, worldPivot.y, worldPivot.z);

		m_pSoftBody->appendAnchor(i, body, localPivot, disableCollisionsWithBody, influence);
	}
}

void Slatissima::update()
{
	btAlignedObjectArray<btSoftBody::Node> nodes = m_pSoftBody->m_nodes;
	std::vector<float> data_serialized;
	for (size_t i = 0; i < nodes.size(); ++i)
	{
		glm::vec3 pos(nodes[i].m_x.getX(), nodes[i].m_x.getY(), nodes[i].m_x.getZ());
		glm::vec3 norm(nodes[i].m_n.getX(), nodes[i].m_n.getY(), nodes[i].m_n.getZ());
		glm::mat4 m = glm::translate(glm::mat4(), m_vec3Position) * glm::mat4(m_mat3Rotation);
		m = glm::inverse(m);
		pos = glm::vec3(m * glm::vec4(pos, 1.f));
		//norm = glm::vec3(m * glm::vec4(norm, 1.f));

		data_serialized.push_back(pos.x);
		data_serialized.push_back(pos.y);
		data_serialized.push_back(pos.z);
		data_serialized.push_back(norm.x);
		data_serialized.push_back(norm.y);
		data_serialized.push_back(norm.z);
		data_serialized.push_back(0.5f);
		data_serialized.push_back(0.5f);
	}

	this->mesh->updateMeshSerial(data_serialized);

	debugDraw();
}

void Slatissima::receiveEvent(Object* obj, const int event, void * data)
{
	if (event == BroadcastSystem::EVENT::KEY_PRESS)
	{	
		int key;
		memcpy(&key, data, sizeof(key));

		if (key == GLFW_KEY_KP_0)
			toggleDebugDrawFlag(fDrawFlags::Std);
		if (key == GLFW_KEY_KP_1)
			toggleDebugDrawFlag(fDrawFlags::Faces);
		if (key == GLFW_KEY_KP_2)
			toggleDebugDrawFlag(fDrawFlags::Nodes);
		if (key == GLFW_KEY_KP_3)
			toggleDebugDrawFlag(fDrawFlags::Links);
		if (key == GLFW_KEY_KP_4)
			toggleDebugDrawFlag(fDrawFlags::Normals);
		if (key == GLFW_KEY_KP_5)
			toggleDebugDrawFlag(fDrawFlags::Contacts);
		if (key == GLFW_KEY_KP_6)
			toggleDebugDrawFlag(fDrawFlags::Clusters);
		if (key == GLFW_KEY_KP_7)
			toggleDebugDrawFlag(fDrawFlags::Anchors);

		if (key == GLFW_KEY_O)
			bump(btVector3(0.f, -1.f, 0.f));
		if (key == GLFW_KEY_U)
			bump(btVector3(0.f, 1.f, 0.f));
		if (key == GLFW_KEY_I)
			bump(btVector3(0.f, 0.f, -1.f));

		if (key == GLFW_KEY_KP_ENTER)
			saveAsObj("test");

		if (key == GLFW_KEY_L)
		{
			//m_pSoftBody->setRestLengthScale(m_pSoftBody->getRestLengthScale() * (event == BroadcastSystem::EVENT::GROW_RAY ? growRayAmount : shrinkRayAmount));
			for (int i = 0; i < m_pSoftBody->m_links.size(); ++i)
			{
				Payload p1 = *static_cast<Payload*>(m_pSoftBody->m_links[i].m_n[0]->m_tag);
				Payload p2 = *static_cast<Payload*>(m_pSoftBody->m_links[i].m_n[1]->m_tag);

				if (mesh->isBoundaryVertex(p1.index) &&
					mesh->isBoundaryVertex(p2.index))
				{
					float ratio = ((p1.y0 + p2.y0) / 2) / m_fLength;
					float changeAmount = rayStrength * calculateEnvelope(ratio, 0.05f, 0.45f, 0.55f, 0.95f);
					m_pSoftBody->m_links[i].m_rl *= 1.f + changeAmount;
					m_pSoftBody->m_links[i].m_c1 = m_pSoftBody->m_links[i].m_rl * m_pSoftBody->m_links[i].m_rl;
				}
			}
		}

		if (key == GLFW_KEY_K)
		{
			//m_pSoftBody->setRestLengthScale(m_pSoftBody->getRestLengthScale() * (event == BroadcastSystem::EVENT::GROW_RAY ? growRayAmount : shrinkRayAmount));
			for (int i = 0; i < m_pSoftBody->m_links.size(); ++i)
			{
				Payload p1 = *static_cast<Payload*>(m_pSoftBody->m_links[i].m_n[0]->m_tag);
				Payload p2 = *static_cast<Payload*>(m_pSoftBody->m_links[i].m_n[1]->m_tag);

				if (mesh->isBoundaryVertex(p1.index) &&
					mesh->isBoundaryVertex(p2.index))
				{
					float ratio = ((p1.y0 + p2.y0) / 2) / m_fLength;
					float changeAmount = rayStrength * calculateEnvelope(ratio, 0.05f, 0.45f, 0.55f, 0.95f);
					m_pSoftBody->m_links[i].m_rl *= 1.f - changeAmount;
					m_pSoftBody->m_links[i].m_c1 = m_pSoftBody->m_links[i].m_rl * m_pSoftBody->m_links[i].m_rl;
				}
			}
		}
	}

	if (event == BroadcastSystem::EVENT::GROW_RAY || event == BroadcastSystem::EVENT::SHRINK_RAY)
	{
		glm::vec3 payload[2];
		memcpy(&payload, data, sizeof(payload));

		btSoftBody::sRayCast results;
		if (m_pSoftBody->rayTest(btVector3(payload[0].x, payload[0].y, payload[0].z), btVector3(payload[1].x, payload[1].y, payload[1].z), results))
		{
			//m_pSoftBody->setRestLengthScale(m_pSoftBody->getRestLengthScale() * (event == BroadcastSystem::EVENT::GROW_RAY ? growRayAmount : shrinkRayAmount));
			for (int i = 0; i < m_pSoftBody->m_links.size(); ++i)
			{
				Payload p1 = *static_cast<Payload*>(m_pSoftBody->m_links[i].m_n[0]->m_tag);
				Payload p2 = *static_cast<Payload*>(m_pSoftBody->m_links[i].m_n[1]->m_tag);

				if (mesh->isBoundaryVertex(p1.index) && 
					mesh->isBoundaryVertex(p2.index))
				{
					float ratioY = ((p1.y0 + p2.y0) / 2) / m_fLength;
					float changeAmount = rayStrength * calculateEnvelope(ratioY, 0.05f, 0.45f, 0.55f, 0.95f);

					// update resting length
					m_pSoftBody->m_links[i].m_rl *= 1.f + (event == BroadcastSystem::EVENT::GROW_RAY ? changeAmount : -changeAmount);

					// update resting length squared (c1)
					m_pSoftBody->m_links[i].m_c1 = m_pSoftBody->m_links[i].m_rl * m_pSoftBody->m_links[i].m_rl;
				}
			}
		}
	}
}

void Slatissima::Draw(Shader s)
{
	if (!(m_debugDrawFlags & fDrawFlags::Faces) && !(m_debugDrawFlags & fDrawFlags::Nodes))
	{
		glm::mat4 modelMat = glm::translate(glm::mat4(), m_vec3Position) * glm::mat4(m_mat3Rotation);
		mesh->Draw(s, modelMat);
	}
}

void Slatissima::initPhysics()
{
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

	// Use Bullet's Feature m_tag property to store its mesh index and original position
	for (int i = 0; i < m_pSoftBody->m_nodes.size(); ++i)
	{
		Payload* p = new Payload();
		p->index = i;
		p->x0 = m_pSoftBody->m_nodes[i].m_x.getX();
		p->y0 = m_pSoftBody->m_nodes[i].m_x.getY();
		p->z0 = m_pSoftBody->m_nodes[i].m_x.getZ();

		m_pSoftBody->m_nodes[i].m_tag = p;
	}

	m_pSoftBody->generateBendingConstraints(2);
	m_pSoftBody->m_cfg.collisions |= btSoftBody::fCollision::VF_SS;
	//m_pSoftBody->m_cfg.piterations = 5.f;

	if (m_bSolidMesh)
	{
		btSoftBody::Material *supportLinkMat = new btSoftBody::Material();
		supportLinkMat->m_kLST = 1.f;
		supportLinkMat->m_kAST = 1.f;
		supportLinkMat->m_kVST = 1.f;                                                                                           

		for (auto p : mesh->m_vOpposingVertPairs)
			m_pSoftBody->appendLink(p.first, p.second, supportLinkMat);

		m_pSoftBody->generateBendingConstraints(2, supportLinkMat);
	}

	m_pSoftBody->randomizeConstraints();

	btMatrix3x3 o; 
	o.setFromOpenGLSubMatrix(glm::value_ptr(glm::mat4(m_mat3Rotation)));
	btVector3 pos(m_vec3Position.x, m_vec3Position.y, m_vec3Position.z);
	btTransform trans(o, pos);
	m_pSoftBody->transform(trans);
	m_pSoftBody->setTotalMass(10, true);
	
	this->m_pDynamicsWorld->addSoftBody(m_pSoftBody);

	sbInfo.m_sparsesdf.Reset();

	m_pSoftBody->getCollisionShape()->setMargin(0.2f); // COLLISION MARGIN
	m_pSoftBody->setUserPointer(this);

	m_bPhysicsInit = true;
}

void Slatissima::buildModel()
{
	std::vector<std::vector<glm::vec3>> vertices; // row major
	glm::vec3 tempVert;
	
	// CENTRAL BLADE VERTICES
	float centerBladeWidthPercent = 0.25f;
	float centerBladeWidth = m_fWidth * centerBladeWidthPercent;
	for (GLuint row = 0; row < nVertsTall; ++row)
	{
		GLfloat dy = static_cast<GLfloat>(row) / static_cast<GLfloat>(nVertsTall - 1);

		std::vector<glm::vec3> vecRow;

		for (GLuint col = 0; col < center_nVertsWide; ++col)
		{
			GLfloat dx = static_cast<GLfloat>(col) / static_cast<GLfloat>(center_nVertsWide - 1);

			GLfloat displacement = -(centerBladeWidth / 2.f) + dx * centerBladeWidth;
			GLfloat sineOffset = sin((0.1f + 0.8f * dy) * glm::pi<GLfloat>());
			tempVert.x = sineOffset * displacement;

			tempVert.y = dy * m_fLength;
						
			tempVert.z = 0.f;

			vecRow.push_back(tempVert);
		}

		vertices.push_back(vecRow);
	}

	GeometryStrip g(vertices);
	
	vertices.clear();

	float edgeWidthPercent = (1.f - centerBladeWidthPercent) * 0.5f;
	float edgeWidth = m_fWidth * edgeWidthPercent;
	
	generateGabors(-m_fWidth / 2.f);

	for (GLuint row = 0; row < nVertsTall; ++row)
	{
		std::vector<glm::vec3> vecRow;
		GLfloat dy = static_cast<GLfloat>(row) / static_cast<GLfloat>(nVertsTall - 1);

		for (GLuint col = 0; col < edge_nVertsWide; ++col)
		{
			GLfloat dx = static_cast<GLfloat>(col) / static_cast<GLfloat>(edge_nVertsWide - 1);

			tempVert.x = (dx - 0.5f) * edgeWidth * calculateEnvelope(dy, 0.f, 0.1f, 0.9f, 1.f);
			
			tempVert.y = dy * m_fLength;

			tempVert.z = 0.f;
			//for(auto g : gabors)
			//	tempVert.z += g->get(glm::vec2(tempVert));

			//tempVert.z *= calculateEnvelope(dy, 0.05f, 0.1f, 0.9f, 0.95f);
			vecRow.push_back(tempVert);
		}

		vertices.push_back(vecRow);
	}

	GeometryStrip g2(vertices);

	g.glueLeft(g2);

	//generateGabors(width / 2.f);
	for (int i = 0; i < gabors.size(); ++i)
		gabors[i]->setGaussianKernelCenter(glm::vec2(m_fWidth / 2.f, gabors[i]->getGaussianKernelCenter().y));

	vertices.clear();
	for (GLuint row = 0; row < nVertsTall; ++row)
	{
		std::vector<glm::vec3> vecRow;
		GLfloat dy = static_cast<GLfloat>(row) / static_cast<GLfloat>(nVertsTall - 1);

		for (GLuint col = 0; col < edge_nVertsWide; ++col)
		{
			GLfloat dx = static_cast<GLfloat>(col) / static_cast<GLfloat>(edge_nVertsWide - 1);

			tempVert.x = (dx - 0.5f) * edgeWidth * calculateEnvelope(dy, 0.f, 0.1f, 0.9f, 1.f);

			tempVert.y = dy * m_fLength;

			tempVert.z = 0.f;
			//for (auto g : gabors)
			//	tempVert.z += g->get(glm::vec2(tempVert));

			//tempVert.z *= calculateEnvelope(dy, 0.05f, 0.1f, 0.9f, 0.95f);
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

	unsigned int nLFWaves = 2u;
	unsigned int nHFWaves = 0u;

	for (unsigned int i = 0; i < nLFWaves; ++i)
	{
		//float y = length * (i / (nLFWaves - 1.f));
		//float y = length * getRandRatio();
		float y = m_fLength / 2.f;
		Gabor *mainG = new Gabor();
		mainG->setGaussianKernelCenter(glm::vec2(x, y));
		mainG->setGaussianKernelSpread(glm::vec2(m_fWidth / 5.f, m_fLength / nLFWaves));
		mainG->setGaussianKernelAmplitude(m_fEdgeWaveAmplitude * (0.75f + 0.25f * getRandRatio()));
		mainG->setComplexSinusoidDistance((5.f + 5.f * getRandRatio()) / m_fLength);

		gabors.push_back(mainG);
	}

	for (unsigned int i = 0; i < nHFWaves; ++i)
	{
		//float y = length * (i / (nHFWaves - 1.f));
		float y = m_fLength * getRandRatio();
		Gabor *g = new Gabor();
		g->setGaussianKernelCenter(glm::vec2(x, y));
		g->setGaussianKernelSpread(glm::vec2(m_fWidth / 10.f, m_fLength / nHFWaves));
		g->setGaussianKernelAmplitude(m_fEdgeWaveAmplitude * 2.f);
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

void Slatissima::debugDraw()
{
	if (m_pDynamicsWorld->getDebugDrawer() && (m_pDynamicsWorld->getDebugDrawer()->getDebugMode() & (btIDebugDraw::DBG_DrawWireframe | btIDebugDraw::DBG_DrawContactPoints | btIDebugDraw::DBG_DrawNormals | btIDebugDraw::DBG_DrawFrames)))
	{
		// set debug drawer's transform to make sure debug drawings go to right place
		static_cast<BulletDebugDrawer*>(m_pDynamicsWorld->getDebugDrawer())->setTransform(m_pSoftBody->getWorldTransform());

		btSoftBodyHelpers::DrawFrame(m_pSoftBody, m_pDynamicsWorld->getDebugDrawer());
		btSoftBodyHelpers::Draw(m_pSoftBody, m_pDynamicsWorld->getDebugDrawer(), m_debugDrawFlags);

		// SOFT CONTACTS
		if(m_debugDrawFlags & fDrawFlags::Contacts)
		{
			const btVector3	axis[] = { btVector3(1,0,0), btVector3(0,1,0), btVector3(0,0,1) };
			const btScalar nscl = 0.5;
			const btVector3 ccolor = btVector3(1, 0, 0);
			for (int i = 0; i < m_pSoftBody->m_scontacts.size(); ++i)
			{
				const btSoftBody::SContact&	c = m_pSoftBody->m_scontacts[i];
				const btVector3				o = c.m_node->m_x;// -c.m_normal*(btDot(c.m_node->m_x, c.m_normal));
				const btVector3				x = btCross(c.m_normal, axis[c.m_normal.minAxis()]).normalized();
				const btVector3				y = btCross(x, c.m_normal).normalized();
				m_pDynamicsWorld->getDebugDrawer()->drawLine(o - x*nscl, o + x*nscl, ccolor);
				m_pDynamicsWorld->getDebugDrawer()->drawLine(o - y*nscl, o + y*nscl, ccolor);
				m_pDynamicsWorld->getDebugDrawer()->drawLine(o, o + c.m_normal*nscl * 3, btVector3(1, 1, 0));

				m_pDynamicsWorld->getDebugDrawer()->drawLine(c.m_face->m_n[0]->m_x, c.m_face->m_n[1]->m_x, btVector3(0, 1, 1));
				m_pDynamicsWorld->getDebugDrawer()->drawLine(c.m_face->m_n[1]->m_x, c.m_face->m_n[2]->m_x, btVector3(0, 1, 1));
				m_pDynamicsWorld->getDebugDrawer()->drawLine(c.m_face->m_n[2]->m_x, c.m_face->m_n[0]->m_x, btVector3(0, 1, 1));
			}
		}
	}
}

bool fileExists(const std::string &fname)
{
	struct stat buffer;
	return (stat(fname.c_str(), &buffer) == 0);
}

bool Slatissima::saveAsObj(std::string name)
{
	std::string outFileName = std::string("export/" + name + ".obj");

	// if file exists, keep trying until we find a filename that doesn't already exist
	for (int i = 0; fileExists(outFileName); ++i)
		outFileName = std::string("export/" + name + "_" + std::to_string(i) + ".obj");

	std::ofstream outFile;
	outFile.open(outFileName);

	if (!outFile.is_open())
	{
		std::cout << "Error opening file " << outFileName << " for writing output" << std::endl;
		return false;
	}

	std::cout << "Opened file " << outFileName << " for writing output" << std::endl;

	outFile << "#" << outFileName << std::endl;

	outFile << "#vertex data" << std::endl;

	btAlignedObjectArray<btSoftBody::Node> nodes = m_pSoftBody->m_nodes;
	for (size_t i = 0; i < nodes.size(); ++i)
	{
		outFile << "v " << nodes[i].m_x.getX() << " " << nodes[i].m_x.getY() << " " << nodes[i].m_x.getZ() << std::endl;
		outFile << "vn " << nodes[i].m_n.getX() << " " << nodes[i].m_n.getY() << " " << nodes[i].m_n.getZ() << std::endl;
	}

	outFile << "#face data" << std::endl;

	std::vector<int> inds;
	std::vector<glm::vec3> verts;
	mesh->getIndexedVertices(inds, verts);

	for (int i = 0; i < inds.size(); i += 3)
	{
		outFile << "f " << i << "/" << i << "/ " << " " << i + 1 << "/" << i + 1 << "/ " << " " << i + 2 << "/" << i + 2 << "/ " <<  std::endl;
	}

	outFile << "#end " << outFileName << std::endl;

	outFile.close();

	return true;
}
