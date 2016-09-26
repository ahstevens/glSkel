#pragma once

#include <vector>

// GL Includes
#define GLEW_STATIC      // use static GLEW libs
#include <GL/glew.h> // Contains all the necessery OpenGL includes
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <bullet/LinearMath/btIDebugDraw.h>

#include <glSkel/shader.h>

class BulletDebugDrawer : public btIDebugDraw
{
public:
	BulletDebugDrawer()
	{
		initGL();
	}

	void BulletDebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color) {
		m_vVertices.push_back(DebugVertex(glm::vec3(from.getX(), from.getY(), from.getZ()), glm::vec3(color.getX(), color.getY(), color.getZ())));
		m_vVertices.push_back(DebugVertex(glm::vec3(to.getX(), to.getY(), to.getZ()), glm::vec3(color.getX(), color.getY(), color.getZ())));
	}

	void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color)
	{
		drawLine(PointOnB, PointOnB + normalOnB * distance, color);
	}

	void reportErrorWarning(const char* warningString)
	{
		std::cerr << "===================" << std::endl;
		std::cerr << "DEBUG DRAWER ERROR:" << std::endl;
		std::cerr << warningString << std::endl;
		std::cerr << "===================" << std::endl;
	}

	void draw3dText(const btVector3& location, const char* textString)
	{}

	void setDebugMode(int debugMode)
	{ 
		m_iDebugMode = debugMode;
	}

	int getDebugMode() const 
	{ 
		return m_iDebugMode; 
	}

	// Render the mesh
	void Draw(Shader shader)
	{
		glBindBuffer(GL_ARRAY_BUFFER, this->m_glVBO);
		glBufferData(GL_ARRAY_BUFFER, m_vVertices.size() * sizeof(DebugVertex), m_vVertices.data(), GL_STREAM_DRAW);

		glm::mat4 model = glm::mat4();
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		
		// Draw mesh
		glBindVertexArray(this->m_glVAO);
		glDrawArrays(GL_LINES, 0, m_vVertices.size());
		glBindVertexArray(0);
	}

	void flushLines()
	{
		m_vVertices.clear();
	}

private:
	struct DebugVertex {
		glm::vec3 pos;
		glm::vec3 col;

		DebugVertex(glm::vec3 p, glm::vec3 c)
			: pos(p)
			, col(c)
		{}
	};

	GLuint m_glVAO, m_glVBO;
	int m_iDebugMode;
	std::vector<DebugVertex> m_vVertices;

	void initGL()
	{
		// Create buffers/arrays
		glGenVertexArrays(1, &this->m_glVAO);
		glGenBuffers(1, &this->m_glVBO);

		glBindVertexArray(this->m_glVAO);

		glBindBuffer(GL_ARRAY_BUFFER, this->m_glVBO);

		// Set the vertex attribute pointers
		// Vertex Positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (GLvoid*)0);
		// Vertex Colors
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (GLvoid*)offsetof(DebugVertex, col));

		glBindVertexArray(0);
	}
};
