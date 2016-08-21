#pragma once

#include <vector>
#include <algorithm>

#include <glSkel/mesh.h>

class GeometryStrip
{
public:
	GeometryStrip(std::vector<glm::vec3> v, GLuint nVertsWide, GLuint nVertsTall) 
		: vertices(v)
		, nVertsWide(nVertsWide)
		, nVertsTall(nVertsTall)
	{}

	~GeometryStrip() {}

	std::vector<glm::vec3> getVertices()
	{
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
				GLfloat displacement = this->vertices[j].x - s.vertices[(s.nVertsWide - 1) * s.nVertsTall + j].x;
				s.vertices[b].x += displacement;
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
				GLfloat displacement = this->vertices[(nVertsWide - 1) * nVertsTall + j].x - s.vertices[j].x;
				this->vertices[b].x += displacement;
			}
		}

		// Stitch the two meshes together by replacing the last column of this->vertices with the first column of s->vertices
		std::vector<glm::vec3>::iterator nth = this->vertices.begin() + (this->nVertsWide - 1) * this->nVertsTall;

		this->vertices.insert(nth, std::begin(s.vertices), std::end(s.vertices));
		
		this->nVertsWide += s.nVertsWide; // update new geometry strip dims
	}

private:
	std::vector<glm::vec3> vertices;
	GLuint nVertsWide;
	GLuint nVertsTall;
};