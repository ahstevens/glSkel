#pragma once
#include <vector>
#include <algorithm>

#include "GLFWInputBroadcaster.h"


GLFWInputBroadcaster::GLFWInputBroadcaster()
	: firstMouse(false)
	, lastX(0)
	, lastY(0)
{
	memset(keys, 0, sizeof keys);
}

GLFWInputBroadcaster& GLFWInputBroadcaster::getInstance()
{
	static GLFWInputBroadcaster instance;
	return instance;
}

void GLFWInputBroadcaster::init(GLFWwindow * window)
{
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
}

// Is called whenever a key is pressed/released via GLFW
void GLFWInputBroadcaster::key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{	
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

void GLFWInputBroadcaster::mouse_callback(GLFWwindow * window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = static_cast<GLfloat>(xpos);
		lastY = static_cast<GLfloat>(ypos);
		firstMouse = false;
	}

	GLfloat xoffset = static_cast<GLfloat>(xpos) - lastX;
	GLfloat yoffset = lastY - static_cast<GLfloat>(ypos);  // Reversed since y-coordinates go from bottom to left

	lastX = static_cast<GLfloat>(xpos);
	lastY = static_cast<GLfloat>(ypos);

	camera.look(xoffset, yoffset);
}

void GLFWInputBroadcaster::scroll_callback(GLFWwindow * window, double xoffset, double yoffset)
{
	camera.zoom(static_cast<GLfloat>(yoffset));
}
