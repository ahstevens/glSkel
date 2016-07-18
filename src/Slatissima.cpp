#include "Slatissima.h"

#include <algorithm>
#include <complex>


Slatissima::Slatissima(GLfloat length, GLfloat width, GLfloat thickness, GLfloat spinePadding, GLfloat wavinessMulti, GLuint nSegments)
{
	this->length = length;
	this->width = width;
	this->thickness = thickness;
	this->spinePadding = spinePadding;
	this->wavinessMulti = wavinessMulti;
	this->nSpineVerts = nSegments + 1;
	this->buildModel();
}


Slatissima::~Slatissima()
{
	if (mesh)
		delete(mesh);
}

void Slatissima::Draw(Shader s)
{
	mesh->Draw(s);
}

void Slatissima::buildModel()
{
	Vertex tempVert;
	GLuint counter = 0;

	tempVert.Normal = glm::vec3(0.f, 0.f, 1.f);

	for (GLuint i = 0; i < nSpineVerts; ++i)
	{
		GLfloat lengthRatio = ((float)i / (float)(nSpineVerts - 1));
		GLfloat x_offset = glm::sin(lengthRatio * 3.14159f) + spinePadding;
		GLfloat y_coord = lengthRatio * length;
		GLfloat z_offset = glm::sin(lengthRatio * 3.14159f * length * wavinessMulti) * (thickness / 2.f);

		z_offset *= glm::cosh(x_offset) * sin(y_coord);

		std::complex<GLfloat> inp(x_offset, lengthRatio * 3.14159f * 10.f);
		z_offset = std::sinh(inp).real() * (thickness / 2.f);

		// Center point first
		tempVert.TexCoords = glm::vec2(0.5f, lengthRatio);
		tempVert.Position = glm::vec3(0.f, y_coord, 0.f);

		vertices.push_back(tempVert);

		// Left side
		tempVert.TexCoords = glm::vec2(0.5f - x_offset / 2.f, lengthRatio);
		tempVert.Position = glm::vec3( -(x_offset * width / 2.f), y_coord, z_offset);

		vertices.push_back(tempVert);

		// Right side

		tempVert.TexCoords = glm::vec2(0.5f + x_offset / 2.f, lengthRatio);
		tempVert.Position = glm::vec3(x_offset / 2.f * width, y_coord, z_offset);

		vertices.push_back(tempVert);

		if (i == nSpineVerts - 1) break;

		// Indices
		indices.push_back(counter + 0);
		indices.push_back(counter + 3);
		indices.push_back(counter + 1);
		
		indices.push_back(counter + 1);
		indices.push_back(counter + 3);
		indices.push_back(counter + 4);
		
		indices.push_back(counter + 0);
		indices.push_back(counter + 2);
		indices.push_back(counter + 3);

		indices.push_back(counter + 2);
		indices.push_back(counter + 5);
		indices.push_back(counter + 3);

		counter += 3;
	}


	calcSpineNormals();
	calcCenterBladeEdgeNormals();

	mesh = new Mesh(vertices, indices, this->loadTextures());
}

void Slatissima::calcSpineNormals()
{
	glm::vec3 normal;

	for (GLuint i = 0; i < vertices.size(); i += 3)
	{
		normal = glm::vec3(0.f);

		if (i != vertices.size() - 3)
		{
			normal += getNormalFromIndices(i + 3, i, i + 1, i);
			normal += getNormalFromIndices(i + 2, i, i + 3, i);
		}
		else if (i != 0)
		{
			normal += getNormalFromIndices(i - 2, i, i - 3, i);
			normal += getNormalFromIndices(i + 1, i, i - 2, i);
			normal += getNormalFromIndices(i - 1, i, i + 2, i);
			normal += getNormalFromIndices(i - 3, i, i - 1, i);
		}

		vertices[i].Normal = glm::normalize(normal);
	}
}

void Slatissima::calcCenterBladeEdgeNormals()
{
	glm::vec3 normal;

	// Left-side verts first
	for (GLuint i = 1; i < vertices.size(); i += 3)
	{
		normal = glm::vec3(0.f);

		if (i != vertices.size() - 2)
		{
			normal += getNormalFromIndices(i - 1, i, i + 2, i);
			normal += getNormalFromIndices(i + 2, i, i + 3, i);
		}
		else if (i != 1)
		{
			normal += getNormalFromIndices(i - 3, i, i - 1, i);
		}
		
		vertices[i].Normal = glm::normalize(normal);
	}

	// Right-side verts
	for (GLuint i = 2; i < vertices.size(); i += 3)
	{
		normal = glm::vec3(0.f);

		if (i != vertices.size() - 1)
		{			
			normal += getNormalFromIndices(i + 1, i, i - 2, i);
			normal += getNormalFromIndices(i + 3, i, i + 1, i);
		}
		else if (i != 2)
		{
			normal += getNormalFromIndices(i - 2, i, i - 3, i);
		}

		vertices[i].Normal = glm::normalize(normal);
	}
}

glm::vec3 Slatissima::getNormalFromIndices(Vertex &v, GLuint aInd1, GLuint aInd2, GLuint bInd1, GLuint bInd2)
{
	glm::vec3 a, b;

	a = (&v)[aInd1].Position - (&v)[aInd2].Position;
	if (glm::length(a) == 0.f) return glm::vec3(0.f);

	b = (&v)[bInd1].Position - (&v)[bInd2].Position;
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

void Slatissima::buildStrip(GLuint widthGranularity = 1)
{
	std::vector<Vertex> verts;
	std::vector<GLuint> inds;

	GLuint k = nSpineVerts;

	glm::vec3 v, n, a, b;
	glm::vec2 t;

	Vertex tempVert;

	// VERTICES
	for (GLuint i = 0; i < k; ++i)
	{
		v.x = (static_cast<GLfloat>(i) / static_cast<GLfloat>(k - 1) - 0.5f) * 3.14159 * width;
		t.x = static_cast<GLfloat>(i) / static_cast<GLfloat>(k - 1);
		for (GLuint j = 0; j < 2 * widthGranularity + 1; ++j)
		{
			v.y = (static_cast<GLfloat>(j) / static_cast<GLfloat>(2 * widthGranularity)) * 3.14159 * length;
			t.y = static_cast<GLfloat>(j) / static_cast<GLfloat>(2 * widthGranularity);

			std::complex<GLfloat> inp(v.x, v.y);
			v.z = (std::sinh(inp).real() / 2.f) * 10.f * thickness;

			tempVert.Position = v;
			tempVert.Normal = glm::vec3(0.f);
			tempVert.TexCoords = t;
			verts.push_back(tempVert);
		}
	}

	//NORMALS
	for (GLuint i = 0; i < k; ++i)
	{
		n = glm::vec3(0.f);
		for (GLuint j = 0; j < 2 * widthGranularity + 1; ++j)
		{
				n += getNormalFromIndices(verts, (i*j) - 1, i, i + k, i);  //
				n += getNormalFromIndices(verts, i - 1, i, i + k, i);  // BELOW, RIGHT
		}
	}

	// INDICES
	for (GLuint i = 0; i < k - 1; ++i)
	{
		for (GLuint j = 0; j < 2 * widthGranularity; ++j)
		{
			inds.push_back(i);
			inds.push_back(i + k*(j + 1));
			inds.push_back(i + k*(j + 1) + 1);
			
			inds.push_back(i);
			inds.push_back(i + k*(j + 1) + 1);
			inds.push_back(i + 1);			
		}
	}
}
