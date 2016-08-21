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

struct HE_Vertex;
struct HE_Face;
struct HE_Edge;

struct HE_Vertex {
	int id;
	glm::vec3 pos;
	HE_Edge *halfedge;

	HE_Vertex()
		: id(-1)
		, pos(glm::vec3(0.f))
		, halfedge(NULL)
	{}
};

struct HE_Face {
	int id;
    HE_Edge *edge;
	glm::vec3 n;

	HE_Face()
		: id(-1)
        , edge(NULL)
		, n(glm::vec3(0.f))
	{}
};

struct HE_Edge {
	int id;
	HE_Edge *next;
	HE_Edge *opposite;
	HE_Face *face;
	HE_Vertex *head;

	HE_Edge()
		: id(-1)
		, next(NULL)
		, opposite(NULL)
		, face(NULL)
		, head(NULL)
	{}
};


struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct Texture {
    GLuint id;
    std::string type;
};

class Mesh {
public:
    /*  Mesh Data  */
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	std::vector<Texture> textures;
    GLuint VAO;

	std::vector<HE_Edge*> edges;
	std::vector<HE_Face*> faces;
	std::vector<HE_Vertex*> verts;
	HE_Edge *boundaryEdge;

	glm::vec3 position;
	GLfloat angle;

    /*  Functions  */
    // Constructor
    Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<Texture> textures)
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;
		this->boundaryEdge = NULL;

        for (int i = 0; i < vertices.size(); ++i)
        {
            HE_Vertex *v = new HE_Vertex();
            v->id = i;
            v->pos = vertices[i].Position;
            verts.push_back(v);
        }

		typedef std::map< std::pair<int, int>, HE_Edge* > EdgeMapT;

        EdgeMapT edgeMap;

        int faceCount = 0;
        int edgeCount = 0;
        for (int i = 0; i < indices.size(); i += 3)
        {
            HE_Face *f = new HE_Face();
            f->id = faceCount++;

            HE_Edge *e1, *e2, *e3;
			e1 = new HE_Edge();
			e2 = new HE_Edge();
			e3 = new HE_Edge();

            e1->id = edgeCount++;
            e2->id = edgeCount++;
            e3->id = edgeCount++;

            f->edge = e1;

            e1->face = f;
            e1->next = e2;
            e1->head = verts[indices[i + 1]];

            e2->face = f;
            e2->next = e3;
            e2->head = verts[indices[i + 2]];

            e3->face = f;
            e3->next = e1;
            e3->head = verts[indices[i]];

            if (!verts[indices[i]]->halfedge) verts[indices[i]]->halfedge = e1;
            if (!verts[indices[i+1]]->halfedge) verts[indices[i+1]]->halfedge = e2;
            if (!verts[indices[i+2]]->halfedge) verts[indices[i+2]]->halfedge = e3;

            faces.push_back(f);
            edges.push_back(e1);
            edges.push_back(e2);
            edges.push_back(e3);

            edgeMap[std::pair<int, int>(indices[i], indices[i + 1])] = e1;
            edgeMap[std::pair<int, int>(indices[i + 1], indices[i + 2])] = e2;
            edgeMap[std::pair<int, int>(indices[i + 2], indices[i])] = e3;
        }

        EdgeMapT::iterator it;
        for (it = edgeMap.begin(); it != edgeMap.end(); it++)
        {
            if (it->second->opposite != NULL) continue;

			EdgeMapT::iterator opp = edgeMap.find(std::pair<int, int>(it->first.second, it->first.first));
            if(opp != edgeMap.end())
            {
                it->second->opposite = opp->second;
                opp->second->opposite = it->second;
            }
			else // boundary edge
			{
				HE_Edge *newBoundaryEdge = new HE_Edge();
				newBoundaryEdge->id = edgeCount++;
				newBoundaryEdge->head = it->second->next->next->head;
				newBoundaryEdge->face = NULL;
				
				// link new edge to next, if it exists
				if (!newBoundaryEdge->head->halfedge->face)
					newBoundaryEdge->next = newBoundaryEdge->head->halfedge;

				// link opposite edges
				newBoundaryEdge->opposite = it->second;
				it->second->opposite = newBoundaryEdge;

				// change emanating vertex to point to new boundary edge
				it->second->head->halfedge = newBoundaryEdge;

				edges.push_back(newBoundaryEdge);

				if (!boundaryEdge) boundaryEdge = newBoundaryEdge;
			}
        }

		for (int i = 0; i < edges.size(); ++i)
			if (!edges[i]->next)
				edges[i]->next = edges[i]->head->halfedge;

		float area = 0.f;
        for (int i = 0; i < faces.size(); ++i)
        {
            glm::vec3 v1 = faces[i]->edge->head->pos;
            glm::vec3 v2 = faces[i]->edge->next->head->pos;
            glm::vec3 v3 = faces[i]->edge->next->next->head->pos;
            glm::vec3 a = v3 - v2; 
		    glm::vec3 b = v1 - v2;

		    faces[i]->n = glm::cross(a, b);
			area += glm::length(faces[i]->n);
        }

		std::cout << "Surface area: " << area << " cm^2 (" << faces.size() << " faces)" << std::endl;

		float perimeter = 0.f;
		int beCount = 0;
		HE_Edge *e = boundaryEdge;
		do
		{
			perimeter += glm::length(e->head->pos - e->opposite->head->pos);
			e = e->next;
			beCount++;
		} while (e != boundaryEdge);

		std::cout << "Surface perimeter: " << perimeter << " cm (" << beCount << " boundary edges)" << std::endl;


        // Now that we have all the required data, set the vertex buffers and its attribute pointers.
        this->setupMesh();
    }

    // Render the mesh
    void Draw(Shader shader) 
    {
        // Bind appropriate textures
        GLuint diffuseNr = 1;
        GLuint specularNr = 1;
        GLuint normalNr = 1;
        GLuint heightNr = 1;
        for(GLuint i = 0; i < this->textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i); // Active proper texture unit before binding
            // Retrieve texture number (the N in diffuse_textureN)
			std::stringstream ss;
			std::string number;
			std::string name = this->textures[i].type;
            if(name == "texture_diffuse")
                ss << diffuseNr++;
            else if(name == "texture_specular")
                ss << specularNr++;
            else if(name == "texture_normal")
                ss << normalNr++;
             else if(name == "texture_height")
                ss << heightNr++;
            number = ss.str(); 
            // Now set the sampler to the correct texture unit
            glUniform1i(glGetUniformLocation(shader.Program, (name + number).c_str()), i);
            // And finally bind the texture
            glBindTexture(GL_TEXTURE_2D, this->textures[i].id);
        }

		glm::mat4 model = glm::mat4();
		model = glm::translate(model, position);
		model = glm::rotate(model, angle, glm::vec3(1.0f, 0.3f, 0.5f));

		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        
        // Draw mesh
        glBindVertexArray(this->VAO);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>( this->indices.size() ), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // Always good practice to set everything back to defaults once configured.
        for (GLuint i = 0; i < this->textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }

private:
    /*  Render data  */
    GLuint VBO, EBO;

    /*  Functions    */
    // Initializes all the buffer objects/arrays
    void setupMesh()
    {
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
        glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex), &this->vertices[0], GL_STATIC_DRAW);  

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint), &this->indices[0], GL_STATIC_DRAW);

        // Set the vertex attribute pointers
        // Vertex Positions
        glEnableVertexAttribArray(0);	
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
        // Vertex Normals
        glEnableVertexAttribArray(1);	
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Normal));
        // Vertex Texture Coords
        glEnableVertexAttribArray(2);	
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, TexCoords));

        glBindVertexArray(0);
    }
};



