// System Headers
#define GLEW_STATIC      // use static GLEW libs
#include <GL/glew.h>     // include before GLFW (gl.h)
#include <GLFW/glfw3.h>

// GLM headers
#include <glm/common.hpp>
#include <glm/gtc/type_ptr.hpp>

// glSkeleton headers
#include <glSkel/shader.h>
#include <glSkel/camera.h>
#include <glSkel/mesh.h>
#include <glSkel/lighting.h>
#include <glSkel/TorusMesh.h>

// Standard Headers
#include <cstdio>
#include <cstdlib>
#include <random>

// Our classes
#include <glSkel/Settings.h>
#include "Cube.h"
#include "GaborTest.h"
#include "GLFWInputBroadcaster.h"

std::default_random_engine generator;

GLFWwindow* init_gl_context(std::string winName);

// Camera
Camera  camera(glm::vec3(0.0f, 50.0f, 50.0f));
LightingSystem ls;
Settings settings;

GaborTest *gt = NULL;

TorusMesh* tm = NULL;

int main(int argc, char * argv[]) {

    // Load GLFW and Create a Window
    glfwInit();

	GLFWwindow* mWindow = init_gl_context("Saccharina latissima");
	if (!mWindow) 
	{
		fprintf(stderr, "Failed to Create OpenGL Context");
		return EXIT_FAILURE;
	}

	srand(time(NULL)); // Seed rand with time
	
	GLFWInputBroadcaster::getInstance().init(mWindow);
	GLFWInputBroadcaster::getInstance().attach(&ls);  // Register lighting system with input broadcaster
	GLFWInputBroadcaster::getInstance().attach(&camera);  // Register camera with input broadcaster
	GLFWInputBroadcaster::getInstance().attach(&settings);  // Register settings with input broadcaster

	settings.m_pPhysicsSystem = new PhysicsSystem();
	settings.m_pPhysicsSystem->init();

	// Build and compile our shader program
	Shader lightingShader(
		"shaders/multiple_lights.vs",
		"shaders/multiple_lights.frag"
	);
	Shader lampShader(
		"shaders/lamp.vs",
		"shaders/lamp.frag"
	);
	Shader normalsShader(
		"shaders/normals.vs",
		"shaders/normals.frag",
		"shaders/normals.geom"
	);
	Shader explodeShader(
		"shaders/explode.vs",
		"shaders/explode.frag",
		"shaders/explode.geom"
	);
	Shader lineShader(
		"shaders/line.vs", 
		"shaders/line.frag"
	);

	// Get the uniform locations
	GLint viewLoc = glGetUniformLocation(lightingShader.Program, "view");
	GLint projLoc = glGetUniformLocation(lightingShader.Program, "projection");
	GLint viewPosLoc = glGetUniformLocation(lightingShader.Program, "viewPos");

	// Initialize the lighting system
	// Directional light
	ls.addDLight(glm::vec3(-1.f, -1.f, -1.f), glm::vec3(0.05f), glm::vec3(0.25f));
	//ls.dLight.on = false;
	// Positions of the point lights
	ls.addPLight(glm::vec3(-5.f, 0.f, -5.f));
	ls.addPLight(glm::vec3( 5.f, 0.f, -5.f));
	ls.addPLight(glm::vec3( 5.f, 0.f,  5.f));
	ls.addPLight(glm::vec3(-5.f, 0.f,  5.f));
	// Spotlight
	ls.addSLight(camera.getPosition(), glm::vec3(camera.getOrientation()[2]));
	//ls.sLight.on = false;

	//gabs.push_back(currentEditGabor);
	settings.generateModels();

    // Main Rendering Loop
    while (glfwWindowShouldClose(mWindow) == false) {
		// Calculate deltatime of current frame
		GLfloat currentFrame = static_cast<GLfloat>( glfwGetTime() );
		settings.m_fDeltaTime = currentFrame - settings.m_fLastFrame;
		settings.m_fLastFrame = currentFrame;

		// Poll input events
		GLFWInputBroadcaster::getInstance().update();
		
		camera.update(settings.m_fDeltaTime);
		
		settings.m_pPhysicsSystem->update();

		// update soft mesh vertices
		for (auto s : settings.slats) s->update();

		if (tm) tm->update();

        // Background Fill Color
        glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use corresponding shader when setting uniforms/drawing objects
		lightingShader.Use();
		glUniform3f(viewPosLoc, camera.getPosition().x, camera.getPosition().y, camera.getPosition().z);
		// Set material properties
		glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 32.0f);
		
		ls.sLight.position = camera.getPosition();
		ls.sLight.direction = glm::vec3(camera.getOrientation()[2]);

		ls.SetupLighting(lightingShader);

		// Create camera transformations
		glm::mat4 view = camera.getViewMatrix();
		glm::mat4 projection = glm::perspective(
			glm::radians(camera.getZoom()),
			static_cast<float>(settings.m_iWidth) / static_cast<float>(settings.m_iHeight),
			0.01f,
			1000.0f
			);

		// Pass the matrices to the shader
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
		
		//c.Draw(lightingShader);

		for (auto s : settings.slats) s->Draw(lightingShader);

		if (tm) tm->Draw(lightingShader);

		if (settings.m_bShowNormals)
		{
			normalsShader.Use();
			glUniformMatrix4fv(glGetUniformLocation(normalsShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(glGetUniformLocation(normalsShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
			for (auto s : settings.slats) s->Draw(normalsShader);
		}

		if (settings.m_bExplode)
		{
			explodeShader.Use();
			glUniformMatrix4fv(glGetUniformLocation(explodeShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(glGetUniformLocation(explodeShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
			for (auto s : settings.slats) s->Draw(explodeShader);
		}

		if (settings.m_bShowLights)
		{
			lampShader.Use();

			glUniformMatrix4fv(glGetUniformLocation(lampShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(glGetUniformLocation(lampShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

			ls.Draw(lampShader);
		}

		lineShader.Use();
			glUniformMatrix4fv(glGetUniformLocation(lineShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(glGetUniformLocation(lineShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
			if (gt) gt->Draw(lineShader);
			settings.m_pPhysicsSystem->getDebugDrawer()->Draw(lineShader);
		lineShader.Off();

        // Flip Buffers and Draw
        glfwSwapBuffers(mWindow);
    }   

	settings.slats.clear();
	
	glfwTerminate();

    return EXIT_SUCCESS;
}

GLFWwindow* init_gl_context(std::string winName)
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	GLFWwindow* mWindow = glfwCreateWindow(settings.m_iWidth, settings.m_iHeight, winName.c_str(), nullptr, nullptr);

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
	glViewport(0, 0, settings.m_iWidth, settings.m_iHeight);

	// OpenGL options
	glEnable(GL_DEPTH_TEST);
	glLineWidth(5.f);

	return mWindow;
}