#include "Slatissima.h"

#include <glSkel/GeometryStrip.h>

#include <algorithm>
#include <cmath>

const GLuint resolution = 100;

Slatissima::Slatissima(GLfloat length, GLfloat width, GLfloat thickness, GLfloat spinePadding, GLfloat wavinessMulti)
{
	this->length = length;
	this->width = width;
	this->thickness = thickness;
	this->spinePadding = spinePadding;
	this->wavinessMulti = wavinessMulti;
	this->nVertsTall = this->nVertsWide = (resolution % 2 == 0) ? resolution + 1 : resolution;
	this->buildStrip();
}


Slatissima::~Slatissima()
{
	if (mesh)
		delete(mesh);
}

void Slatissima::buildStrip()
{
	glm::vec3 v;
	glm::vec2 t;

	Vertex tempVert;

	// CENTRAL BLADE VERTICES
	for (GLuint i = 0; i < 3; ++i)
	{
		for (GLuint j = 0; j < nVertsTall; ++j)
		{
			GLfloat heightRatio = static_cast<GLfloat>(j) / static_cast<GLfloat>(nVertsTall - 1);
			
			GLfloat displacement = 0.f;
			if(i == 0) displacement = -width / 2;
			if(i == 2) displacement = width / 2;
			v.x = sin(heightRatio * glm::pi<GLfloat>()) * displacement * 0.5f;
			v.y = heightRatio * length;

			t.x = static_cast<GLfloat>(i) / 2.f;
			t.y = heightRatio;
			
			v.z = 0.f;

			tempVert.Position = v;
			tempVert.TexCoords = t;
			vertices.push_back(tempVert);
		}
	}

	GeometryStrip g(vertices, 3, nVertsTall);

	// LEFT STRIP
	glm::vec2 center{ -width / 2.f, length / 2.f };
	glm::vec2 kernelSpread{ 0.5f, 2.f };
	GLfloat kernelOrientation{ 0.f }; // degrees
	GLfloat kernelAmplitude{ 0.5f };
	glm::vec2 spatialOrientation{ 0.f, 1.f }; // Cartesian coords, not polar
	GLfloat spatialFrequency{ 1.f };

	GaussianKernel k = getGaussianKernel(center, kernelSpread, glm::radians(kernelOrientation), kernelAmplitude);
	
	vertices.clear();

	for (GLuint i = 0; i < nVertsWide; ++i)
	{
		GLfloat widthRatio = static_cast<GLfloat>(i) / static_cast<GLfloat>(nVertsWide - 1);
		t.x = static_cast<GLfloat>(i) / static_cast<GLfloat>(nVertsWide - 1);
		for (GLuint j = 0; j < nVertsTall; ++j)
		{
			GLfloat heightRatio = static_cast<GLfloat>(j) / static_cast<GLfloat>(nVertsTall - 1);
			v.x = (widthRatio - 0.5f) * width * 0.5f * sin(heightRatio * glm::pi<GLfloat>());
			v.y = heightRatio * length;
			t.y = heightRatio;

			v.z = complexSinusoid(glm::vec2(v), spatialOrientation, spatialFrequency).real() * gaussian(glm::vec2(v), k);

			tempVert.Position = v;
			tempVert.TexCoords = t;
			vertices.push_back(tempVert);
		}
	}

	GeometryStrip g2(vertices, nVertsWide, nVertsTall);

	g.glueLeft(g2);

	// RIGHT STRIP
	// glm::vec2 center2{ -width / 2.f, length / 2.f };
	// glm::vec2 kernelSpread2{ 0.5f, 2.f };
	// GLfloat kernelOrientation2{ 0.f }; // degrees
	// GLfloat kernelAmplitude2{ 0.5f };
	// glm::vec2 spatialOrientation2{ 0.f, 1.f }; // Cartesian coords, not polar
	// GLfloat spatialFrequency2{ 1.f };

	// GaussianKernel k2 = getGaussianKernel(center2, kernelSpread2, glm::radians(kernelOrientation2), kernelAmplitude2);
	
	// vertices.clear();

	// for (GLuint i = 0; i < nVertsWide; ++i)
	// {
	// 	GLfloat widthRatio = static_cast<GLfloat>(i) / static_cast<GLfloat>(nVertsWide - 1);
	// 	t.x = static_cast<GLfloat>(i) / static_cast<GLfloat>(nVertsWide - 1);
	// 	for (GLuint j = 0; j < nVertsTall; ++j)
	// 	{
	// 		GLfloat heightRatio = static_cast<GLfloat>(j) / static_cast<GLfloat>(nVertsTall - 1);
	// 		v.x = (widthRatio - 0.5f) * width * 0.5f * sin(heightRatio * glm::pi<GLfloat>());
	// 		v.y = heightRatio * length;
	// 		t.y = heightRatio;

	// 		v.z = complexSinusoid(glm::vec2(v), spatialOrientation2, spatialFrequency2).real() * gaussian(glm::vec2(v), k2);

	// 		tempVert.Position = v;
	// 		tempVert.TexCoords = t;
	// 		vertices.push_back(tempVert);
	// 	}
	// }

	// GeometryStrip g3(vertices, nVertsWide, nVertsTall);

	// g.glueRight(g3);

	mesh = new Mesh(g.getVertices(), g.getIndices(), this->loadTextures());
}

GaussianKernel Slatissima::getGaussianKernel(glm::vec2 center, glm::vec2 spread, GLfloat angle, GLfloat amplitude)
{
	GLfloat a = 0.5f * (pow(cos(angle), 2) / pow(spread.x, 2)) + 0.5f * (pow(sin(angle), 2) / pow(spread.y, 2));
	GLfloat b = -0.25f * (sin(2 * angle) / pow(spread.x, 2)) + 0.25f * (sin(2 * angle) / pow(spread.y, 2));
	GLfloat c = 0.5f * (pow(sin(angle), 2) / pow(spread.x, 2)) + 0.5f * (pow(cos(angle), 2) / pow(spread.y, 2));
	
	return GaussianKernel{ center, amplitude, a, b, c };
}

GLfloat Slatissima::gaussian(glm::vec2 pos, GaussianKernel k)
{
	glm::vec2 dist = pos - k.center;
	return k.amplitude * exp(-((k.a)*pow(dist.x, 2) - 2*(k.b)*(dist.x)*(dist.y) + (k.c)*pow(dist.y, 2)));
}

std::complex<GLfloat> Slatissima::complexSinusoid(glm::vec2 pos, glm::vec2 spatialFreq, GLfloat theta)
{
	using namespace std::complex_literals;
	return exp(glm::two_pi<GLfloat>() * theta * 1if * glm::dot(spatialFreq, pos));
}

std::vector<Texture> Slatissima::loadTextures()
{
	// Load textures
	Texture diffuseMap, specularMap;
	glGenTextures(1, &diffuseMap.id);
	glGenTextures(1, &specularMap.id);
	int width = 1, height = 1;
	unsigned char image[3];

	// Diffuse map
	diffuseMap.type = "texture_diffuse";
	image[0] = 0x55;
	image[1] = 0xFF;
	image[2] = 0x11;
	glBindTexture(GL_TEXTURE_2D, diffuseMap.id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, &image);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);

	// Specular map
	specularMap.type = "texture_specular";
	image[0] = 0x55;
	image[1] = 0xFF;
	image[2] = 0x11;
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

void Slatissima::Draw(Shader s)
{
	mesh->Draw(s);
}