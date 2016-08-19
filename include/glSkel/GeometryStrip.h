#pragma once

#include <vector>
#include <algorithm>

#include <glSkel/mesh.h>

class GeometryStrip
{
public:
	GeometryStrip(std::vector<Vertex> v, GLuint nVertsWide, GLuint nVertsTall) 
		: vertices(v)
		, nVertsWide(nVertsWide)
		, nVertsTall(nVertsTall)
	{}

	~GeometryStrip() {}

	std::vector<Vertex> getVertices()
	{
		this->calculateNormals();
		return this->vertices;
	}

	std::vector<GLuint> getIndices()
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

	void glueLeft(GeometryStrip &s)
	{
		assert(this->nVertsTall == s.nVertsTall);

		for (GLuint i  = 0; i < s.nVertsWide; ++i)
		{
			for (GLuint j = 0; j < s.nVertsTall; ++j)
			{
				GLuint b = i * s.nVertsTall + j;
				GLfloat displacement = this->vertices[j].Position.x - s.vertices[(s.nVertsWide - 1) * s.nVertsTall + j].Position.x;
				s.vertices[b].Position.x += displacement;
			}
		}

		// Stitch the two meshes together
		this->vertices.insert(std::begin(this->vertices), std::begin(s.vertices), std::end(s.vertices));
		
		this->nVertsWide += s.nVertsWide; // update new geometry strip dims
	}

	void glueRight(const GeometryStrip &s)
	{
		assert(this->nVertsTall == s.nVertsTall);

		for (GLuint i = 0; i < s.nVertsWide; ++i)
		{
			for (GLuint j = 0; j < s.nVertsTall; ++j)
			{
				GLuint b = i * s.nVertsTall + j;
				GLfloat displacement = this->vertices[(nVertsWide - 1) * nVertsTall + j].Position.x - s.vertices[j].Position.x;
				this->vertices[b].Position.x += displacement;
			}
		}

		// Stitch the two meshes together by replacing the last column of this->vertices with the first column of s->vertices
		std::vector<Vertex>::iterator nth = this->vertices.begin() + (this->nVertsWide - 1) * this->nVertsTall;

		this->vertices.insert(nth, std::begin(s.vertices), std::end(s.vertices));
		
		this->nVertsWide += s.nVertsWide; // update new geometry strip dims
	}

private:
	std::vector<Vertex> vertices;
	GLuint nVertsWide;
	GLuint nVertsTall;

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
		glm::vec3 a = v[aInd1].Position - v[aInd2].Position; 
		glm::vec3 b = v[bInd1].Position - v[bInd2].Position;

		return glm::cross(a, b);
	}
};