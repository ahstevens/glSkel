#pragma once
#include <vector>
#include <algorithm>
#include <iostream>

#include "GLFWInputSystem.h"


GLFWInputSystem::GLFWInputSystem()
{
}

GLFWInputSystem& GLFWInputSystem::getInstance()
{
	static GLFWInputSystem instance;
	return instance;
}

void GLFWInputSystem::init(GLFWwindow * window)
{
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetMouseButtonCallback(window, MouseButtonCallback);
	glfwSetCursorPosCallback(window, MousePositionCallback);
	glfwSetScrollCallback(window, ScrollCallback);

	memset(keys, 0, sizeof keys);
	firstMouse = true;
	lastX = 0;
	lastY = 0;
}

bool GLFWInputSystem::keyPressed(const int glfwKeyCode)
{
	return keys[glfwKeyCode];
}

void GLFWInputSystem::poll()
{
	glfwPollEvents();
}

// Is called whenever a key is pressed/released via GLFW
void GLFWInputSystem::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{	
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
		return;
	}

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			keys[key] = true;
			getInstance().notify(NULL, Observer::KEY_PRESS, &key);
		}
		else if (action == GLFW_RELEASE)
		{
			keys[key] = false;
			getInstance().notify(NULL, Observer::KEY_UNPRESS, &key);
		}
	}
}

void GLFWInputSystem::MouseButtonCallback(GLFWwindow * window, int button, int action, int mods)
{
	if (action == GLFW_PRESS)
		getInstance().notify(NULL, Observer::MOUSE_CLICK, &button);
	else if (action == GLFW_RELEASE)
		getInstance().notify(NULL, Observer::MOUSE_UNCLICK, &button);
}

void GLFWInputSystem::MousePositionCallback(GLFWwindow * window, double xpos, double ypos)
{
	if (getInstance().firstMouse)
	{
		lastX = static_cast<GLfloat>(xpos);
		lastY = static_cast<GLfloat>(ypos);
		firstMouse = false;
	}

	GLfloat xoffset = static_cast<GLfloat>(xpos) - lastX;
	GLfloat yoffset = lastY - static_cast<GLfloat>(ypos);  // Reversed since y-coordinates go from bottom to left

	lastX = static_cast<GLfloat>(xpos);
	lastY = static_cast<GLfloat>(ypos);

	float offset[2] = { static_cast<float>(xoffset), static_cast<float>(yoffset) };

	getInstance().notify(NULL, Observer::MOUSE_MOVE, &offset);
}

void GLFWInputSystem::ScrollCallback(GLFWwindow * window, double xoffset, double yoffset)
{
	float offset = static_cast<float>(yoffset);
	getInstance().notify(NULL, Observer::MOUSE_SCROLL, &offset);
}
