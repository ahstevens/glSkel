#include "Slatissima.h"

#include <glSkel/GeometryStrip.h>

#include <algorithm>
#include <cmath>

const float gridSpacing = 0.05f; // cm, approx

Slatissima::Slatissima(GLfloat length_cm, GLfloat width_cm, GLfloat thickness_cm, GLfloat spinePadding_cm, GLfloat wavinessMulti)
{
	this->length = length_cm;
	this->width = width_cm; 
	this->thickness = thickness_cm;
	this->spinePadding = spinePadding_cm;
	this->wavinessMulti = wavinessMulti;
	this->nVertsTall = static_cast<GLuint>(length_cm / gridSpacing);
	this->nVertsWide = static_cast<GLuint>(width_cm / gridSpacing);
	this->buildStrip();
}


Slatissima::~Slatissima()
{
	if (mesh)
		delete(mesh);
}

void Slatissima::rotateX(float degrees)
{
	this->mesh->addRotation(glm::quat(glm::vec3(glm::radians(degrees), 0.f, 0.f)));
}

void Slatissima::rotateY(float degrees)
{
	this->mesh->addRotation(glm::quat(glm::vec3(0.f, glm::radians(degrees), 0.f)));
}

void Slatissima::rotateZ(float degrees)
{
	this->mesh->addRotation(glm::quat(glm::vec3(0.f, 0.f, glm::radians(degrees))));
}

void Slatissima::resetOrientation() { this->mesh->setRotation(glm::quat()); }

void Slatissima::Draw(Shader s)
{
	mesh->Draw(s);
}

void Slatissima::buildStrip()
{
	std::vector<std::vector<glm::vec3>> vertices; // row major
	glm::vec3 tempVert;

	// CENTRAL BLADE VERTICES
	for (GLuint row = 0; row < nVertsTall; ++row)
	{
		GLfloat heightRatio = static_cast<GLfloat>(row) / static_cast<GLfloat>(nVertsTall - 1);

		std::vector<glm::vec3> vecRow;

		for (GLuint col = 0; col < nVertsWide; ++col)
		{
			GLfloat widthRatio = static_cast<GLfloat>(col) / static_cast<GLfloat>(nVertsWide - 1);

			GLfloat displacement = -(width / 2.f) + widthRatio * width;
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

	// LEFT STRIP
	glm::vec2 center{ 0.f, 0.f };
	glm::vec2 kernelSpread{ width / 2.f, length / 2.f };
	GLfloat kernelOrientation{ 0.f }; // degrees
	GLfloat kernelAmplitude{ 1.f };
	glm::vec2 spatialOrientation{ 0.f, 0.5f }; // Cartesian coords, not polar
	GLfloat spatialFrequency{ 1.f };

	Gabor gabor;
	gabor.setGaussianKernel(center, kernelSpread, glm::radians(kernelOrientation), kernelAmplitude);
	gabor.setComplexSinusoid(spatialOrientation, spatialFrequency);
	
	vertices.clear();
	for (GLuint row = 0; row < nVertsTall; ++row)
	{
		std::vector<glm::vec3> vecRow;
		GLfloat heightRatio = static_cast<GLfloat>(row) / static_cast<GLfloat>(nVertsTall - 1);

		for (GLuint col = 0; col < nVertsWide; ++col)
		{
			GLfloat widthRatio = static_cast<GLfloat>(col) / static_cast<GLfloat>(nVertsWide - 1);

			tempVert.x = (widthRatio - 0.5f) * width * 0.5f * sin(heightRatio * glm::pi<GLfloat>());
			//tempVert.x = (widthRatio - 0.5f) * width * 0.5f;
			tempVert.y = heightRatio * length;

			tempVert.z = gabor.get(glm::vec2(tempVert));
			//v.z = 0.f;

			vecRow.push_back(tempVert);
		}

		vertices.push_back(vecRow);
	}

	GeometryStrip g2(vertices);

	g.glueLeft(g2);

	// RIGHT STRIP
	glm::vec2 center2{ -length / 2.f, 0.f };
	glm::vec2 kernelSpread2{ 1.f, sqrtf(length)};
	GLfloat kernelOrientation2{ 0.f }; // degrees
	GLfloat kernelAmplitude2{ 0.5f };
	glm::vec2 spatialOrientation2{ 1.f, 0.f }; // Cartesian coords, not polar
	GLfloat spatialFrequency2{ 0.1f };
	
	Gabor gabor2;
	gabor2.setGaussianKernel(center2, kernelSpread2, glm::radians(kernelOrientation2), kernelAmplitude2);
	gabor2.setComplexSinusoid(spatialOrientation2, spatialFrequency2);
	
	vertices.clear();

	for (GLuint row = 0; row < nVertsTall; ++row)
	{
		std::vector<glm::vec3> vecRow;
		GLfloat heightRatio = static_cast<GLfloat>(row) / static_cast<GLfloat>(nVertsTall - 1);

		for (GLuint col = 0; col < nVertsTall; ++col)
		{
			GLfloat widthRatio = static_cast<GLfloat>(col) / static_cast<GLfloat>(nVertsTall - 1);

			//tempVert.x = (widthRatio + 0.5f) * width * 0.5f * sin(heightRatio * glm::pi<GLfloat>());
			tempVert.x = -(length / 2.f) + widthRatio * length;
			tempVert.y = -(length / 2.f) + heightRatio * length;

			tempVert.z = gabor.get(glm::vec2(tempVert));// +gabor2.get(glm::vec2(tempVert));

			vecRow.push_back(tempVert);
		}

		vertices.push_back(vecRow);
	}

	GeometryStrip g3(vertices);

	//g.glueRight(g3);

	std::cout << "Creating DCEL mesh from geometry strip that is " << g3.getWidthVertexCount() << " verts wide and " << g3.getHeightVertexCount() << " verts long" << std::endl;
	mesh = new Mesh(g3.getVertices(), g3.getIndices(), this->loadTextures());
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