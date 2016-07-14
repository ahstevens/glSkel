#include "Slatissima.h"

#include <algorithm>


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
	Vertex tempVert;

	GLuint nSamples = 100;

	GLuint counter = 0;

	tempVert.Normal = glm::vec3(0.f, 0.f, 1.f);

	for (GLuint i = 0; i < nSamples; ++i)
	{
		GLfloat lengthRatio = ((float)i / (float)(nSamples - 1));
		GLfloat x_offset = glm::sin(lengthRatio * 3.14159);
		GLfloat y_coord = lengthRatio * length;
		GLfloat z_offset = glm::sin(lengthRatio * 3.14159 * length) * (thickness / 2.f);

		// Center point first
		tempVert.TexCoords = glm::vec2(0.5f, lengthRatio);
		tempVert.Position = glm::vec3(0.f, y_coord, 0.f);

		vertices.push_back(tempVert);

		// Left side
		tempVert.TexCoords = glm::vec2(0.5f - x_offset / 2.f, lengthRatio);
		tempVert.Position = glm::vec3( -(x_offset * width / 2.f), y_coord, z_offset);

		vertices.push_back(tempVert);

		// Right side

		tempVert.TexCoords = glm::vec2(0.5f + x_offset / 2.f, lengthRatio);
		tempVert.Position = glm::vec3(x_offset / 2.f * width, y_coord, z_offset);

		vertices.push_back(tempVert);

		if (i == nSamples - 1) break;

		// Indices
		indices.push_back(counter + 0);
		indices.push_back(counter + 3);
		indices.push_back(counter + 1);
		
		indices.push_back(counter + 1);
		indices.push_back(counter + 3);
		indices.push_back(counter + 4);
		
		indices.push_back(counter + 0);
		indices.push_back(counter + 2);
		indices.push_back(counter + 3);

		indices.push_back(counter + 2);
		indices.push_back(counter + 5);
		indices.push_back(counter + 3);

		counter += 3;
	}


	calcSpineNormals();
	calcEdgeNormals();

	mesh = new Mesh(vertices, indices, this->loadTextures());
}

void Slatissima::calcSpineNormals()
{
	glm::vec3 a, b, normal;

	for (GLuint i = 0; i < vertices.size(); i += 3)
	{
		if (i == 0)
		{
			a = glm::normalize(vertices[i + 3].Position - vertices[i].Position);
			b = glm::normalize(vertices[i + 1].Position - vertices[i].Position);
			normal = glm::cross(a, b);

			a = glm::normalize(vertices[i + 2].Position - vertices[i].Position);
			b = glm::normalize(vertices[i + 3].Position - vertices[i].Position);
			normal += glm::cross(a, b);
		}
		else if (i == vertices.size() - 3)
		{
			a = glm::normalize(vertices[i - 2].Position - vertices[i].Position);
			b = glm::normalize(vertices[i - 3].Position - vertices[i].Position);
			normal = glm::cross(a, b);

			a = glm::normalize(vertices[i + 1].Position - vertices[i].Position);
			b = glm::normalize(vertices[i - 2].Position - vertices[i].Position);
			normal += glm::cross(a, b);

			a = glm::normalize(vertices[i - 1].Position - vertices[i].Position);
			b = glm::normalize(vertices[i + 2].Position - vertices[i].Position);
			normal += glm::cross(a, b);

			a = glm::normalize(vertices[i - 3].Position - vertices[i].Position);
			b = glm::normalize(vertices[i - 1].Position - vertices[i].Position);
			normal += glm::cross(a, b);
		}
		else
		{
			a = glm::normalize(vertices[i + 3].Position - vertices[i].Position);
			b = glm::normalize(vertices[i + 1].Position - vertices[i].Position);
			normal = glm::cross(a, b);

			a = glm::normalize(vertices[i + 2].Position - vertices[i].Position);
			b = glm::normalize(vertices[i + 3].Position - vertices[i].Position);
			normal += glm::cross(a, b);

			a = glm::normalize(vertices[i - 2].Position - vertices[i].Position);
			b = glm::normalize(vertices[i - 3].Position - vertices[i].Position);
			normal += glm::cross(a, b);

			a = glm::normalize(vertices[i + 1].Position - vertices[i].Position);
			b = glm::normalize(vertices[i - 2].Position - vertices[i].Position);
			normal += glm::cross(a, b);

			a = glm::normalize(vertices[i - 1].Position - vertices[i].Position);
			b = glm::normalize(vertices[i + 2].Position - vertices[i].Position);
			normal += glm::cross(a, b);

			a = glm::normalize(vertices[i - 3].Position - vertices[i].Position);
			b = glm::normalize(vertices[i - 1].Position - vertices[i].Position);
			normal += glm::cross(a, b);
		}

		vertices[i].Normal = glm::normalize(normal);

	}
}

void Slatissima::calcEdgeNormals()
{
	glm::vec3 a, b, normal;

	// Left-side verts first
	for (GLuint i = 1; i < vertices.size(); i += 3)
	{

		if (i == 1)
		{
			a = glm::normalize(vertices[i - 1].Position - vertices[i].Position);
			b = glm::normalize(vertices[i + 2].Position - vertices[i].Position);
			normal = glm::cross(a, b);

			a = glm::normalize(vertices[i + 2].Position - vertices[i].Position);
			b = glm::normalize(vertices[i + 3].Position - vertices[i].Position);
			normal += glm::cross(a, b);
		}
		else if (i == vertices.size() - 2)
		{
			a = glm::normalize(vertices[i - 3].Position - vertices[i].Position);
			b = glm::normalize(vertices[i - 1].Position - vertices[i].Position);
			normal = glm::cross(a, b);
		}
		else
		{
			a = glm::normalize(vertices[i - 1].Position - vertices[i].Position);
			b = glm::normalize(vertices[i + 2].Position - vertices[i].Position);
			normal = glm::cross(a, b);

			a = glm::normalize(vertices[i + 2].Position - vertices[i].Position);
			b = glm::normalize(vertices[i + 3].Position - vertices[i].Position);
			normal += glm::cross(a, b);
			
			a = glm::normalize(vertices[i - 3].Position - vertices[i].Position);
			b = glm::normalize(vertices[i - 1].Position - vertices[i].Position);
			normal += glm::cross(a, b);
		}
		
		vertices[i].Normal = glm::normalize(normal);
	}

	// Right-side verts
	for (GLuint i = 2; i < vertices.size(); i += 3)
	{

		if (i == 2)
		{
			a = glm::normalize(vertices[i + 1].Position - vertices[i].Position);
			b = glm::normalize(vertices[i - 2].Position - vertices[i].Position);
			normal = glm::cross(a, b);

			a = glm::normalize(vertices[i + 3].Position - vertices[i].Position);
			b = glm::normalize(vertices[i + 1].Position - vertices[i].Position);
			normal += glm::cross(a, b);
		}
		else if (i == vertices.size() - 1)
		{
			a = glm::normalize(vertices[i - 2].Position - vertices[i].Position);
			b = glm::normalize(vertices[i - 3].Position - vertices[i].Position);
			normal = glm::cross(a, b);
		}
		else
		{
			a = glm::normalize(vertices[i + 1].Position - vertices[i].Position);
			b = glm::normalize(vertices[i - 2].Position - vertices[i].Position);
			normal = glm::cross(a, b);

			a = glm::normalize(vertices[i + 3].Position - vertices[i].Position);
			b = glm::normalize(vertices[i + 1].Position - vertices[i].Position);
			normal += glm::cross(a, b);

			a = glm::normalize(vertices[i - 2].Position - vertices[i].Position);
			b = glm::normalize(vertices[i - 3].Position - vertices[i].Position);
			normal += glm::cross(a, b);
		}

		vertices[i].Normal = glm::normalize(normal);
	}
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
