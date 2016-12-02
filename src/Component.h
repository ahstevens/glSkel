#pragma once

#include "Message.h"

class Component
{
public:
	Component() : m_bActive(true), m_strName("Default Component") {}
	virtual ~Component() {}

	virtual void Init() = 0;
	virtual void Update(float dt) = 0;
	virtual void Shutdown() = 0;

	virtual void SendMessage(Message *msg) = 0;

	bool IsActive() const { return m_bActive; }
	const std::string& GetName() const { return m_strName; }

private:
	bool m_bActive;
	std::string m_strName;
};

