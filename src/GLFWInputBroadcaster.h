#pragma once
#include <vector>
#include <algorithm>

#include <glSkel/Subject.h>

#include <GLFW/glfw3.h>

class GLFWInputBroadcaster : public Subject
{
public:
	static GLFWInputBroadcaster& getInstance();

	GLFWInputBroadcaster();

	void init(GLFWwindow* window);

private:
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
	static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	
	bool keys[1024];
	bool firstMouse;
	float lastX, lastY;
};
