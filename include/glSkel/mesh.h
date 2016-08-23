#pragma once
// Std. Includes
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>

// GL Includes
#include <GL/glew.h> // Contains all the necessery OpenGL includes
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glSkel/shader.h>

struct Texture {
    GLuint id;
    std::string type;
};

class Mesh {
public:
    /*  Mesh Data  */
	std::vector<GLuint> indices;
    GLuint VAO;

	glm::vec3 position;
	GLfloat angle;

    /*  Functions  */
    // Constructor to make a DCEL mesh from a triangle soup
    Mesh(std::vector<glm::vec3> vvec3Vertices, std::vector<GLuint> vuiIndices, std::vector<Texture> vTextures)
    {
        this->indices = vuiIndices;
        this->m_vTextures = vTextures;
		this->m_pBoundaryEdge = NULL;

		this->initializeVertices(vvec3Vertices);

		this->processFacesAndEdges(vuiIndices);

		this->consolidateDuplicateVertices();

		std::cout << "Surface area: " << getSurfaceArea() << " cm^2 (" << m_vpFaces.size() << " faces)" << std::endl;

		std::cout << "Surface perimeter: " << getPerimeter() << " cm (" << getBoundaryEdgeCount() << " boundary edges)" << std::endl;

        // Now that we have all the required data, set the vertex buffers and its attribute pointers.
        this->setupGL();
    }

	unsigned int getVertexCount() { return m_vpVertices.size(); }

	unsigned int getFaceCount()	{ return m_vpFaces.size(); }

	unsigned int getHalfEdgeCount() { return m_vpEdges.size(); }

	unsigned int getBoundaryEdgeCount()
	{
		if (!m_pBoundaryEdge)
			return 0u;

		HE_Edge *e = m_pBoundaryEdge;

		unsigned int count = 0u;
		do
		{
			count++;
			e = e->next;
		} while (e != m_pBoundaryEdge);

		return count;
	}

	float getSurfaceArea()
	{
		float area = 0.f;

		for (int i = 0; i < m_vpFaces.size(); ++i)
			area += glm::length(m_vpFaces[i]->normal);     // face area is magnitude of face normal

		return area;
	}

	float getPerimeter()
	{
		if (!m_pBoundaryEdge)
			return 0.f;

		float p = 0.f;
		HE_Edge *e = m_pBoundaryEdge;
		do
		{
			p += glm::length(e->head->pos - e->opposite->head->pos);
			e = e->next;
		} while (e != m_pBoundaryEdge);

		return p;
	}

	// Render the mesh
	void Draw(Shader shader)
	{
		// Bind appropriate textures
		GLuint diffuseNr = 1;
		GLuint specularNr = 1;
		GLuint normalNr = 1;
		GLuint heightNr = 1;
		for (GLuint i = 0; i < this->m_vTextures.size(); i++)
		{
			glActiveTexture(GL_TEXTURE0 + i); // Active proper texture unit before binding
											  // Retrieve texture number (the N in diffuse_textureN)
			std::stringstream ss;
			std::string number;
			std::string name = this->m_vTextures[i].type;
			if (name == "texture_diffuse")
				ss << diffuseNr++;
			else if (name == "texture_specular")
				ss << specularNr++;
			else if (name == "texture_normal")
				ss << normalNr++;
			else if (name == "texture_height")
				ss << heightNr++;
			number = ss.str();
			// Now set the sampler to the correct texture unit
			glUniform1i(glGetUniformLocation(shader.Program, (name + number).c_str()), i);
			// And finally bind the texture
			glBindTexture(GL_TEXTURE_2D, this->m_vTextures[i].id);
		}

		glm::mat4 model = glm::mat4();
		model = glm::translate(model, position);
		model = glm::rotate(model, angle, glm::vec3(1.0f, 0.3f, 0.5f));

		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));

		// Draw mesh
		glBindVertexArray(this->VAO);
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(this->indices.size()), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		// Always good practice to set everything back to defaults once configured.
		for (GLuint i = 0; i < this->m_vTextures.size(); i++)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

private:

	struct HE_Vertex;
	struct HE_Face;
	struct HE_Edge;

	struct HE_Vertex {
		glm::vec3 pos;
		HE_Edge *halfedge;

		HE_Vertex()
			: pos(glm::vec3(0.f))
			, halfedge(NULL)
		{}
	};

	struct HE_Face {
		HE_Edge *edge;
		glm::vec3 normal;

		HE_Face()
			: edge(NULL)
			, normal(glm::vec3(0.f))
		{}
	};

	struct HE_Edge {
		HE_Edge *next;
		HE_Edge *opposite;
		HE_Face *face;
		HE_Vertex *head;

		HE_Edge()
			: next(NULL)
			, opposite(NULL)
			, face(NULL)
			, head(NULL)
		{}
	};

	struct Vertex {
		glm::vec3 pos;
		glm::vec3 norm;
		glm::vec2 tex;
	};

    /*  Render data  */
    GLuint VBO, EBO;

	std::vector<HE_Edge*> m_vpEdges;
	std::vector<HE_Face*> m_vpFaces;
	std::vector<HE_Vertex*> m_vpVertices;
	std::vector<Texture> m_vTextures;
	HE_Edge *m_pBoundaryEdge;

    /*  Functions    */
	void initializeVertices(std::vector<glm::vec3> vertices)
	{
		for (int i = 0; i < vertices.size(); ++i)
		{
			HE_Vertex *v = new HE_Vertex();
			v->pos = vertices[i];
			this->m_vpVertices.push_back(v);
		}
	}

	void processFacesAndEdges(std::vector<GLuint> vuiIndices)
	{
		// make an edge map to hold the index pairs of the vertices that make up
		// the start and end of the half edge, respectively
		typedef std::pair<int, int> EdgeMapIndexT;
		typedef std::map< EdgeMapIndexT, HE_Edge* > EdgeMapT;
		EdgeMapT edgeMap;

		for (int i = 0; i < indices.size(); i += 3)
		{
			HE_Face *f = new HE_Face();

			// Create the three halfedges of the face in CCW order
			HE_Edge *e1, *e2, *e3;
			e1 = new HE_Edge();
			e2 = new HE_Edge();
			e3 = new HE_Edge();

			e1->face = f;
			e1->next = e2;
			e1->head = m_vpVertices[indices[i + 1]];

			e2->face = f;
			e2->next = e3;
			e2->head = m_vpVertices[indices[i + 2]];

			e3->face = f;
			e3->next = e1;
			e3->head = m_vpVertices[indices[i]];

			if (!m_vpVertices[indices[i]]->halfedge) m_vpVertices[indices[i]]->halfedge = e1;
			if (!m_vpVertices[indices[i + 1]]->halfedge) m_vpVertices[indices[i + 1]]->halfedge = e2;
			if (!m_vpVertices[indices[i + 2]]->halfedge) m_vpVertices[indices[i + 2]]->halfedge = e3;

			f->edge = e1; // doesn't matter which edge it points to
			f->normal = glm::cross(e1->head->pos - e3->head->pos, e2->head->pos - e3->head->pos);
			m_vpFaces.push_back(f);
			m_vpEdges.push_back(e1);
			m_vpEdges.push_back(e2);
			m_vpEdges.push_back(e3);

			// enter these 
			edgeMap[EdgeMapIndexT(indices[i], indices[i + 1])] = e1;
			edgeMap[EdgeMapIndexT(indices[i + 1], indices[i + 2])] = e2;
			edgeMap[EdgeMapIndexT(indices[i + 2], indices[i])] = e3;
		}

		// link up paired half edges
		for (EdgeMapT::iterator it = edgeMap.begin(); it != edgeMap.end(); it++)
		{
			if (it->second->opposite != NULL) continue;

			// look for an edge going the opposite way in the edge map
			EdgeMapT::iterator opp = edgeMap.find(std::pair<int, int>(it->first.second, it->first.first));
			if (opp != edgeMap.end()) // found a match, so link 'em up
			{
				it->second->opposite = opp->second;
				opp->second->opposite = it->second;
			}
			else // no match, therefore it's a boundary edge
			{
				HE_Edge *newBoundaryEdge = new HE_Edge();
				newBoundaryEdge->head = it->second->next->next->head; // a boundary edge always points to boundary vertex 
				newBoundaryEdge->face = NULL; // NULL face indicates a boundary edge

				// link new edge to next, if it exists
				if (!newBoundaryEdge->head->halfedge->face)
					newBoundaryEdge->next = newBoundaryEdge->head->halfedge;

				// link opposite edges
				newBoundaryEdge->opposite = it->second;
				it->second->opposite = newBoundaryEdge;

				// change emanating vertex to point to new boundary edge.
				// edges of boundary are oriented in CW order
				it->second->head->halfedge = newBoundaryEdge;

				m_vpEdges.push_back(newBoundaryEdge);
				
				// set the mesh boundary edge pointer the first time.
				// NULL boundary edge pointer afterwards means it's a closed mesh
				if (!m_pBoundaryEdge) m_pBoundaryEdge = newBoundaryEdge;
			}
		}

		// one more pass to fill in any unlinked boundary edges
		for (int i = 0; i < m_vpEdges.size(); ++i)
			if (!m_vpEdges[i]->next)
				m_vpEdges[i]->next = m_vpEdges[i]->head->halfedge;
	}

	void consolidateDuplicateVertices(float threshold = 0.0000001) // 1 nm
	{
		float threshold_sq = threshold * threshold;

		for (std::vector<HE_Vertex*>::iterator it = m_vpVertices.begin(); it != m_vpVertices.end(); it++)
		{
			HE_Edge *begin = (*it)->halfedge;
			HE_Edge *e = begin;
			do
			{
				HE_Vertex *v = e->head;

				if (1)
				{
					//consolidate verts
				}

				e = e->opposite->next;
			} while (e != begin);
		}
	}

	std::vector<Vertex> makeBufferVertices()
	{
		std::vector<Vertex> ret;
		for (std::vector<HE_Vertex*>::iterator it = m_vpVertices.begin(); it != m_vpVertices.end(); it++)
		{
			Vertex v;
			v.pos = (*it)->pos;
			v.norm = glm::vec3(0.f);
			v.tex = glm::vec2(0.5f);

			// calc vertex normals as sum of (unnormalized) incident face normals
			HE_Edge *beginEdge = (*it)->halfedge;
			HE_Edge *e = beginEdge;
			do
			{
				if (e->face)
					v.norm += e->face->normal;
				e = e->opposite->next;
			} while (e != beginEdge);

			v.norm = glm::normalize(v.norm);

			ret.push_back(v);
		}

		return ret;
	}

	// Initializes all the buffer objects/arrays
	void setupGL()
	{
		std::vector<Vertex> bufferVertices = makeBufferVertices();

		// Create buffers/arrays
		glGenVertexArrays(1, &this->VAO);
		glGenBuffers(1, &this->VBO);
		glGenBuffers(1, &this->EBO);

		glBindVertexArray(this->VAO);
		// Load data into vertex buffers
		glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
		// A great thing about structs is that their memory layout is sequential for all its items.
		// The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
		// again translates to 3/2 floats which translates to a byte array.
		glBufferData(GL_ARRAY_BUFFER, bufferVertices.size() * sizeof(Vertex), &bufferVertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint), &this->indices[0], GL_STATIC_DRAW);

		// Set the vertex attribute pointers
		// Vertex Positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
		// Vertex Normals
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, norm));
		// Vertex Texture Coords
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, tex));

		glBindVertexArray(0);
	}
};



