#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

class Object
{
public:
	Object()
		: m_vec3Position(glm::vec3())
		, m_mat4Rotation(glm::mat4())
	{}

	Object(glm::vec3 position, glm::mat4 rot)
		: m_vec3Position(position)
		, m_mat4Rotation(rot)
	{}

	~Object() {}

	void setPosition(glm::vec3 pos) { m_vec3Position = pos; }
	glm::vec3 getPosition() { return m_vec3Position; }

	void setOrientation(glm::mat4 rot) { m_mat4Rotation = rot; }
	glm::mat4 getOrientation() { return m_mat4Rotation; }

	void setScale(glm::vec3 s) { m_vec3Scale = s; }
	void setScale(float s) { m_vec3Scale = glm::vec3(s); }
	glm::vec3 getScale() { return m_vec3Scale; }

protected:
	glm::vec3 m_vec3Position;
	glm::mat4 m_mat4Rotation;
	glm::vec3 m_vec3Scale;
};

