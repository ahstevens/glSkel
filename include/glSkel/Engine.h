#pragma once
#include <glSkel/mesh.h>
#include <glSkel/shader.h>

#include <glSkel/BroadcastSystem.h>
#include <glSkel/shader.h>
#include <glSkel/camera.h>
#include <glSkel/mesh.h>
#include <glSkel/lighting.h>

#include "PhysicsSystem.h"
#include "GLFWInputBroadcaster.h"
#include "Ground.h"
#include "AnchorPoint.h"
#include "SLatissima.h"

#define MS_PER_UPDATE 0.0333333333f
#define CAST_RAY_LEN 1000.f

class Engine : public BroadcastSystem::Listener
{
public:
	GLFWwindow * m_pWindow;
	PhysicsSystem* m_pPhysicsSystem;
	LightingSystem* m_pLightingSystem;

	bool m_bRunPhysics;
	bool m_bShowLights;
	bool m_bShowNormals;
	bool m_bExplode;

	// Constants
	const int m_iWidth = 1280;
	const int m_iHeight = 800;
	const float m_fStepSize = 1.f / 120.f;

	float m_fDeltaTime;	// Time between current frame and last frame
	float m_fLastTime; // Time of last frame

	Camera  *m_pCamera;
	std::vector<Shader*> m_vpShaders;
	Shader *m_pShaderLighting, *m_pShaderLamps, *m_pShaderNormals, *m_pShaderExplode, *m_pShaderLines;

	GLint m_iViewLocLightingShader;
	GLint m_iProjLocLightingShader;
	GLint m_iViewPosLocLightingShader;
	GLint m_iShininessLightingShader;

	std::vector<Slatissima*> slats;
	std::vector<AnchorPoint*> anchors;
	Ground* m_pGround;

public:
	Engine()
		: m_pWindow(NULL)
		, m_pPhysicsSystem(NULL)
		, m_pLightingSystem(NULL)
		, m_bRunPhysics(false)
		, m_bShowLights(true)
		, m_bShowNormals(false)
		, m_bExplode(false)
		, m_fDeltaTime(0.f)
		, m_fLastTime(0.f)
		, m_pCamera(NULL)
		, m_pShaderLighting(NULL)
		, m_pShaderLamps(NULL)
		, m_pShaderNormals(NULL)
		, m_pShaderExplode(NULL)
		, m_pShaderLines(NULL)
		, m_iViewLocLightingShader(-1)
		, m_iProjLocLightingShader(-1)
		, m_iViewPosLocLightingShader(-1)
		, m_iShininessLightingShader(-1)
	{
	}

	~Engine()
	{
		if (slats.size())
		{
			for (auto s : slats)
				delete s;
			slats.clear();
		}
	}

	void receiveEvent(Object * obj, const int event, void * data)
	{
		if (event == BroadcastSystem::EVENT::KEY_PRESS)
		{
			int key;
			memcpy(&key, data, sizeof(key));

			if (key == GLFW_KEY_L)
				m_bShowLights = !m_bShowLights;
			if (key == GLFW_KEY_N)
				m_bShowNormals = !m_bShowNormals;
			if (key == GLFW_KEY_B)
				m_bExplode = !m_bExplode;
			if (key == GLFW_KEY_R)
				generateModels();
			if (key == GLFW_KEY_SPACE)
				m_bRunPhysics = !m_bRunPhysics;
		}

		if (event == BroadcastSystem::EVENT::MOUSE_UNCLICK)
		{
			int button;
			memcpy(&button, data, sizeof(button));

			if (button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_RIGHT)
			{
				glm::vec3 rayFrom = m_pCamera->getPosition();
				glm::vec3 rayTo = rayFrom + m_pCamera->getOrientation()[2] * CAST_RAY_LEN;
				glm::vec3 payload[2] = { rayFrom, rayTo };

				BroadcastSystem::EVENT rayType = button == GLFW_MOUSE_BUTTON_LEFT ? BroadcastSystem::EVENT::GROW_RAY : BroadcastSystem::EVENT::SHRINK_RAY;

				for (auto& s : slats)
				{
					s->receiveEvent(m_pCamera, rayType, &payload);
				}
			}
		}
	}

	bool init()
	{
		m_pWindow = init_gl_context("Saccharina latissima");

		if (!m_pWindow)
			return false;

		GLFWInputBroadcaster::getInstance().init(m_pWindow);
		GLFWInputBroadcaster::getInstance().attach(this);  // Register self with input broadcaster

		m_pPhysicsSystem = new PhysicsSystem();
		m_pPhysicsSystem->init();

		init_shaders();
		init_camera();
		init_lighting();
		//generateModels();
		//generateQuad1Models();
		//generateQuad2Models();
		generateNewModels();

		return true;
	}

	void mainLoop()
	{
		m_fLastTime = static_cast<float>(glfwGetTime());

		// Main Rendering Loop
		while (!glfwWindowShouldClose(m_pWindow)) {
			// Calculate deltatime of current frame
			float newTime = static_cast<float>(glfwGetTime());
			m_fDeltaTime = newTime - m_fLastTime;
			m_fLastTime = newTime;

			// Poll input events first
			GLFWInputBroadcaster::getInstance().poll();

			update(m_fStepSize);

			render();

			// Flip buffers and render to screen
			glfwSwapBuffers(m_pWindow);
		}
	}

	void update(float dt)
	{
		m_pCamera->update(dt);

		// update soft mesh vertices
		for (auto &s : slats)
			s->update();

		if (m_bRunPhysics)
		{
			m_pPhysicsSystem->update(dt);
		}

		btTransform trans;
		trans.setIdentity();

		// draw the quadrat boundary
		m_pPhysicsSystem->getDebugDrawer()->setTransform(trans);
		m_pPhysicsSystem->getDebugDrawer()->drawBox(btVector3(0.f, 0.f, 0.f), btVector3(50.f, 50.f, -50.f), btVector3(1.f, 0.f, 0.f));
	}

	void render()
	{
		// OpenGL options
		glEnable(GL_DEPTH_TEST);
		glLineWidth(5.f);

		// Background Fill Color
		glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use corresponding shader when setting uniforms/drawing objects
		m_pShaderLighting->Use();
		glUniform3f(m_iViewPosLocLightingShader, m_pCamera->getPosition().x, m_pCamera->getPosition().y, m_pCamera->getPosition().z);

		m_pLightingSystem->sLight.position = m_pCamera->getPosition();
		m_pLightingSystem->sLight.direction = glm::vec3(m_pCamera->getOrientation()[2]);

		m_pLightingSystem->SetupLighting(*m_pShaderLighting);

		// Create camera transformations
		glm::mat4 view = m_pCamera->getViewMatrix();
		glm::mat4 projection = glm::perspective(
			glm::radians(m_pCamera->getZoom()),
			static_cast<float>(m_iWidth) / static_cast<float>(m_iHeight),
			0.01f,
			1000.0f
		);

		// Pass the matrices to the shader
		glUniformMatrix4fv(m_iViewLocLightingShader, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(m_iProjLocLightingShader, 1, GL_FALSE, glm::value_ptr(projection));

		// Set material properties
		glUniform1f(m_iShininessLightingShader, 32.0f);

		for (auto& shader : m_vpShaders)
		{
			if (shader->status())
			{
				shader->Use();
				glUniformMatrix4fv(glGetUniformLocation(shader->Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
				glUniformMatrix4fv(glGetUniformLocation(shader->Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

				if (shader == m_pShaderLines)
				{
					m_pPhysicsSystem->getDebugDrawer()->Draw(*shader);
				}
				else if (shader == m_pShaderLamps)
				{
					m_pLightingSystem->Draw(*shader);
				}
				else
				{
					m_pGround->Draw(*shader);

					for (auto &a : anchors)
						a->Draw(*shader);

					for (auto &s : slats)
						s->Draw(*shader);
				}
			}
		}

		Shader::Off();
	}

private:
	GLFWwindow * init_gl_context(std::string winName)
	{
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
		GLFWwindow* mWindow = glfwCreateWindow(m_iWidth, m_iHeight, winName.c_str(), nullptr, nullptr);

		// Check for Valid Context
		if (mWindow == nullptr)
			return nullptr;

		// Create Context and Load OpenGL Functions
		glfwMakeContextCurrent(mWindow);

		// GLFW Options
		glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
		glewExperimental = GL_TRUE;
		glewInit();
		fprintf(stderr, "OpenGL %s\n", glGetString(GL_VERSION));

		// Define the viewport dimensions
		glViewport(0, 0, m_iWidth, m_iHeight);

		return mWindow;
	}

	// Initialize the lighting system
	void init_lighting()
	{
		m_pLightingSystem = new LightingSystem();
		GLFWInputBroadcaster::getInstance().attach(m_pLightingSystem);

		// Directional light
		m_pLightingSystem->addDLight(glm::vec3(-1.f, -1.f, -1.f), glm::vec3(0.1f), glm::vec3(0.25f), glm::vec3(0.5f));
		m_pLightingSystem->addDLight(glm::vec3(1.f, 1.f, 1.f), glm::vec3(0.1f), glm::vec3(0.25f), glm::vec3(0.5f));

		// Positions of the point lights
		m_pLightingSystem->addPLight(glm::vec3(0.f, 50.f, 0.f));
		m_pLightingSystem->addPLight(glm::vec3(0.f, 50.f, -50.f));
		m_pLightingSystem->addPLight(glm::vec3(50.f, 50.f, 0.f));
		m_pLightingSystem->addPLight(glm::vec3(50.f, 50.f, -50.f));

		// Spotlight
		m_pLightingSystem->addSLight();
	}

	void init_camera()
	{
		m_pCamera = new Camera(glm::vec3(0.0f, 50.0f, 50.0f));
		GLFWInputBroadcaster::getInstance().attach(m_pCamera);
	}

	void init_shaders()
	{
		// Build and compile our shader program
		m_pShaderLighting = new Shader(
			"shaders/multiple_lights.vs",
			"shaders/multiple_lights.frag"
		);
		m_pShaderLighting->enable();
		m_vpShaders.push_back(m_pShaderLighting);

		m_pShaderLamps = new Shader(
			"shaders/lamp.vs",
			"shaders/lamp.frag"
		);
		m_pShaderLamps->enable();
		m_vpShaders.push_back(m_pShaderLamps);

		m_pShaderNormals = new Shader(
			"shaders/normals.vs",
			"shaders/normals.frag",
			"shaders/normals.geom"
		);
		m_vpShaders.push_back(m_pShaderNormals);

		m_pShaderExplode = new Shader(
			"shaders/explode.vs",
			"shaders/explode.frag",
			"shaders/explode.geom"
		);
		m_vpShaders.push_back(m_pShaderExplode);

		m_pShaderLines = new Shader(
			"shaders/line.vs",
			"shaders/line.frag"
		);
		m_pShaderLines->enable();
		m_vpShaders.push_back(m_pShaderLines);

		// Get the uniform locations
		m_iViewLocLightingShader = glGetUniformLocation(m_pShaderLighting->Program, "view");
		m_iProjLocLightingShader = glGetUniformLocation(m_pShaderLighting->Program, "projection");
		m_iViewPosLocLightingShader = glGetUniformLocation(m_pShaderLighting->Program, "viewPos");
		m_iShininessLightingShader = glGetUniformLocation(m_pShaderLighting->Program, "material.shininess");
	}

	void generateModels()
	{
		if (!m_pGround)
		{
			m_pGround = new Ground(500.f, 500.f, 10.f, m_pPhysicsSystem->getDynamicsWorld());
		}

		Slatissima *slat;
		unsigned int nSlats = 4u;
		float spaceBetween = 7.5f;

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

			glm::vec3 pos(-(nSlats * spaceBetween / 2) + i * spaceBetween, 0.f, 0.f);
			glm::mat3 rot(glm::angleAxis(glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f)));

			slat = new Slatissima(0.f, pos, rot, m_pPhysicsSystem->getSoftDynamicsWorld());

			slat->pinToBody(0.f, slat->getWidth() * 0.2f, 1.f, 1.f, m_pGround->getRigidBody());

			GLFWInputBroadcaster::getInstance().attach(slat);

			slats.push_back(slat);
		}

		AnchorPoint *a1 = new AnchorPoint(glm::vec3(0.f, 20.f, 0.f), m_pPhysicsSystem->getDynamicsWorld());

		slats[0]->pinToBody(0.f, slats[0]->getWidth() * 0.2f, 1.f, 1.f, a1->getRigidBody());
	}

	void generateQuad1Models()
	{
		if (!m_pGround)
		{
			m_pGround = new Ground(500.f, 500.f, 10.f, m_pPhysicsSystem->getDynamicsWorld());
		}

		Slatissima *slat1, *slat2, *slat3, *slat4, *slat56, *slat7, *slat8;

		if (slats.size() > 0)
		{
			for (auto &s : slats)
			{
				GLFWInputBroadcaster::getInstance().detach(s);
				delete s;
			}
			slats.clear();
		}

		if (anchors.size() > 0)
		{
			for (auto &a : anchors)
				delete a;

			anchors.clear();
		}

		glm::vec3 pos;
		glm::mat3 rot;

		pos = glm::vec3(17.f, 1.f, -33.f);
		rot = glm::mat3(glm::angleAxis(glm::radians(15.f), glm::vec3(0.f, 1.f, 0.f))) * glm::mat3(glm::angleAxis(glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f)));
		slat1 = new Slatissima(0.f
			, 164.f
			, 32.f
			, 5.f
			, pos
			, rot
			, m_pPhysicsSystem->getSoftDynamicsWorld()
		);
		slat1->pinToBody(0.f
			, slat1->getWidth() * 0.01f, 1.f, 1.f
			, m_pGround->getRigidBody()
		);
		GLFWInputBroadcaster::getInstance().attach(slat1);
		slats.push_back(slat1);

		AnchorPoint *anchor = new AnchorPoint(glm::vec3(28.f, 1.f, 0.f), m_pPhysicsSystem->getDynamicsWorld());
		anchors.push_back(anchor);
		//slat1->pinToBody(0.25f, a1->getRigidBody(), 0.5f);
		slat1->pinToBody(28.f, 1.f, 0.f, slat1->getWidth() * 0.2f, 1.f, 1.f, anchor->getRigidBody(), 0.5f, true, false, false);

		pos = glm::vec3(40.f, 2.f, -55.f);
		rot = glm::mat3(glm::angleAxis(glm::radians(5.f), glm::vec3(0.f, 1.f, 0.f))) * glm::mat3(glm::angleAxis(glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f)));
		slat2 = new Slatissima(0.f, 147.f, 30.f, 5.f, pos, rot, m_pPhysicsSystem->getSoftDynamicsWorld());
		slat2->pinToBody(0.f, slat2->getWidth() * 0.2f, 1.f, 1.f, m_pGround->getRigidBody());
		GLFWInputBroadcaster::getInstance().attach(slat2);
		slats.push_back(slat2);

		anchor = new AnchorPoint(glm::vec3(41.f, 2.f, -50.f), m_pPhysicsSystem->getDynamicsWorld());
		anchors.push_back(anchor);
		slat2->pinToBody(0.05f, slat2->getWidth() * 0.2f, 1.f, 1.f, anchor->getRigidBody(), 0.5f, true, true, false);

		anchor = new AnchorPoint(glm::vec3(45.f, 2.f, 0.f), m_pPhysicsSystem->getDynamicsWorld());
		anchors.push_back(anchor);
		slat2->pinToBody(0.35f, slat2->getWidth() * 0.2f, 1.f, 1.f, anchor->getRigidBody(), 0.5f, true, false, false);

		//pos = glm::vec3(20.f, 3.f, -38.f);
		//rot = glm::mat3(glm::angleAxis(glm::radians(-45.f), glm::vec3(0.f, 1.f, 0.f))) * glm::mat3(glm::angleAxis(glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f)));
		//slat3 = new Slatissima(0.f, 96.f, 25.f, 5.f, pos, rot, m_pPhysicsSystem->getSoftDynamicsWorld());
		//slat3->anchorBaseToBody(m_pGround->getRigidBody());
		//GLFWInputBroadcaster::getInstance().attach(slat3);
		//slats.push_back(slat3);

		//pos = glm::vec3(32.f, 4.f, -55.f);
		//rot = glm::mat3(glm::angleAxis(glm::radians(-45.f), glm::vec3(0.f, 1.f, 0.f))) * glm::mat3(glm::angleAxis(glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f)));
		//slat4 = new Slatissima(0.f, 158.f, 21.f, 5.f, pos, rot, m_pPhysicsSystem->getSoftDynamicsWorld());
		//slat4->anchorBaseToBody(m_pGround->getRigidBody());
		//GLFWInputBroadcaster::getInstance().attach(slat4);
		//slats.push_back(slat4);

		//pos = glm::vec3(22.f, 5.f, -32.f);
		//rot = glm::mat3(glm::angleAxis(glm::radians(-35.f), glm::vec3(0.f, 1.f, 0.f))) * glm::mat3(glm::angleAxis(glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f)));
		//slat56 = new Slatissima(0.f, 133.f, 24.f, 5.f, pos, rot, m_pPhysicsSystem->getSoftDynamicsWorld());
		//slat56->anchorBaseToBody(m_pGround->getRigidBody());
		//GLFWInputBroadcaster::getInstance().attach(slat56);
		//slats.push_back(slat56);

		//pos = glm::vec3(65.f, 6.f, -51.f);
		//rot = glm::mat3(glm::angleAxis(glm::radians(-35.f), glm::vec3(0.f, 1.f, 0.f))) * glm::mat3(glm::angleAxis(glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f)));
		//slat7 = new Slatissima(0.f, 119.f, 29.f, 5.f, pos, rot, m_pPhysicsSystem->getSoftDynamicsWorld());
		//slat7->anchorBaseToBody(m_pGround->getRigidBody());
		//GLFWInputBroadcaster::getInstance().attach(slat7);
		//slats.push_back(slat7);

		//pos = glm::vec3(-12.f, 7.f, -35.f);
		//rot = glm::mat3(glm::angleAxis(glm::radians(135.f), glm::vec3(0.f, 1.f, 0.f))) * glm::mat3(glm::angleAxis(glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f)));
		//slat8 = new Slatissima(0.f, 165.f, 29.f, 5.f, pos, rot, m_pPhysicsSystem->getSoftDynamicsWorld());
		//slat8->anchorBaseToBody(m_pGround->getRigidBody());
		//GLFWInputBroadcaster::getInstance().attach(slat8);
		//slats.push_back(slat8);

		//AnchorPoint *a1 = new AnchorPoint(glm::vec3(0.f, 20.f, 0.f), m_pPhysicsSystem->getDynamicsWorld());

		//slats[0]->anchorBaseToBody(a1->getRigidBody());
	}

	void generateQuad2Models()
	{
		if (!m_pGround)
		{
			m_pGround = new Ground(500.f, 500.f, 10.f, m_pPhysicsSystem->getDynamicsWorld());
		}

		if (slats.size() > 0)
		{
			for (auto &s : slats)
			{
				GLFWInputBroadcaster::getInstance().detach(s);
				delete s;
			}
			slats.clear();
		}

		if (anchors.size() > 0)
		{
			for (auto &a : anchors)
				delete a;

			anchors.clear();
		}

		std::vector<glm::vec3> anchorPositions;
		glm::vec3 kernel(1.f);

		glm::vec3 pos;

		float modelLength, modelWidth, modelWavinessAmplitude, rotAngle;

		int nModels = 9;

		for (int i = 1; i <= nModels; ++i)
		{
			anchorPositions.clear();

			switch (i) {
			case 1:
				modelLength = 217.f;
				modelWidth = 22.f;
				modelWavinessAmplitude = 5.f;
				pos = glm::vec3(-6.f, i * 3.f, -42.f);
				rotAngle = 110.f;
				anchorPositions.push_back(glm::vec3(15.f, i * 3.f, -50.f));
				break;

			case 2:
				modelLength = 148.f;
				modelWidth = 15.f;
				modelWavinessAmplitude = 5.f;
				pos = glm::vec3(-8.f, i * 3.f, -34.f);
				rotAngle = 115.f;
				anchorPositions.push_back(glm::vec3(25.f, i * 3.f, -50.f));
				break;

			case 3:
				modelLength = 255.f;
				modelWidth = 28.f;
				modelWavinessAmplitude = 5.f;
				pos = glm::vec3(-5.f, i * 3.f, -28.f);
				rotAngle = 118.f;
				anchorPositions.push_back(glm::vec3(35.f, i * 3.f, -50.f));
				break;

			case 4:
				modelLength = 180.f;
				modelWidth = 24.f;
				modelWavinessAmplitude = 5.f;
				pos = glm::vec3(-7.f, i * 3.f, -12.f);
				rotAngle = 125.f;
				anchorPositions.push_back(glm::vec3(48.f, i * 3.f, -50.f));
				break;

			case 5:
				modelLength = 52.f;
				modelWidth = 14.f;
				modelWavinessAmplitude = 5.f;
				pos = glm::vec3(19.f, i * 3.f, -32.f);
				rotAngle = 120.f;
				anchorPositions.push_back(glm::vec3(50.f, i * 3.f, -50.f));
				break;

			case 6:
				modelLength = 194.f;
				modelWidth = 23.f;
				modelWavinessAmplitude = 5.f;
				pos = glm::vec3(-8.f, i * 3.f, -4.f);
				rotAngle = 127.f;
				anchorPositions.push_back(glm::vec3(50.f, i * 3.f, -49.f));
				break;

			case 7:
				modelLength = 124.f;
				modelWidth = 22.f;
				modelWavinessAmplitude = 5.f;
				pos = glm::vec3(0.f, i * 3.f, 7.f);
				rotAngle = 123.f;
				anchorPositions.push_back(glm::vec3(50.f, i * 3.f, -26.f));
				break;

			case 8:
				modelLength = 174.f;
				modelWidth = 22.f;
				modelWavinessAmplitude = 5.f;
				pos = glm::vec3(7.f, i * 3.f, -5.f);
				rotAngle = 103.f;
				anchorPositions.push_back(glm::vec3(50.f, i * 3.f, -15.f));
				break;

			case 9:
				modelLength = 154.f;
				modelWidth = 24.f;
				modelWavinessAmplitude = 5.f;
				pos = glm::vec3(65.f, i * 3.f, -22.f);
				rotAngle = -66.f;
				anchorPositions.push_back(glm::vec3(50.f, i * 3.f, -16.f));
				anchorPositions.push_back(glm::vec3(15.f, i * 3.f, 0.f));
				break;
			}

			glm::mat3 rot = glm::mat3(glm::angleAxis(glm::radians(rotAngle), glm::vec3(0.f, 1.f, 0.f))) * glm::mat3(glm::angleAxis(glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f)));

			Slatissima *slat = new Slatissima(0.f
				, modelLength
				, modelWidth
				, modelWavinessAmplitude
				, pos
				, rot
				, m_pPhysicsSystem->getSoftDynamicsWorld()
			);
			GLFWInputBroadcaster::getInstance().attach(slat);
			slats.push_back(slat);

			// anchor to the ground
			kernel.x = slat->getWidth() * 0.01f;
			slat->pinToBody(0.f
				, kernel
				, m_pGround->getRigidBody()
			);

			// pin model at quadrat boundary			
			kernel.x = slat->getWidth() * 0.2f;
			for (auto &anchorPos : anchorPositions)
			{
				AnchorPoint *anchor = new AnchorPoint(anchorPos, m_pPhysicsSystem->getDynamicsWorld());
				anchors.push_back(anchor);
				slat->pinToBody(anchorPos
					, kernel
					, anchor->getRigidBody()
					, 0.1f
					, true
					, false
					, false
				);
			}
		}
	}


	void generateNewModels()
	{
		if (!m_pGround)
		{
			m_pGround = new Ground(500.f, 500.f, 10.f, m_pPhysicsSystem->getDynamicsWorld());
		}

		if (slats.size() > 0)
		{
			for (auto &s : slats)
			{
				GLFWInputBroadcaster::getInstance().detach(s);
				delete s;
			}
			slats.clear();
		}

		if (anchors.size() > 0)
		{
			for (auto &a : anchors)
				delete a;

			anchors.clear();
		}

		std::vector<glm::vec3> anchorPositions;
		glm::vec3 kernel(1.f);

		glm::vec3 pos;

		float modelLength, modelWidth, modelWavinessAmplitude, rotAngle;

		int nModels = 13;

		for (int i = 1; i <= nModels; ++i)
		{
			anchorPositions.clear();

			switch (i) {
			case 1:
				modelLength = 217.f;
				modelWidth = 22.f;
				modelWavinessAmplitude = 5.f;
				pos = glm::vec3(-6.f, i * 3.f, -42.f);
				rotAngle = 110.f;
				anchorPositions.push_back(glm::vec3(15.f, i * 3.f, -50.f));
				break;

			case 2:
				modelLength = 148.f;
				modelWidth = 15.f;
				modelWavinessAmplitude = 5.f;
				pos = glm::vec3(-8.f, i * 3.f, -34.f);
				rotAngle = 115.f;
				anchorPositions.push_back(glm::vec3(25.f, i * 3.f, -50.f));
				break;

			case 3:
				modelLength = 255.f;
				modelWidth = 28.f;
				modelWavinessAmplitude = 5.f;
				pos = glm::vec3(-5.f, i * 3.f, -28.f);
				rotAngle = 118.f;
				anchorPositions.push_back(glm::vec3(35.f, i * 3.f, -50.f));
				break;

			case 4:
				modelLength = 180.f;
				modelWidth = 24.f;
				modelWavinessAmplitude = 5.f;
				pos = glm::vec3(-7.f, i * 3.f, -12.f);
				rotAngle = 125.f;
				anchorPositions.push_back(glm::vec3(48.f, i * 3.f, -50.f));
				break;

			case 5:
				modelLength = 52.f;
				modelWidth = 14.f;
				modelWavinessAmplitude = 5.f;
				pos = glm::vec3(19.f, i * 3.f, -32.f);
				rotAngle = 120.f;
				anchorPositions.push_back(glm::vec3(50.f, i * 3.f, -50.f));
				break;

			case 6:
				modelLength = 194.f;
				modelWidth = 23.f;
				modelWavinessAmplitude = 5.f;
				pos = glm::vec3(-8.f, i * 3.f, -4.f);
				rotAngle = 127.f;
				anchorPositions.push_back(glm::vec3(50.f, i * 3.f, -49.f));
				break;

			case 7:
				modelLength = 124.f;
				modelWidth = 22.f;
				modelWavinessAmplitude = 5.f;
				pos = glm::vec3(0.f, i * 3.f, 7.f);
				rotAngle = 123.f;
				anchorPositions.push_back(glm::vec3(50.f, i * 3.f, -26.f));
				break;

			case 8:
				modelLength = 174.f;
				modelWidth = 22.f;
				modelWavinessAmplitude = 5.f;
				pos = glm::vec3(7.f, i * 3.f, -5.f);
				rotAngle = 103.f;
				anchorPositions.push_back(glm::vec3(50.f, i * 3.f, -15.f));
				break;

			case 9:
				modelLength = 154.f;
				modelWidth = 24.f;
				modelWavinessAmplitude = 5.f;
				pos = glm::vec3(65.f, i * 3.f, -22.f);
				rotAngle = -66.f;
				anchorPositions.push_back(glm::vec3(50.f, i * 3.f, -16.f));
				//anchorPositions.push_back(glm::vec3(15.f, i * 3.f, 0.f));
				break;

			case 10:
				modelLength = 125.f;
				modelWidth = 20.f;
				modelWavinessAmplitude = 5.f;
				pos = glm::vec3(65.f, 4.f, -22.f);
				rotAngle = -90.f;
				anchorPositions.push_back(glm::vec3(50.f, 4.f, -16.f));
				//anchorPositions.push_back(glm::vec3(15.f, i * 3.f, 0.f));
				break;

			case 11:
				modelLength = 160.f;
				modelWidth = 24.f;
				modelWavinessAmplitude = 6.f;
				pos = glm::vec3(10.f, 1.f, 10.f);
				rotAngle = 180.f;
				anchorPositions.push_back(glm::vec3(10.f, 1.f, 10.f));
				//anchorPositions.push_back(glm::vec3(15.f, i * 3.f, 0.f));
				break;
			
			case 12:
				modelLength = 105.f;
				modelWidth = 17.f;
				modelWavinessAmplitude = 5.f;
				pos = glm::vec3(40.f, 1.f, -45.f);
				rotAngle = 0.f;
				anchorPositions.push_back(glm::vec3(40.f, 1.f, -45.f));
				break;
			
			case 13:
				modelLength = 175.f;
				modelWidth = 30.f;
				modelWavinessAmplitude = 5.f;
				pos = glm::vec3(50.f, 16.5f, 0.f);
				rotAngle = -135.f;
				anchorPositions.push_back(glm::vec3(0.f, 16.5f, -50.f));
				break;
			}

			glm::mat3 rot = glm::mat3(glm::angleAxis(glm::radians(rotAngle), glm::vec3(0.f, 1.f, 0.f))) * glm::mat3(glm::angleAxis(glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f)));

			Slatissima *slat = new Slatissima(0.f
				, modelLength
				, modelWidth
				, modelWavinessAmplitude
				, pos
				, rot
				, m_pPhysicsSystem->getSoftDynamicsWorld()
			);
			GLFWInputBroadcaster::getInstance().attach(slat);
			slats.push_back(slat);

			// anchor to the ground
			kernel.x = slat->getWidth() * 0.01f;
			slat->pinToBody(0.f
				, kernel
				, m_pGround->getRigidBody()
			);

			// pin model at quadrat boundary			
			kernel.x = slat->getWidth() * 0.2f;
			for (auto &anchorPos : anchorPositions)
			{
				AnchorPoint *anchor = new AnchorPoint(anchorPos, m_pPhysicsSystem->getDynamicsWorld());
				anchors.push_back(anchor);
				slat->pinToBody(anchorPos
					, kernel
					, anchor->getRigidBody()
					, 0.1f
					, true
					, false
					, false
				);
			}
		}
	}
};



