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

std::default_random_engine generator;

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
btRigidBody* wall = NULL;

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
    GLFWwindow* mWindow = glfwCreateWindow(mWidth, mHeight, "Saccharina latissima", nullptr, nullptr);

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
	Slatissima *slat;
	unsigned int nSlats = 4u;
	float spaceBetween = 5.f;

	for (int i = 0u; i < nSlats; ++i)
	{
		slat = new Slatissima(0.5f);
		slat->setPosition(glm::vec3(-(nSlats * spaceBetween / 2) + i * spaceBetween, 0.f, 0.f));
		//slat->setOrientation(glm::angleAxis(glm::radians((static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 180.f), glm::vec3(0.f, 1.f, 0.f)));
		slat->setOrientation(glm::angleAxis(glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f)));
		slat->initPhysics(static_cast<btSoftRigidDynamicsWorld*>(dynamicsWorld));
		slat->anchorToBody(groundBody);
		slats.push_back(slat);
	}

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
		glUniform3f(viewPosLoc, camera.getPosition().x, camera.getPosition().y, camera.getPosition().z);
		// Set material properties
		glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 32.0f);
		
		ls.sLight.position = camera.getPosition();
		ls.sLight.direction = glm::vec3(camera.getOrientation()[2]);

		ls.SetupLighting(lightingShader);

		// Create camera transformations
		glm::mat4 view = camera.getViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(camera.getZoom()), (GLfloat)mWidth / (GLfloat)mHeight, 0.01f, 1000.0f);

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
	if (key == GLFW_KEY_R && action == GLFW_PRESS)
	{
		slats.clear();
		Slatissima *slat;
		unsigned int nSlats = 4u;
		float spaceBetween = 5.f;

		for (int i = 0u; i < nSlats; ++i)
		{
			delete slats[i];
			slat = new Slatissima(0.5f);
			slat->setPosition(glm::vec3(-(nSlats * spaceBetween / 2) + i * spaceBetween, 0.f, 0.f));
			//slat->setOrientation(glm::angleAxis(glm::radians((static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 180.f), glm::vec3(0.f, 1.f, 0.f)));
			slat->setOrientation(glm::angleAxis(glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f)));
			slat->initPhysics(static_cast<btSoftRigidDynamicsWorld*>(dynamicsWorld));
			slat->anchorToBody(groundBody);
			slats.push_back(slat);
		}
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
		wall->applyForce(btVector3(1, 0, 0), btVector3(-50, 1, 0));
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
	if (keys[GLFW_KEY_KP_6])
		for (auto s : slats) s->toggleDebugDrawFlag(fDrawFlags::Clusters);

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
		camera.move(FORWARD, deltaTime);
	if (keys[GLFW_KEY_S])
		camera.move(BACKWARD, deltaTime);
	if (keys[GLFW_KEY_A])
		camera.move(LEFT, deltaTime);
	if (keys[GLFW_KEY_D])
		camera.move(RIGHT, deltaTime);

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

	camera.look(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.zoom(static_cast<GLfloat>( yoffset ));
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

	if (0)
	{
		btCollisionShape* shape = new btBoxShape(btVector3(1.f, 1.f, 1.f));
		float mass = 0.f;
		btVector3 localInertia(0.f, 0.f, 0.f);

		btTransform trans;
		trans.setIdentity();
		btVector3 worldPos(-15, 1, 0);
		trans.setOrigin(worldPos);

		btTransform frameInA, frameInB;
		frameInA = btTransform::getIdentity();
		frameInB = btTransform::getIdentity();

		btDefaultMotionState* pMsA1 = new btDefaultMotionState(trans);
		btRigidBody::btRigidBodyConstructionInfo rbInfoA(mass, pMsA1, shape, localInertia);
		btRigidBody* pRbA1 = new btRigidBody(rbInfoA);
		//	btRigidBody* pRbA1 = createRigidBody(0.f, trans, shape);
		pRbA1->setActivationState(DISABLE_DEACTIVATION);

		// add dynamic rigid body B1
		worldPos.setValue(-10, 1, 0);
		trans.setOrigin(worldPos);
		btDefaultMotionState* pMsB1 = new btDefaultMotionState(trans);
		btRigidBody::btRigidBodyConstructionInfo rbInfoB(mass, pMsB1, shape, localInertia);
		wall = new btRigidBody(rbInfoB);
		//	btRigidBody* pRbB1 = createRigidBody(0.f, trans, shape);
		wall->setCollisionFlags(wall->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
		wall->setActivationState(DISABLE_DEACTIVATION);

		// create slider constraint between A1 and B1 and add it to world

		btSliderConstraint* spSlider1 = new btSliderConstraint(*pRbA1, *wall, frameInA, frameInB, true);
		spSlider1->setLowerLinLimit(-15.0F);
		spSlider1->setUpperLinLimit(10.0F);

		spSlider1->setLowerAngLimit(0.f);
		spSlider1->setUpperAngLimit(0.f);
		
		dynamicsWorld->addConstraint(spSlider1, true);
	}
}

void step_physics()
{
	dynamicsWorld->stepSimulation(1.f / 120.f, 10);

	// update soft mesh vertices
	for (auto s : slats) s->update();

	if (tm) tm->update();	
}