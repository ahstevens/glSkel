#pragma once

#include <map>
#include <string>

#include "Component.h"

class Entity
{
	friend class Factory;
public:	
	void AddComponent(Component* comp);
	Component* GetComponent(std::string id);
	bool HasComponent(std::string id);
	bool IsActive();
	const std::string& GetName() const;

protected:
	std::map<std::string, Component*> m_mapComponents;
	bool m_bActive;
	std::string m_strName;

	// private to enforce use by Factory system only
	Entity();
	~Entity();
};

class EntityCreator
{
public:
	virtual Entity* Create();
	virtual ~EntityCreator();
};