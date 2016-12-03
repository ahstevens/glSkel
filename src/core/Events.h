#pragma once

#include <vector>
#include <algorithm>

enum EventID {
	MOUSE_CLICK,
	MOUSE_UNCLICK,
	MOUSE_MOVE,
	MOUSE_SCROLL,
	KEY_PRESS,
	KEY_UNPRESS
};

struct Event
{
	EventID m_ID;
	void* m_pPayload;
};

class Listener
{
public:
	virtual ~Listener() {}
	virtual void ReceiveEvent(Event* event) = 0;
};

class Broadcaster
{
public:
	void Subscribe(Listener *lis)
	{
		m_vpListeners.push_back(lis);
	}

	void Unsubscribe(Listener *lis)
	{
		m_vpListeners.erase(std::remove(m_vpListeners.begin(), m_vpListeners.end(), lis), m_vpListeners.end());
	}

protected:
	virtual void Broadcast(Event* event)
	{
		for (auto obs : m_vpListeners) obs->ReceiveEvent(event);
	}

private:
	std::vector<Listener*> m_vpListeners;
};