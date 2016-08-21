#include "Cube.h"

#include <algorithm>



Cube::Cube()
{
	std::vector<glm::vec3> vertices;
	std::vector<GLuint> indices;
	glm::vec3 tempVert;
	std::vector<GLuint> tempInds = { 0, 1, 2, 2, 3, 0 };

	glm::vec3 backTopLeft   = glm::vec3(-0.5f,  0.5f, -0.5f);
	glm::vec3 backTopRight  = glm::vec3( 0.5f,  0.5f, -0.5f);
	glm::vec3 backBotLeft   = glm::vec3(-0.5f, -0.5f, -0.5f);
	glm::vec3 backBotRight  = glm::vec3( 0.5f, -0.5f, -0.5f);
	glm::vec3 frontTopLeft  = glm::vec3(-0.5f,  0.5f,  0.5f);
	glm::vec3 frontTopRight = glm::vec3( 0.5f,  0.5f,  0.5f);
	glm::vec3 frontBotLeft  = glm::vec3(-0.5f, -0.5f,  0.5f);
	glm::vec3 frontBotRight = glm::vec3( 0.5f, -0.5f,  0.5f);

	// Face 1
	tempVert = backBotLeft;  vertices.push_back(tempVert);
	tempVert = backBotRight; vertices.push_back(tempVert);
	tempVert = backTopRight; vertices.push_back(tempVert);
	tempVert = backTopLeft;  vertices.push_back(tempVert);

	indices.insert(indices.end(), tempInds.begin(), tempInds.end());

	std::transform(tempInds.begin(), tempInds.end(), tempInds.begin(), [](GLuint n) { return n + 4; });

	// Face 2
	tempVert = frontBotLeft;  vertices.push_back(tempVert);
	tempVert = frontBotRight; vertices.push_back(tempVert);
	tempVert = frontTopRight; vertices.push_back(tempVert);
	tempVert = frontTopLeft;  vertices.push_back(tempVert);

	indices.insert(indices.end(), tempInds.begin(), tempInds.end());

	std::transform(tempInds.begin(), tempInds.end(), tempInds.begin(), [](GLuint n) { return n + 4; });

	// Face 3
	tempVert = frontTopLeft; vertices.push_back(tempVert);
	tempVert = backTopLeft;  vertices.push_back(tempVert);
	tempVert = backBotLeft;  vertices.push_back(tempVert);
	tempVert = frontBotLeft; vertices.push_back(tempVert);

	indices.insert(indices.end(), tempInds.begin(), tempInds.end());

	std::transform(tempInds.begin(), tempInds.end(), tempInds.begin(), [](GLuint n) { return n + 4; });

	// Face 4
	tempVert = frontTopRight; vertices.push_back(tempVert);
	tempVert = backTopRight;  vertices.push_back(tempVert);
	tempVert = backBotRight;  vertices.push_back(tempVert);
	tempVert = frontBotRight; vertices.push_back(tempVert);

	indices.insert(indices.end(), tempInds.begin(), tempInds.end());

	std::transform(tempInds.begin(), tempInds.end(), tempInds.begin(), [](GLuint n) { return n + 4; });

	// Face 5
	tempVert = backBotLeft;   vertices.push_back(tempVert);
	tempVert = backBotRight;  vertices.push_back(tempVert);
	tempVert = frontBotRight; vertices.push_back(tempVert);
	tempVert = frontBotLeft;  vertices.push_back(tempVert);

	indices.insert(indices.end(), tempInds.begin(), tempInds.end());

	std::transform(tempInds.begin(), tempInds.end(), tempInds.begin(), [](GLuint n) { return n + 4; });

	// Face 6
	tempVert = backTopLeft;   vertices.push_back(tempVert);
	tempVert = backTopRight;  vertices.push_back(tempVert);
	tempVert = frontTopRight; vertices.push_back(tempVert);
	tempVert = frontTopLeft;  vertices.push_back(tempVert);

	indices.insert(indices.end(), tempInds.begin(), tempInds.end());


	// Load textures
	Texture diffuseMap, specularMap;
	glGenTextures(1, &diffuseMap.id);
	glGenTextures(1, &specularMap.id);
	int width = 1, height = 1;
	unsigned char image[3];

	// Diffuse map
	diffuseMap.type = "texture_diffuse";
	image[0] = 0xFF;
	image[1] = 0x88;
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

	mesh = new Mesh(vertices, indices, textures);

}


Cube::~Cube()
{
	if (mesh)
		delete(mesh);
}

void Cube::Draw(Shader s)
{
	for (int i = 0; i < positions.size(); ++i)
	{
		mesh->position = positions[i];
		mesh->angle = angles[i];
		mesh->Draw(s);
	}
}
