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
	~Object() {}

	void setPosition(glm::vec3 pos) { m_vec3Position = pos; }
	glm::vec3 getPosition() { return m_vec3Position; }

	void setOrientation(glm::quat o) { m_quatOrientation = o; }
	glm::quat getOrientation() { return m_quatOrientation; }

protected:
	glm::vec3 m_vec3Position;
	glm::quat m_quatOrientation;
};

