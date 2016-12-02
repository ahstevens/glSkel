#include "Entity.h"

#include <iostream>

void Entity::AddComponent(Component* comp)
{
	if (m_mapComponents[comp->GetName()] != nullptr)
	{
		std::cerr << "Tried to add component " << comp->GetName() << " to entity " << this->GetName() << " when it already exists!" << std::endl;
		return;
	}

	m_mapComponents[comp->GetName()] = comp;
}

Component* Entity::GetComponent(std::string id)
{
	return m_mapComponents[id];
}

bool Entity::HasComponent(std::string id)
{
	return m_mapComponents[id] != nullptr;
}

bool Entity::IsActive()
{
	return m_bActive;
}

const std::string & Entity::GetName() const
{
	return m_strName;
}

Entity::Entity() 
	: m_bActive(true)
	, m_strName("Default Entity")
{
}

Entity::~Entity()
{
}

Entity* EntityCreator::Create()
{
	return nullptr;
}

EntityCreator::~EntityCreator()
{
}
