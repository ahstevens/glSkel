#include "Slatissima.h"


Slatissima::Slatissima(GLfloat length, GLfloat width, GLfloat thickness)
{
	this->buildModel(length, width, thickness);
}


Slatissima::~Slatissima()
{
	if (mesh)
		delete(mesh);
}

void Slatissima::Draw(Shader s)
{
	for (int i = 0; i < positions.size(); ++i)
	{
		mesh->position = positions[i];
		mesh->angle = angles[i];
		mesh->Draw(s);
	}
}

void Slatissima::buildModel(GLfloat length, GLfloat width, GLfloat thickness)
{
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	Vertex tempVert;
	std::vector<GLuint> tempInds = { 0, 1, 2, 2, 3, 0 };

	length /= 2;
	width /= 2;
	thickness /= 2;

	glm::vec3 backTopLeft   = glm::vec3(-width,  length, -thickness);
	glm::vec3 backTopRight  = glm::vec3( width,  length, -thickness);
	glm::vec3 backBotLeft   = glm::vec3(-width, -length, -thickness);
	glm::vec3 backBotRight  = glm::vec3( width, -length, -thickness);
	glm::vec3 frontTopLeft  = glm::vec3(-width,  length,  thickness);
	glm::vec3 frontTopRight = glm::vec3( width,  length,  thickness);
	glm::vec3 frontBotLeft  = glm::vec3(-width, -length,  thickness);
	glm::vec3 frontBotRight = glm::vec3( width, -length,  thickness);

	// Face 1
	tempVert.Normal = glm::vec3(0.0f, 0.0f, -1.0f);

	tempVert.Position = backBotLeft;  tempVert.TexCoords = glm::vec2(0.0f, 0.0f); vertices.push_back(tempVert);
	tempVert.Position = backBotRight; tempVert.TexCoords = glm::vec2(1.0f, 0.0f); vertices.push_back(tempVert);
	tempVert.Position = backTopRight; tempVert.TexCoords = glm::vec2(1.0f, 1.0f); vertices.push_back(tempVert);
	tempVert.Position = backTopLeft;  tempVert.TexCoords = glm::vec2(0.0f, 1.0f); vertices.push_back(tempVert);

	indices.insert(indices.end(), tempInds.begin(), tempInds.end());

	std::transform(tempInds.begin(), tempInds.end(), tempInds.begin(), [](GLuint n) { return n + 4; });

	// Face 2
	tempVert.Normal = glm::vec3(0.0f, 0.0f, 1.0f);

	tempVert.Position = frontBotLeft;  tempVert.TexCoords = glm::vec2(0.0f, 0.0f); vertices.push_back(tempVert);
	tempVert.Position = frontBotRight; tempVert.TexCoords = glm::vec2(1.0f, 0.0f); vertices.push_back(tempVert);
	tempVert.Position = frontTopRight; tempVert.TexCoords = glm::vec2(1.0f, 1.0f); vertices.push_back(tempVert);
	tempVert.Position = frontTopLeft;  tempVert.TexCoords = glm::vec2(0.0f, 1.0f); vertices.push_back(tempVert);

	indices.insert(indices.end(), tempInds.begin(), tempInds.end());

	std::transform(tempInds.begin(), tempInds.end(), tempInds.begin(), [](GLuint n) { return n + 4; });

	// Face 3
	tempVert.Normal = glm::vec3(-1.0f, 0.0f, 0.0f);

	tempVert.Position = frontTopLeft; tempVert.TexCoords = glm::vec2(1.0f, 0.0f); vertices.push_back(tempVert);
	tempVert.Position = backTopLeft;  tempVert.TexCoords = glm::vec2(1.0f, 1.0f); vertices.push_back(tempVert);
	tempVert.Position = backBotLeft;  tempVert.TexCoords = glm::vec2(0.0f, 1.0f); vertices.push_back(tempVert);
	tempVert.Position = frontBotLeft; tempVert.TexCoords = glm::vec2(0.0f, 0.0f); vertices.push_back(tempVert);

	indices.insert(indices.end(), tempInds.begin(), tempInds.end());

	std::transform(tempInds.begin(), tempInds.end(), tempInds.begin(), [](GLuint n) { return n + 4; });

	// Face 4
	tempVert.Normal = glm::vec3(1.0f, 0.0f, 0.0f);

	tempVert.Position = frontTopRight; tempVert.TexCoords = glm::vec2(1.0f, 0.0f); vertices.push_back(tempVert);
	tempVert.Position = backTopRight;  tempVert.TexCoords = glm::vec2(1.0f, 1.0f); vertices.push_back(tempVert);
	tempVert.Position = backBotRight;  tempVert.TexCoords = glm::vec2(0.0f, 1.0f); vertices.push_back(tempVert);
	tempVert.Position = frontBotRight; tempVert.TexCoords = glm::vec2(0.0f, 0.0f); vertices.push_back(tempVert);

	indices.insert(indices.end(), tempInds.begin(), tempInds.end());

	std::transform(tempInds.begin(), tempInds.end(), tempInds.begin(), [](GLuint n) { return n + 4; });

	// Face 5
	tempVert.Normal = glm::vec3(0.0f, -1.0f, 0.0f);

	tempVert.Position = backBotLeft;   tempVert.TexCoords = glm::vec2(0.0f, 1.0f); vertices.push_back(tempVert);
	tempVert.Position = backBotRight;  tempVert.TexCoords = glm::vec2(1.0f, 1.0f); vertices.push_back(tempVert);
	tempVert.Position = frontBotRight; tempVert.TexCoords = glm::vec2(1.0f, 0.0f); vertices.push_back(tempVert);
	tempVert.Position = frontBotLeft;  tempVert.TexCoords = glm::vec2(0.0f, 0.0f); vertices.push_back(tempVert);

	indices.insert(indices.end(), tempInds.begin(), tempInds.end());

	std::transform(tempInds.begin(), tempInds.end(), tempInds.begin(), [](GLuint n) { return n + 4; });

	// Face 6
	tempVert.Normal = glm::vec3(0.0f, 1.0f, 0.0f);

	tempVert.Position = backTopLeft;   tempVert.TexCoords = glm::vec2(0.0f, 1.0f); vertices.push_back(tempVert);
	tempVert.Position = backTopRight;  tempVert.TexCoords = glm::vec2(1.0f, 1.0f); vertices.push_back(tempVert);
	tempVert.Position = frontTopRight; tempVert.TexCoords = glm::vec2(1.0f, 0.0f); vertices.push_back(tempVert);
	tempVert.Position = frontTopLeft;  tempVert.TexCoords = glm::vec2(0.0f, 0.0f); vertices.push_back(tempVert);

	indices.insert(indices.end(), tempInds.begin(), tempInds.end());

	mesh = new Mesh(vertices, indices, this->loadTextures());
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
	image[0] = 0xFF;
	image[1] = 0xFF;
	image[2] = 0xFF;
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
