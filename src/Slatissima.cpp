#include "Slatissima.h"

#include <algorithm>
#include <complex>
#include <cmath>

const GLuint resolution = 1000;

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

	GaussianKernel k = getGaussianKernel(glm::vec2(0.f, length / 2.f), glm::vec2(1.f / glm::e<GLfloat>(), 1.f / glm::e<GLfloat>()), 0.f, 1.f);

	// VERTICES
	for (GLuint i = 0; i < nVertsWide; ++i)
	{
		GLfloat widthRatio = static_cast<GLfloat>(i) / static_cast<GLfloat>(nVertsWide - 1);
		v.x = (widthRatio - 0.5f) * width;
		t.x = static_cast<GLfloat>(i) / static_cast<GLfloat>(nVertsWide - 1);
		for (GLuint j = 0; j < nVertsTall; ++j)
		{
			GLfloat heightRatio = static_cast<GLfloat>(j) / static_cast<GLfloat>(nVertsTall - 1);
			v.y = heightRatio * length;
			t.y = heightRatio;
			
			v.z = gaussian(glm::vec2(v), k);

			tempVert.Position = v;
			tempVert.Normal = glm::vec3(0.f);
			tempVert.TexCoords = t;
			vertices.push_back(tempVert);
		}
	}

	calculateStripNormals(vertices, nVertsWide, nVertsTall);

	mesh = new Mesh(vertices, getStripIndices(nVertsWide, nVertsTall), this->loadTextures());
}

void Slatissima::calculateStripNormals(std::vector<Vertex> &verts, GLuint nVertsWide, GLuint nVertsTall)
{
	for (GLuint i = 0; i < nVertsWide; ++i)
	{
		for (GLuint j = 0; j < nVertsTall; ++j)
		{
			glm::vec3 n = glm::vec3(0.f);
			GLuint b = i * nVertsTall + j;

			// BELOW, LEFT TRIANGLES 1 and 2
			if (i > 0 && j > 0)
			{
				n += getNormalFromIndices(verts, b - nVertsTall - 1, b, b - 1, b);
				n += getNormalFromIndices(verts, b - nVertsTall, b, b - nVertsTall - 1, b);
			}

			// BELOW, RIGHT TRIANGLE
			if (i < nVertsWide - 1 && j > 0)
			{
				n += getNormalFromIndices(verts, b - 1, b, b + nVertsTall, b);
			}

			// ABOVE, LEFT TRIANGLE
			if (i > 0 && j < nVertsTall - 1)
			{
				n += getNormalFromIndices(verts, b + 1, b, b - nVertsTall, b);  // ABOVE, LEFT
			}

			// ABOVE, RIGHT TRIANGLES
			if (i < nVertsWide - 1 && j < nVertsTall - 1)
			{
				n += getNormalFromIndices(verts, b + nVertsTall + 1, b, b + 1, b);
				n += getNormalFromIndices(verts, b + nVertsTall, b, b + nVertsTall + 1, b);
			}

			verts[b].Normal = glm::normalize(n);
		}
	}
}

std::vector<GLuint> Slatissima::getStripIndices(GLuint nVertsWide, GLuint nVertsTall)
{
	std::vector<GLuint> inds;
	for (GLuint i = 0; i < nVertsWide - 1; ++i)
	{
		for (GLuint j = 0; j < nVertsTall - 1; ++j)
		{
			GLuint b = i * nVertsTall + j;

			inds.push_back(b);
			inds.push_back(b + nVertsTall);
			inds.push_back(b + nVertsTall + 1);

			inds.push_back(b);
			inds.push_back(b + nVertsTall + 1);
			inds.push_back(b + 1);
		}
	}

	return inds;
}

glm::vec3 Slatissima::getNormalFromIndices(std::vector<Vertex> &v, GLuint aInd1, GLuint aInd2, GLuint bInd1, GLuint bInd2)
{
	glm::vec3 a, b;

	a = v[aInd1].Position - v[aInd2].Position;
	if (glm::length(a) == 0.f) return glm::vec3(0.f);

	b = v[bInd1].Position - v[bInd2].Position;
	if (glm::length(b) == 0.f) return glm::vec3(0.f);

	return glm::cross(glm::normalize(a), glm::normalize(b));
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

GaussianKernel Slatissima::getGaussianKernel(glm::vec2 center, glm::vec2 spread, GLfloat angle, GLfloat amplitude)
{
	GLfloat a = 0.5f * (pow(cos(angle), 2) / pow(spread.x, 2)) + 0.5f * (pow(sin(angle), 2) / pow(spread.y, 2));
	GLfloat b = -0.25f * (sin(2 * angle) / pow(spread.x, 2)) + 0.25f * (sin(2 * angle) / pow(spread.y, 2));
	GLfloat c = 0.5f * (pow(sin(angle), 2) / pow(spread.x, 2)) + 0.5f * (pow(cos(angle), 2) / pow(spread.y, 2));

	GaussianKernel k = { center, amplitude, a, b, c };

	return k;
}

GLfloat Slatissima::gaussian(glm::vec2 pos, GaussianKernel k)
{
	glm::vec2 dist = glm::vec2(pos.x - k.center.x, pos.y - k.center.y);
	return k.amplitude * exp(-((k.a)*pow(dist.x, 2) - 2*(k.b)*(dist.x)*(dist.y) + (k.c)*pow(dist.y, 2)));
}

void Slatissima::Draw(Shader s)
{
	mesh->Draw(s);
}