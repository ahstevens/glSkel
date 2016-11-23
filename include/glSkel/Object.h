#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

class Object
{
public:
	Object()
		: m_vec3Position(glm::vec3())
		, m_quatOrientation(glm::quat())
	{}

	Object(glm::vec3 position, glm::quat orientation)
		: m_vec3Position(position)
		, m_quatOrientation(orientation)
	{}

	~Object() {}

	void setPosition(glm::vec3 pos) { m_vec3Position = pos; }
	glm::vec3 getPosition() { return m_vec3Position; }

	void setOrientation(glm::quat o) { m_quatOrientation = o; }
	glm::quat getOrientation() { return m_quatOrientation; }

	void setScale(glm::vec3 s) { m_vec3Scale = s; }
	void setScale(float s) { m_vec3Scale = glm::vec3(s); }
	glm::vec3 getScale() { return m_vec3Scale; }

protected:
	glm::vec3 m_vec3Position;
	glm::quat m_quatOrientation;
	glm::vec3 m_vec3Scale;
};

