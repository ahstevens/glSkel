#pragma once
#include <glSkel/mesh.h>
#include <glSkel/shader.h>

#include <glSkel/Observer.h>

#include "PhysicsSystem.h"
#include "GLFWInputBroadcaster.h"
#include "SLatissima.h"

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

	std::vector<Slatissima *> slats;

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
			if (key == GLFW_KEY_R)
				generateModels();
		}
	}

	void generateModels()
	{
		Slatissima *slat;
		unsigned int nSlats = 4u;
		float spaceBetween = 5.f;

		bool isEmpty = slats.size() == 0;

		if (!isEmpty)
		{
			slats.clear();
		}

		for (int i = 0u; i < nSlats; ++i)
		{
			if (!isEmpty)
			{
				GLFWInputBroadcaster::getInstance().detach(slats[i]);
				delete slats[i];
			}

			slat = new Slatissima(0.5f);
			slat->setPosition(glm::vec3(-(nSlats * spaceBetween / 2) + i * spaceBetween, 0.f, 0.f));
			//slat->setOrientation(glm::angleAxis(glm::radians((static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 180.f), glm::vec3(0.f, 1.f, 0.f)));
			slat->setOrientation(glm::angleAxis(glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f)));
			slat->initPhysics(m_pPhysicsSystem->getSoftDynamicsWorld());
			slat->anchorToBody(m_pPhysicsSystem->getGroundBody());
			GLFWInputBroadcaster::getInstance().attach(slat);
			slats.push_back(slat);
		}
	}

private:
	
};

