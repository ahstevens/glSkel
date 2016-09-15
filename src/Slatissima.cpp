#include "Slatissima.h"

#include <glSkel/GeometryStrip.h>

#include <algorithm>
#include <cmath>

const float gridSpacing = 0.05f; // cm, approx

Slatissima::Slatissima(GLfloat length_cm, GLfloat width_cm, GLfloat thickness_cm, std::vector<Gabor*> g, btDiscreteDynamicsWorld* dynamicsWorld)
{
	this->length = length_cm;
	this->width = width_cm; 
	this->thickness = thickness_cm;
	this->gabors = g;
	this->nVertsTall = static_cast<GLuint>(length_cm / gridSpacing);
	this->nVertsWide = static_cast<GLuint>(width_cm / gridSpacing);
	this->buildStrip();
	this->m_pDynamicsWorld = dynamicsWorld;

	//create a dynamic rigidbody

	//btCollisionShape* colShape = new btBoxShape(btVector3(1,1,1));
	btCollisionShape* colShape = new btSphereShape(btScalar(0.));

	/// Create Dynamic Objects
	btTransform startTransform;
	startTransform.setIdentity();

	btScalar	mass(10.f);

	//rigidbody is dynamic if and only if mass is non zero, otherwise static
	bool isDynamic = (mass != 0.f);

	btVector3 localInertia(0, 0, 0);
	if (isDynamic)
		colShape->calculateLocalInertia(mass, localInertia);

	startTransform.setOrigin(btVector3(0, 10, 0));

	//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);
	m_pRigidBody = new btRigidBody(rbInfo);

	this->m_pDynamicsWorld->addRigidBody(m_pRigidBody);
}


Slatissima::~Slatissima()
{
	if (mesh)
		delete(mesh);

	delete m_pRigidBody->getMotionState();
	m_pDynamicsWorld->removeCollisionObject(m_pRigidBody);
	delete m_pRigidBody;
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

void Slatissima::drop(btVector3 pos)
{
	btMotionState* motionState = m_pRigidBody->getMotionState();
	btTransform trans;
	motionState->getWorldTransform(trans);
	trans.setOrigin(btVector3(0.f, 10.f, 0.f));
	motionState->setWorldTransform(trans);
	m_pRigidBody->setMotionState(motionState);
	m_pRigidBody->activate();
}

void Slatissima::bump(btVector3 dir)
{
	m_pRigidBody->activate();
	m_pRigidBody->applyCentralImpulse(dir);
}

void Slatissima::Draw(Shader s)
{
	mesh->Draw(s);
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