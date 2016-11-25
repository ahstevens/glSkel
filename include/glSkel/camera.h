#pragma once

// Std. Includes
#include <vector>

// GL Includes
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glSkel/Object.h>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};
	
// Default camera values
const glm::vec3 m_vec3DefaultPosition =  glm::vec3(0.f, 0.f, 0.f);
const glm::vec3 m_vec3DefaultUp       =  glm::vec3(0.f, 1.f, 0.f);
const float m_fDefaultYaw             = -90.f;
const float m_fDefaultPitch           =   0.f;
const float m_fDefaultSpeed           =  30.f;
const float m_fDefaultSensitivity     =   0.25f;
const float m_fDefaultZoom            =  45.f;
const float m_fDefaultZoomMin         =  45.f;
const float m_fDefaultZoomMax         =   1.f;

// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera : public Object
{
public:
    // Constructor with vectors
    Camera(
		glm::vec3 position = m_vec3DefaultPosition,
		glm::vec3 up = m_vec3DefaultUp,
		float yaw = m_fDefaultYaw,
		float pitch = m_fDefaultPitch
	) 
		: m_vec3Front(glm::vec3(0.f, 0.f, -1.f))
		, m_fMovementSpeed(m_fDefaultSpeed)
		, m_fSensitivity(m_fDefaultSensitivity)
		, m_fZoom(m_fDefaultZoom)
		, m_fZoomMin(m_fDefaultZoomMin)
		, m_fZoomMax(m_fDefaultZoomMax)
    {
        m_vec3Position = position;
		m_vec3WorldUp = up;
        m_fYaw = yaw;
        m_fPitch = pitch;
        updateCameraVectors();
    }

    // Returns the view matrix calculated using Eular Angles and the LookAt Matrix
    glm::mat4 getViewMatrix()
    {
        return glm::lookAt(m_vec3Position, m_vec3Position + m_vec3Front, m_vec3Up);
    }

	float getZoom() 
	{ 
		return m_fZoom; 
	}

    // Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void move(Camera_Movement direction, float deltaTime)
    {
        float velocity = m_fMovementSpeed * deltaTime;
        if (direction == FORWARD)
            m_vec3Position += m_vec3Front * velocity;
        if (direction == BACKWARD)
            m_vec3Position -= m_vec3Front * velocity;
        if (direction == LEFT)
            m_vec3Position -= m_vec3Right * velocity;
        if (direction == RIGHT)
            m_vec3Position += m_vec3Right * velocity;
    }

    // Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void look(float dx, float dy, GLboolean constrainPitch = true)
    {
        dx *= m_fSensitivity;
        dy *= m_fSensitivity;

        m_fYaw   += dx;
        m_fPitch += dy;

        // Make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch)
        {
            if (m_fPitch > 89.f)
				m_fPitch = 89.f;
            if (m_fPitch < -89.f)
				m_fPitch = -89.f;
        }

        // Update Front, Right and Up Vectors using the updated Eular angles
        this->updateCameraVectors();
    }

    // Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void zoom(float dz)
    {
        if (m_fZoom >= m_fZoomMax && m_fZoom <= m_fZoomMin)
			m_fZoom -= dz;
        if (m_fZoom <= m_fZoomMax)
			m_fZoom = m_fZoomMax;
        if (m_fZoom >= m_fZoomMin)
			m_fZoom = m_fZoomMin;
    }

private:
	// Camera Attributes
	glm::vec3 m_vec3Front;
	glm::vec3 m_vec3Up;
	glm::vec3 m_vec3Right;
	glm::vec3 m_vec3WorldUp;

	// Eular Angles
	float m_fYaw;
	float m_fPitch;

	// Camera options
	float m_fMovementSpeed;
	float m_fSensitivity;
	float m_fZoom, m_fZoomMin, m_fZoomMax;

    // Calculates the front vector from the Camera's (updated) Eular Angles
    void updateCameraVectors()
    {
        // Calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(m_fYaw)) * cos(glm::radians(m_fPitch));
        front.y = sin(glm::radians(m_fPitch));
        front.z = sin(glm::radians(m_fYaw)) * cos(glm::radians(m_fPitch));
		m_vec3Front = glm::normalize(front);

        // Also re-calculate the Right and Up vector
		m_vec3Right = glm::normalize(glm::cross(m_vec3Front, m_vec3WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		m_vec3Up    = glm::normalize(glm::cross(m_vec3Right, m_vec3Front));
    }
};
