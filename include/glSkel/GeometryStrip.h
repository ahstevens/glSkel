#pragma once

#include <vector>
#include <algorithm>

#include <glSkel/mesh.h>

class GeometryStrip
{
public:
	GeometryStrip(std::vector<Vertex> v, GLuint nVertsWide, GLuint nVertsTall) : vertices(v), nVertsWide(nVertsWide), nVertsTall(nVertsTall) {}
	~GeometryStrip() {}

	Mesh* createMesh()
	{
		if (m)
			delete(m);

		this->calculateNormals();
		m = new Mesh(this->vertices, this->getStripIndices(), this->loadTextures());
		return m;
	}

	void glueOnLeftOf(GeometryStrip* s)
	{
		assert(this->nVertsTall == s->nVertsTall);

		

		for (GLuint i = 0; i < this->nVertsWide; ++i)
		{
			for (GLuint j = 0; j < this->nVertsTall; ++j)
			{
				GLuint b = i * nVertsTall + j;
				GLfloat widthHere = s->vertices[nVertsTall * (nVertsWide - 1) + j].Position.x - s->vertices[j].Position.x;
				this->vertices[b].Position.x -= widthHere;
			}
		}

		// Stitch the two meshes together by replacing the last column of this->vertices with the first column of s->vertices
		std::vector<Vertex>::iterator nth = this->vertices.begin() + (this->nVertsWide - 1) * this->nVertsTall;

		this->vertices.insert(nth, std::begin(s->vertices), std::end(s->vertices));
		
		this->nVertsWide += s->nVertsWide - 1; // update new geometry strip dims
	}

private:
	std::vector<Vertex> vertices;
	GLuint nVertsWide;
	GLuint nVertsTall;
	Mesh* m;

	std::vector<GLuint> getStripIndices()
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

	void calculateNormals()
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
					n += getNormalFromIndices(vertices, b - nVertsTall - 1, b, b - 1, b);
					n += getNormalFromIndices(vertices, b - nVertsTall, b, b - nVertsTall - 1, b);
				}

				// BELOW, RIGHT TRIANGLE
				if (i < nVertsWide - 1 && j > 0)
				{
					n += getNormalFromIndices(vertices, b - 1, b, b + nVertsTall, b);
				}

				// ABOVE, LEFT TRIANGLE
				if (i > 0 && j < nVertsTall - 1)
				{
					n += getNormalFromIndices(vertices, b + 1, b, b - nVertsTall, b);  // ABOVE, LEFT
				}

				// ABOVE, RIGHT TRIANGLES
				if (i < nVertsWide - 1 && j < nVertsTall - 1)
				{
					n += getNormalFromIndices(vertices, b + nVertsTall + 1, b, b + 1, b);
					n += getNormalFromIndices(vertices, b + nVertsTall, b, b + nVertsTall + 1, b);
				}

				vertices[b].Normal = glm::normalize(n);
			}
		}
	}
	
	glm::vec3 getNormalFromIndices(std::vector<Vertex> &v, GLuint aInd1, GLuint aInd2, GLuint bInd1, GLuint bInd2)
	{
		glm::vec3 a, b;

		a = v[aInd1].Position - v[aInd2].Position;
		if (glm::length(a) == 0.f) return glm::vec3(0.f);

		b = v[bInd1].Position - v[bInd2].Position;
		if (glm::length(b) == 0.f) return glm::vec3(0.f);

		return glm::cross(glm::normalize(a), glm::normalize(b));
	}

	std::vector<Texture> loadTextures()
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

};