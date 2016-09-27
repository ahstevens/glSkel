// System Headers
#define GLEW_STATIC      // use static GLEW libs
#include <GL/glew.h>     // include before GLFW (gl.h)
#include <GLFW/glfw3.h>

// GLM headers
#include <glm/common.hpp>
#include <glm/gtc/type_ptr.hpp>

// Bullet Physics headers
#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h>
#include <bullet/BulletSoftBody/btSoftRigidDynamicsWorld.h>
#include <bullet/BulletSoftBody/btSoftBodyHelpers.h>

// glSkeleton headers
#include <glSkel/shader.h>
#include <glSkel/camera.h>
#include <glSkel/mesh.h>
#include <glSkel/lighting.h>
#include <glSkel/TorusMesh.h>
#include <glSkel/BulletDebugDrawer.h>

// Standard Headers
#include <cstdio>
#include <cstdlib>
#include <random>

// Our classes
#include "Cube.h"
#include "SLatissima.h"
#include "GaborTest.h"

// Define Some Constants
const int mWidth = 1280;
const int mHeight = 800;

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void do_movement();
void init_physics();
void step_physics();

// Camera
Camera  camera(glm::vec3(0.0f, 50.0f, 50.0f));
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
bool explode = false;

std::vector<Slatissima *> slats;
GaborTest *gt = NULL;

btDynamicsWorld* dynamicsWorld = NULL;
btRigidBody* groundBody = NULL;

BulletDebugDrawer* debugDrawer = NULL;

TorusMesh* tm = NULL;

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
	glLineWidth(5.f);

	srand(time(NULL)); // Seed the time

	init_physics();

	//-----stepsimulation_start-----
	//for (int i = 0; i<100; i++)
	//{
	//	dynamicsWorld->stepSimulation(1.f / 60.f, 10);

	//	//print positions of all objects
	//	for (int j = dynamicsWorld->getNumCollisionObjects() - 1; j >= 0; j--)
	//	{
	//		btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[j];
	//		btRigidBody* body = btRigidBody::upcast(obj);
	//		btTransform trans;
	//		if (body && body->getMotionState())
	//			body->getMotionState()->getWorldTransform(trans);
	//		else
	//			trans = obj->getWorldTransform();

	//		std::cout << "world pos object " << j << " = " << float(trans.getOrigin().getX()) << "," << float(trans.getOrigin().getY()) << "," << float(trans.getOrigin().getZ()) << std::endl;
	//	}
	//}


	// Build and compile our shader program
	Shader lightingShader("shaders/multiple_lights.vs", "shaders/multiple_lights.frag");
	Shader lampShader("shaders/lamp.vs", "shaders/lamp.frag");
	Shader normalsShader("shaders/normals.vs", "shaders/normals.frag", "shaders/normals.geom");
	Shader explodeShader("shaders/explode.vs", "shaders/explode.frag", "shaders/explode.geom");
	Shader lineShader("shaders/line.vs", "shaders/line.frag");


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
	ls.addSLight(camera.Position, camera.Front);
	//ls.sLight.on = false;

	//gabs.push_back(currentEditGabor);
	Slatissima *slat;
	slat = new Slatissima(80.f, 10.f, 4.f, 0.f);
	slat->setPosition(glm::vec3(10.f, 0.f, -10.f));
	slat->setOrientation(glm::angleAxis(glm::radians((static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 180.f), glm::vec3(0.f, 1.f, 0.f)));
	slat->initPhysics(static_cast<btSoftRigidDynamicsWorld*>(dynamicsWorld));
	slat->anchorToBody(groundBody);
	slats.push_back(slat);

	slat = new Slatissima(120.f, 20.f, 7.f, 0.f);
	slat->setPosition(glm::vec3(-10.f, 0.f, -12.5));
	slat->setOrientation(glm::angleAxis(glm::radians((static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 180.f), glm::vec3(0.f, 1.f, 0.f)));
	slat->initPhysics(static_cast<btSoftRigidDynamicsWorld*>(dynamicsWorld));
	slat->anchorToBody(groundBody);
	slats.push_back(slat);

	slat = new Slatissima(100.f, 30.f, 7.f, 0.f);
	slat->setPosition(glm::vec3(-5.f, 0.f, 10.f));
	slat->setOrientation(glm::angleAxis(glm::radians((static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 180.f), glm::vec3(0.f, 1.f, 0.f)));
	slat->initPhysics(static_cast<btSoftRigidDynamicsWorld*>(dynamicsWorld));
	slat->anchorToBody(groundBody);
	slats.push_back(slat);

	slat = new Slatissima(140.f, 22.f, 4.f, 0.f);
	slat->setPosition(glm::vec3(10.f, 0.f, 0.f));
	slat->setOrientation(glm::angleAxis(glm::radians((static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 180.f), glm::vec3(0.f, 1.f, 0.f)));
	slat->initPhysics(static_cast<btSoftRigidDynamicsWorld*>(dynamicsWorld));
	slat->anchorToBody(groundBody);
	slats.push_back(slat);

	//tm = new TorusMesh();
	//tm->init(static_cast<btSoftRigidDynamicsWorld*>(dynamicsWorld));	

	//gt = new GaborTest();

    // Main Rendering Loop
    while (glfwWindowShouldClose(mWindow) == false) {
		// Calculate deltatime of current frame
		GLfloat currentFrame = static_cast<GLfloat>( glfwGetTime() );
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		
		do_movement();

		step_physics();

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
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (GLfloat)mWidth / (GLfloat)mHeight, 0.01f, 1000.0f);
		// Get the uniform locations
		GLint viewLoc = glGetUniformLocation(lightingShader.Program, "view");
		GLint projLoc = glGetUniformLocation(lightingShader.Program, "projection");
		// Pass the matrices to the shader
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
		
		//c.Draw(lightingShader);

		for (auto s : slats) s->Draw(lightingShader);

		if (tm) tm->Draw(lightingShader);

		if (showNormals)
		{
			normalsShader.Use();
			glUniformMatrix4fv(glGetUniformLocation(normalsShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(glGetUniformLocation(normalsShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
			for (auto s : slats) s->Draw(normalsShader);
		}

		if (explode)
		{
			explodeShader.Use();
			glUniformMatrix4fv(glGetUniformLocation(explodeShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(glGetUniformLocation(explodeShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
			for (auto s : slats) s->Draw(explodeShader);
		}

		if (showLights)
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
			debugDrawer->Draw(lineShader);
		lineShader.Off();

        // Flip Buffers and Draw
        glfwSwapBuffers(mWindow);
        glfwPollEvents();
    }   

	slats.clear();
	
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
	if (key == GLFW_KEY_B && action == GLFW_PRESS)
		explode = !explode;
	if (key == GLFW_KEY_G && action == GLFW_PRESS)
	{
		delete gt;
		gt = new GaborTest();
	}
	
	if (key == GLFW_KEY_MINUS && action == GLFW_PRESS)
	{
		
	}
	if (key == GLFW_KEY_EQUAL && action == GLFW_PRESS)
	{

	}

	if (key == GLFW_KEY_COMMA && action == GLFW_PRESS)
	{

	}
	if (key == GLFW_KEY_PERIOD && action == GLFW_PRESS)
	{

	}

	if (keys[GLFW_KEY_KP_0])
		for (auto s : slats) s->toggleDebugDrawFlag(fDrawFlags::Std);
	if (keys[GLFW_KEY_KP_1])
		for (auto s : slats) s->toggleDebugDrawFlag(fDrawFlags::Faces);
	if (keys[GLFW_KEY_KP_2])
		for (auto s : slats) s->toggleDebugDrawFlag(fDrawFlags::Nodes);
	if (keys[GLFW_KEY_KP_3])
		for (auto s : slats) s->toggleDebugDrawFlag(fDrawFlags::Links);
	if (keys[GLFW_KEY_KP_4])
		for (auto s : slats) s->toggleDebugDrawFlag(fDrawFlags::Normals);
	if (keys[GLFW_KEY_KP_5])
		for (auto s : slats) s->toggleDebugDrawFlag(fDrawFlags::Contacts);

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

	//if (keys[GLFW_KEY_A])
	//	for (auto s: slats) s->rotateY(-1.f);
	//if (keys[GLFW_KEY_D])
	//	for (auto s : slats) s->rotateY(1.f);
	//if (keys[GLFW_KEY_W])
	//	for (auto s : slats) s->rotateX(-1.f);
	//if (keys[GLFW_KEY_S])
	//	for (auto s : slats) s->rotateX(1.f);
	//if (keys[GLFW_KEY_Q])
	//	for (auto s : slats) s->rotateZ(-1.f);
	//if (keys[GLFW_KEY_E])
	//	for (auto s : slats) s->rotateZ(1.f);
	//if (keys[GLFW_KEY_R])
	//	for (auto s : slats) s->setOrientation();
	if (keys[GLFW_KEY_O])
		for (auto s : slats) s->bump(btVector3(0.f, -1.f, 0.f));
	if (keys[GLFW_KEY_U])
		for (auto s : slats) s->bump(btVector3(0.f, 1.f, 0.f));
	if (keys[GLFW_KEY_I])
		for (auto s : slats) s->bump(btVector3(0.f, 0.f, -1.f));
	if (keys[GLFW_KEY_K])
		slats[0]->bump(btVector3(0.f, 0.f, 1.f));

	if (keys[GLFW_KEY_KP_SUBTRACT])
	{

	}
	if (keys[GLFW_KEY_KP_ADD])
	{

	}

	if (keys[GLFW_KEY_UP])
	{

	}
	if (keys[GLFW_KEY_DOWN])
	{

	}
	if (keys[GLFW_KEY_RIGHT])
	{

	}
	if (keys[GLFW_KEY_LEFT])
	{

	}
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = static_cast<GLfloat>( xpos );
		lastY = static_cast<GLfloat>( ypos );
		firstMouse = false;
	}

	GLfloat xoffset = static_cast<GLfloat>( xpos ) - lastX;
	GLfloat yoffset = lastY - static_cast<GLfloat>( ypos );  // Reversed since y-coordinates go from bottom to left
	
	lastX = static_cast<GLfloat>( xpos );
	lastY = static_cast<GLfloat>( ypos );

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll( static_cast<GLfloat>( yoffset ));
}

void init_physics()
{
	btDefaultCollisionConfiguration* collisionConfiguration = new btSoftBodyRigidBodyCollisionConfiguration();
	btCollisionDispatcher* dispatcher = new	btCollisionDispatcher(collisionConfiguration);

	btVector3 worldAabbMin(-1000,-1000,-1000);
	btVector3 worldAabbMax(1000, 1000, 1000);
	btBroadphaseInterface* broadphase = new btAxisSweep3(worldAabbMin, worldAabbMax, 32766U);

	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver();

	dynamicsWorld = new btSoftRigidDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);

	btSoftBodyWorldInfo &sbInfo = static_cast<btSoftRigidDynamicsWorld*>(dynamicsWorld)->getWorldInfo();
	//sbInfo.m_gravity = btVector3(0.f, 0.f, 0.f);
	//sbInfo.m_gravity = btVector3(0.f, -9.8f, 0.f);
	sbInfo.m_gravity = btVector3(1.f, 3.f, -0.5f);
	sbInfo.m_dispatcher = dispatcher;
	sbInfo.m_broadphase = broadphase;
	sbInfo.m_sparsesdf.Initialize();

	debugDrawer = new BulletDebugDrawer();
	//debugDrawer->setDebugMode(btIDebugDraw::DBG_DrawWireframe);
	debugDrawer->setDebugMode(btIDebugDraw::DBG_DrawWireframe);
	dynamicsWorld->setDebugDrawer(debugDrawer);	

	//-----initialization_end-----
	// GROUND PLANE
	btCollisionShape* groundShape = new btBoxShape(btVector3(500.f, 10.f, 500.f));
	{
		btScalar mass(0.f);
		btVector3 localInertia(0.f, 0.f, 0.f);
		btMatrix3x3 m;
		m.setIdentity();
		btTransform trans(m, btVector3(0.f, -10.f, 0.f));

		//using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState(trans);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, groundShape, localInertia);
		groundBody = new btRigidBody(rbInfo);

		groundBody;
		//add the body to the dynamics world
		dynamicsWorld->addRigidBody(groundBody);
	}
}

void step_physics()
{
	dynamicsWorld->stepSimulation(1.f / 120.f, 10);

	// Despite misleading interface name, this actually just fills up the debug buffer with geometry
	//static_cast<btCollisionWorld*>(dynamicsWorld)->debugDrawWorld();

	// update soft mesh vertices
	for (auto s : slats) s->update();

	if (tm) tm->update();
	
}