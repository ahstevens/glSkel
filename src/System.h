#pragma once

#include "Message.h"
#include "Entity.h"

class System
{
public:
	System() : m_strSystemName("Default System") {}
	virtual ~System() {}

	virtual void Init() = 0;
	virtual void Update(float dt, std::vector<Entity> &entities) = 0;
	virtual void SendMessage(Message *msg) = 0;

	std::string GetName() const { return m_strSystemName; }
	
protected:
	std::string m_strSystemName;
};
