#pragma once

#include <vector>
#include <random>
#include <ctime>

// GL Includes
#define GLEW_STATIC      // use static GLEW libs
#include <GL/glew.h> // Contains all the necessery OpenGL includes
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glSkel/shader.h>
#include <glSkel/Gabor.h>

class GaborTest
{
public:

	GaborTest()
	{
		this->m_vec3Position = glm::vec3(0.f, 0.f, 0.f);
		this->m_qOrientation = glm::quat();
		setupGeometry();
		setupGL();
	}

	~GaborTest()
	{
	}

	// Render the mesh
	void Draw(Shader shader)
	{                                                                                                        
		glm::mat4 model = glm::mat4();
		model = glm::translate(model, m_vec3Position);
		model *= glm::mat4_cast(m_qOrientation);

		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		
		// Draw mesh
		glBindVertexArray(this->m_glVAO);
		glDrawArrays(GL_LINE_STRIP, 0, m_vVertices.size());
		glBindVertexArray(0);
	}

private:
	struct Vertex {
		glm::vec2 pos;
		glm::vec3 col;
	};

	GLuint m_glVAO, m_glVBO;

	glm::vec3 m_vec3Position;
	glm::quat m_qOrientation;

	std::vector<Vertex> m_vVertices;

	void setupGeometry()
	{
		unsigned int res = 10000u;
		float width = 10;
		float xmin = -width / 2.f;
		float xmax = width / 2.f;
		float xoffset;
		std::vector<Gabor> gabs;

		for (unsigned int i = 0; i < 100; ++i)
		{
			float randratio = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
			float x = width * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
			float subwidth = 1.f;
			Gabor g;
			g.setGaussianKernelCenter(glm::vec2(x, 0.f));
			g.setGaussianKernelSpread(glm::vec2(0.1f + 0.4f * subwidth * randratio, 0.1f + 0.2f * subwidth * randratio));
			g.setGaussianKernelAngle(0.f);
			g.setGaussianKernelAmplitude(0.1f + subwidth * randratio / 5.f);
			g.setComplexSinusoidDistance(1.f + 3.f * randratio);
			g.setComplexSinusoidAngle(90.f);

			gabs.push_back(g);
		}

		for (unsigned int i = 0; i < res; ++i)
		{			
			float x = width * (static_cast<float>(i) / static_cast<float>(res - 1));
			
			Vertex v;
			v.pos.x = x;
			v.pos.y = 0.f;
			v.col = glm::vec3(x/width, 1.f - x / width, 1.f - x / width);

			//Gabor gabor;
			//gabor.setGaussianKernelCenter(glm::vec2(width / 2.f, 0.f));
			//gabor.setGaussianKernelSpread(glm::vec2(width / 4.f, 1.f));
			//gabor.setGaussianKernelAngle(0.f);
			//gabor.setGaussianKernelAmplitude(1.f);
			//gabor.setComplexSinusoidDistance(0.8f);
			//gabor.setComplexSinusoidAngle(90.f);

			//v.pos.y = gabor.get(glm::vec2(x, 0.f));
			for (auto gab : gabs)
				v.pos.y += gab.get(glm::vec2(x, 0.f));

			m_vVertices.push_back(v);
		}

		m_vec3Position = glm::vec3(xmin, 0.f, 0.f);
	}

	// Initializes all the buffer objects/arrays
	void setupGL()
	{
		// Create buffers/arrays
		glGenVertexArrays(1, &this->m_glVAO);
		glGenBuffers(1, &this->m_glVBO);

		glBindVertexArray(this->m_glVAO);
		// Load data into vertex buffers
		glBindBuffer(GL_ARRAY_BUFFER, this->m_glVBO);
		// A great thing about structs is that their memory layout is sequential for all its items.
		// The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
		// again translates to 3/2 floats which translates to a byte array.
		glBufferData(GL_ARRAY_BUFFER, m_vVertices.size() * sizeof(Vertex), &m_vVertices[0], GL_STATIC_DRAW);

		// Set the vertex attribute pointers
		// Vertex Positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
		// Vertex Colors
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, col));

		glBindVertexArray(0);
	}
};

