#pragma once

class Object;

class Observer
{
public:
	virtual ~Observer() {}
	virtual void receiveEvent(Object* obj, const int event, void* data) = 0;
	
	enum EVENT {
		MOUSE_CLICK,
		MOUSE_UNCLICK,
		MOUSE_MOVE,
		MOUSE_SCROLL,
		KEY_PRESS,
		KEY_UNPRESS
	};
};
