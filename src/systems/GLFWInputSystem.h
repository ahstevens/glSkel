#pragma once
#include <vector>
#include <algorithm>

#include <GLFW/glfw3.h>

#include <core/Events.h>

class GLFWInputSystem : public Broadcaster
{
public:
	static GLFWInputSystem& getInstance();

	void init(GLFWwindow* window);

	bool keyPressed(const int glfwKeyCode);

	void poll();

private:
	GLFWInputSystem();

	static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
	static void MouseButtonCallback(GLFWwindow* window,int x, int y, int z);
	static void MousePositionCallback(GLFWwindow* window, double xpos, double ypos);
	static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

	static bool keys[1024];
};
