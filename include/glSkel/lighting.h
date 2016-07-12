#ifndef LIGHTING_H
#define LIGHTING_H

#include <GL/glew.h>

#include <glSkel/shader.h>

struct BasicLight {	
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
};

struct DLight : BasicLight {
	glm::vec3 direction;
};

struct PLight : BasicLight {
	glm::vec3 position;
	GLfloat constant;
	GLfloat linear;
	GLfloat quadratic;
};

struct SLight : PLight {
	glm::vec3 direction;
	GLfloat cutOff;
	GLfloat outerCutOff;
};


#define MAX_N_PLIGHTS 4


class LightingSystem
{
public:
	DLight dLight;
	std::vector<PLight> pLights;
	SLight sLight;

    LightingSystem()
    {
		setupLightMesh();
    }

    // Uses the current shader
    void SetupLighting(Shader s) 
	{ 
		// == ==========================
		// Here we set all the uniforms for the 5/6 types of lights we have. We have to set them manually and index 
		// the proper PointLight struct in the array to set each uniform variable. This can be done more code-friendly
		// by defining light types as classes and set their values in there, or by using a more efficient uniform approach
		// by using 'Uniform buffer objects', but that is something we discuss in the 'Advanced GLSL' tutorial.
		// == ==========================
		// Directional light
		glUniform3fv(glGetUniformLocation(s.Program, "dirLight.direction"), 3, glm::value_ptr(dLight.direction));
		glUniform3fv(glGetUniformLocation(s.Program, "dirLight.ambient"), 3, glm::value_ptr(dLight.ambient));
		glUniform3fv(glGetUniformLocation(s.Program, "dirLight.diffuse"), 3, glm::value_ptr(dLight.diffuse));
		glUniform3fv(glGetUniformLocation(s.Program, "dirLight.specular"), 3, glm::value_ptr(dLight.specular));

		// Point light
		for (int i = 0; i < pLights.size(); ++i)
		{
			std::string name = "pointLights[" + std::to_string(i);
			name += "]";

			std::cout << name << std::endl;
			glUniform3fv(glGetUniformLocation(s.Program, (name + ".position").c_str()), 3, glm::value_ptr(pLights[i].position));
			glUniform3fv(glGetUniformLocation(s.Program, (name + ".ambient").c_str()), 3, glm::value_ptr(pLights[i].ambient));
			glUniform3fv(glGetUniformLocation(s.Program, (name + ".diffuse").c_str()), 3, glm::value_ptr(pLights[i].diffuse));
			glUniform3fv(glGetUniformLocation(s.Program, (name + ".specular").c_str()), 3, glm::value_ptr(pLights[i].specular));
			glUniform1f(glGetUniformLocation(s.Program, (name + ".constant").c_str()), pLights[i].constant);
			glUniform1f(glGetUniformLocation(s.Program, (name + ".linear").c_str()), pLights[i].linear);
			glUniform1f(glGetUniformLocation(s.Program, (name + ".quadratic").c_str()), pLights[i].quadratic);
		}

		// SpotLight
		glUniform3fv(glGetUniformLocation(s.Program, "spotLight.position"), 3, glm::value_ptr(sLight.position));
		glUniform3fv(glGetUniformLocation(s.Program, "spotLight.direction"), 3, glm::value_ptr(sLight.direction));
		glUniform3fv(glGetUniformLocation(s.Program, "spotLight.ambient"), 3, glm::value_ptr(sLight.ambient));
		glUniform3fv(glGetUniformLocation(s.Program, "spotLight.diffuse"), 3, glm::value_ptr(sLight.diffuse));
		glUniform3fv(glGetUniformLocation(s.Program, "spotLight.specular"), 3, glm::value_ptr(sLight.specular));
		glUniform1f(glGetUniformLocation(s.Program, "spotLight.constant"), sLight.constant);
		glUniform1f(glGetUniformLocation(s.Program, "spotLight.linear"), sLight.linear);
		glUniform1f(glGetUniformLocation(s.Program, "spotLight.quadratic"), sLight.quadratic);
		glUniform1f(glGetUniformLocation(s.Program, "spotLight.cutOff"), sLight.cutOff);
		glUniform1f(glGetUniformLocation(s.Program, "spotLight.outerCutOff"), sLight.outerCutOff);
	}

	bool addDLight(glm::vec3 direction = glm::vec3(-1.0f),
		glm::vec3 ambient = glm::vec3(0.05f), glm::vec3 diffuse = glm::vec3(0.4f), glm::vec3 specular = glm::vec3(0.5f))
	{
		this->dLight.direction = direction;
		this->dLight.ambient = ambient;
		this->dLight.diffuse = diffuse;
		this->dLight.specular = specular;

		return true;
	}

	bool addPLight(glm::vec3 position = glm::vec3(1.0f),
		glm::vec3 ambient = glm::vec3(0.05f), glm::vec3 diffuse = glm::vec3(0.8f), glm::vec3 specular = glm::vec3(1.0f),
		GLfloat constant = 1.0f, GLfloat linear = 0.09f, GLfloat quadratic = 0.032f)
	{
		if (pLights.size() == MAX_N_PLIGHTS) return false;

		PLight pl;
		pl.position = position;
		pl.ambient = ambient;
		pl.diffuse = diffuse;
		pl.specular = specular;
		pl.constant = constant;
		pl.linear = linear;
		pl.quadratic = quadratic;

		pLights.push_back(pl);

		return true;
	}

	bool addSLight(glm::vec3 position = glm::vec3(1.0f), glm::vec3 direction = glm::vec3(0.0f),
		glm::vec3 ambient = glm::vec3(0.0f), glm::vec3 diffuse = glm::vec3(1.0f), glm::vec3 specular = glm::vec3(1.0f),
		GLfloat constant = 1.0f, GLfloat linear = 0.09f, GLfloat quadratic = 0.032f,
		GLfloat cutOffDeg = glm::cos(glm::radians(12.5f)), GLfloat outerCutOffDeg = glm::cos(glm::radians(15.0f)))
	{
		this->sLight.position = position;
		this->sLight.direction = direction;
		this->sLight.ambient = ambient;
		this->sLight.diffuse = diffuse;
		this->sLight.specular = specular;
		this->sLight.constant = constant;
		this->sLight.linear = linear;
		this->sLight.quadratic = quadratic;
		this->sLight.cutOff = glm::cos(glm::radians(cutOffDeg));
		this->sLight.outerCutOff = glm::cos(glm::radians(outerCutOffDeg));

		return true;
	}

	void Draw(Shader s)
	{
		glm::mat4 model = glm::mat4();

		glBindVertexArray(this->VAO);
		for (GLuint i = 0; i < pLights.size(); ++i)
		{
			model = glm::mat4();
			model = glm::translate(model, pLights[i].position);
			model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
			glUniformMatrix4fv(glGetUniformLocation(s.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
			glDrawElements(GL_TRIANGLES, this->nIndices, GL_UNSIGNED_INT, 0);
		}
		glBindVertexArray(0);

	}

private:
	GLuint VBO, VAO, EBO, nIndices;

    void setupLightMesh()
	{
		std::vector<glm::vec3> vertices;
		std::vector<GLuint> indices;
		std::vector<GLuint> faceInds;

		glm::vec3 frontBotLeft = glm::vec3(-0.5f, -0.5f, 0.5f); // 0
		glm::vec3 frontBotRight = glm::vec3(0.5f, -0.5f, 0.5f); // 1
		glm::vec3 frontTopRight = glm::vec3(0.5f, 0.5f, 0.5f);  // 2
		glm::vec3 frontTopLeft = glm::vec3(-0.5f, 0.5f, 0.5f);  // 3
		glm::vec3 backBotRight = glm::vec3(0.5f, -0.5f, -0.5f); // 4
		glm::vec3 backBotLeft = glm::vec3(-0.5f, -0.5f, -0.5f); // 5
		glm::vec3 backTopLeft = glm::vec3(-0.5f, 0.5f, -0.5f);  // 6
		glm::vec3 backTopRight = glm::vec3(0.5f, 0.5f, -0.5f);  // 7

		vertices = { frontBotLeft, frontBotRight, frontTopRight, frontTopLeft,
			backBotRight, backBotLeft, backTopLeft, backTopRight };

		// Face 1		
		faceInds = { 0, 1, 2, 2, 3, 0 };
		indices.insert(indices.end(), faceInds.begin(), faceInds.end());

		// Face 2
		faceInds = { 4, 5, 6, 6, 7, 4 };
		indices.insert(indices.end(), faceInds.begin(), faceInds.end());

		// Face 3
		faceInds = { 4, 0, 2, 2, 6, 4 };
		indices.insert(indices.end(), faceInds.begin(), faceInds.end());

		// Face 4
		faceInds = { 5, 1, 3, 3, 7, 5 };
		indices.insert(indices.end(), faceInds.begin(), faceInds.end());

		// Face 5
		faceInds = { 2, 3, 7, 7, 6, 2 };
		indices.insert(indices.end(), faceInds.begin(), faceInds.end());

		// Face 6
		faceInds = { 0, 1, 5, 5, 4, 0 };
		indices.insert(indices.end(), faceInds.begin(), faceInds.end());

		glGenVertexArrays(1, &this->VAO);
		glGenBuffers(1, &this->VBO);
		glGenBuffers(1, &this->EBO);

		glBindVertexArray(this->VAO);
		// Load data into vertex buffers
		glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

		// Set the vertex attributes (only position data for the lamp))
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);
		glBindVertexArray(0);

		nIndices = indices.size();
	}
};

#endif