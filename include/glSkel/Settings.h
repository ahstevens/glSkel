#pragma once
#include <glSkel/mesh.h>
#include <glSkel/shader.h>

#include <glSkel/Observer.h>

#include "PhysicsSystem.h"

class Settings : public Observer
{
public:
	PhysicsSystem* m_pPhysicsSystem;

	bool m_bShowLights;
	bool m_bShowNormals;
	bool m_bExplode; 

	// Constants
	const int m_iWidth = 1280;
	const int m_iHeight = 800;

	float m_fDeltaTime;	// Time between current frame and last frame
	float m_fLastFrame; // Time of last frame

public:
	Settings()
		: m_pPhysicsSystem(NULL)
		, m_bShowLights(true)
		, m_bShowNormals(false)
		, m_bExplode(false)
		, m_fDeltaTime(0.f)
		, m_fLastFrame(0.f)
	{

	}

	~Settings()
	{

	}

	void receiveEvent(Object * obj, const int event, void * data)
	{
		int key;
		memcpy(&key, data, sizeof(key));

		if (event == Observer::KEY_PRESS)
		{
			if (key == GLFW_KEY_L)
				m_bShowLights = !m_bShowLights;
			if (key == GLFW_KEY_N)
				m_bShowNormals = !m_bShowNormals;
			if (key == GLFW_KEY_B)
				m_bExplode = !m_bExplode;
		}
	}

private:
	
};

