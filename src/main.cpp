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

// Standard Headers
#include <cstdio>
#include <cstdlib>

// Our classes
#include "Cube.h"
#include "SLatissima.h"

// Define Some Constants
const int mWidth = 1280;
const int mHeight = 800;

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void do_movement();

// Camera
Camera  camera(glm::vec3(0.0f, 5.0f, 15.0f));
LightingSystem ls;
GLfloat lastX = mWidth / 2.0;
GLfloat lastY = mHeight / 2.0;
bool    keys[1024];

// Deltatime
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame

bool firstMouse = true;
bool showLights = true;
bool showNormals = false;

int main(int argc, char * argv[]) {

    // Load GLFW and Create a Window
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    auto mWindow = glfwCreateWindow(mWidth, mHeight, "Saccharina latissima", nullptr, nullptr);

    // Check for Valid Context
    if (mWindow == nullptr) {
        fprintf(stderr, "Failed to Create OpenGL Context");
        return EXIT_FAILURE;
    }

    // Create Context and Load OpenGL Functions
    glfwMakeContextCurrent(mWindow);

	glfwSetKeyCallback(mWindow, key_callback);
	glfwSetCursorPosCallback(mWindow, mouse_callback);
	glfwSetScrollCallback(mWindow, scroll_callback);

	// GLFW Options
	glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
    glewInit();
    fprintf(stderr, "OpenGL %s\n", glGetString(GL_VERSION));

	// Define the viewport dimensions
	glViewport(0, 0, mWidth, mHeight);

	// OpenGL options
	glEnable(GL_DEPTH_TEST);


	// Build and compile our shader program
	Shader lightingShader("shaders/multiple_lights.vs", "shaders/multiple_lights.frag");
	Shader lampShader("shaders/lamp.vs", "shaders/lamp.frag");
	Shader normalsShader("shaders/normals.vs", "shaders/normals.frag", "shaders/normals.geom");


	// Initialize the lighting system
	// Directional light
	ls.addDLight(glm::vec3(-0.2f, -1.0f, -0.3f));
	// Positions of the point lights
	ls.addPLight(glm::vec3(-5.f, 5.f, -5.f));
	ls.addPLight(glm::vec3( 5.f, 5.f, -5.f));
	ls.addPLight(glm::vec3( 5.f, 5.f,  5.f));
	ls.addPLight(glm::vec3(-5.f, 5.f,  5.f));
	// Spotlight
	ls.addSLight(camera.Position, camera.Front);


	// Example cube objects
	// Cube c;	

	// // Positions all cubes
	// c.positions.push_back(glm::vec3( 0.0f,  0.0f,  5.0f));
	// c.positions.push_back(glm::vec3( 2.0f,  5.0f, -15.0f));
	// c.positions.push_back(glm::vec3(-1.5f, -2.2f, -2.5f));
	// c.positions.push_back(glm::vec3(-3.8f, -2.0f, -12.3f));
	// c.positions.push_back(glm::vec3( 2.4f, -0.4f, -3.5f));
	// c.positions.push_back(glm::vec3(-1.7f,  3.0f, -7.5f));
	// c.positions.push_back(glm::vec3( 1.3f, -2.0f, -2.5f));
	// c.positions.push_back(glm::vec3( 1.5f,  2.0f, -2.5f));
	// c.positions.push_back(glm::vec3( 1.5f,  0.2f, -1.5f));
	// c.positions.push_back(glm::vec3(-1.3f,  1.0f, -1.5f));

	// // Angles of cubes
	// for (GLuint i = 0; i < 10; i++)	c.angles.push_back(20.0f * i);


	Slatissima s(10.f, 2.5f, 0.25f, 0.01f, 2.5f, 1000);

	// Set texture units
	lightingShader.Use();
	glUniform1i(glGetUniformLocation(lightingShader.Program, "material.diffuse"), 0);
	glUniform1i(glGetUniformLocation(lightingShader.Program, "material.specular"), 1);	


    // Main Rendering Loop
    while (glfwWindowShouldClose(mWindow) == false) {
		// Calculate deltatime of current frame
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		
		do_movement();

        // Background Fill Color
        glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use corresponding shader when setting uniforms/drawing objects
		lightingShader.Use();
		GLint viewPosLoc = glGetUniformLocation(lightingShader.Program, "viewPos");
		glUniform3f(viewPosLoc, camera.Position.x, camera.Position.y, camera.Position.z);
		// Set material properties
		glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 32.0f);
		
		ls.sLight.position = camera.Position;
		ls.sLight.direction = camera.Front;

		ls.SetupLighting(lightingShader);

		// Create camera transformations
		glm::mat4 view;
		view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (GLfloat)mWidth / (GLfloat)mHeight, 0.1f, 100.0f);
		// Get the uniform locations
		GLint viewLoc = glGetUniformLocation(lightingShader.Program, "view");
		GLint projLoc = glGetUniformLocation(lightingShader.Program, "projection");
		// Pass the matrices to the shader
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));


		//c.Draw(lightingShader);

		s.Draw(lightingShader);

		if (showNormals)
		{
			normalsShader.Use();
			glUniformMatrix4fv(glGetUniformLocation(normalsShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(glGetUniformLocation(normalsShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
			s.Draw(normalsShader);
		}

		if (showLights)
		{
			lampShader.Use();

			glUniformMatrix4fv(glGetUniformLocation(lampShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(glGetUniformLocation(lampShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

			ls.Draw(lampShader);
		}

        // Flip Buffers and Draw
        glfwSwapBuffers(mWindow);
        glfwPollEvents();
    }   
	
	glfwTerminate();

    return EXIT_SUCCESS;
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key == GLFW_KEY_L && action == GLFW_PRESS)
		showLights = !showLights;
	if (key == GLFW_KEY_1 && action == GLFW_PRESS)
		ls.dLight.on = !ls.dLight.on;
	if (key == GLFW_KEY_2 && action == GLFW_PRESS)
		ls.pLights[0].on = !ls.pLights[0].on;
	if (key == GLFW_KEY_3 && action == GLFW_PRESS)
		ls.pLights[1].on = !ls.pLights[1].on;
	if (key == GLFW_KEY_4 && action == GLFW_PRESS)
		ls.pLights[2].on = !ls.pLights[2].on;
	if (key == GLFW_KEY_5 && action == GLFW_PRESS)
		ls.pLights[3].on = !ls.pLights[3].on;
	if (key == GLFW_KEY_6 && action == GLFW_PRESS)
		ls.sLight.on = !ls.sLight.on;
	if (key == GLFW_KEY_N && action == GLFW_PRESS)
		showNormals = !showNormals;
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
	}
}

void do_movement()
{
	// Camera controls
	if (keys[GLFW_KEY_W])
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (keys[GLFW_KEY_S])
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (keys[GLFW_KEY_A])
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (keys[GLFW_KEY_D])
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos;  // Reversed since y-coordinates go from bottom to left

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}