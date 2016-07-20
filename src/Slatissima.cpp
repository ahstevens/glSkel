#include "Slatissima.h"

#include <glSkel\GeometryStrip.h>

#include <algorithm>
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

	glm::vec2 center{ 0.f, length / 2.f };
	glm::vec2 kernelSpread{ 1.f, 10.f };
	GLfloat kernelOrientation{ 0.f }; // degrees
	GLfloat kernelAmplitude{ 1.f };
	glm::vec2 spatialOrientation{ 1.f, 1.f }; // Cartesian coords, not polar
	GLfloat spatialFrequency{ 1.f };

	GaussianKernel k = getGaussianKernel(center, kernelSpread, glm::radians(kernelOrientation), kernelAmplitude);

	// CENTRAL BLADE VERTICES
	for (GLuint i = 0; i < 3; ++i)
	{
		for (GLuint j = 0; j < nVertsTall; ++j)
		{
			GLfloat heightRatio = static_cast<GLfloat>(j) / static_cast<GLfloat>(nVertsTall - 1);
			
			v.x = sin(heightRatio * glm::pi<GLfloat>()) * width;
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

	
	vertices.clear();
	GaussianKernel k2 = getGaussianKernel(center, kernelSpread, glm::radians(kernelOrientation), kernelAmplitude);

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

			v.z = complexSinusoid(glm::vec2(v), glm::vec2(-1.f, 1.f), spatialFrequency).real() * gaussian(glm::vec2(v), k2);

			tempVert.Position = v;
			tempVert.TexCoords = t;
			vertices.push_back(tempVert);
		}
	}

	GeometryStrip g2(vertices, nVertsWide, nVertsTall);

	g.glueOnLeftOf(&g2);

	mesh = g.createMesh();
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

void Slatissima::Draw(Shader s)
{
	mesh->Draw(s);
}