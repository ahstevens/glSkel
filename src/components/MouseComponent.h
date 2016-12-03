#pragma once

#include <core/Component.h>

class MouseComponent : public Component
{
public:


private:
	bool firstMouse;
	float lastX;
	float lastY;
};