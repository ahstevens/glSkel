#include "Factory.h"

Factory::Factory()	
{
	m_strSystemName = "Factory";
}

Factory::~Factory()
{
}

void Factory::Init()
{
}

void Factory::Update(float dt, std::vector<Entity>& entities)
{
	for (auto& ent : m_vpEntities)
	{
		delete ent;
	}
}

void Factory::SendMessage(Message * msg)
{
}

Entity* Factory::CreateEntity(std::string objID)
{
	Entity* newEnt = nullptr;

	newEnt = static_cast<EntityCreator*>(m_mapCreators[objID])->Create();

	m_vpEntities.push_back(newEnt);
}

std::vector<Entity*>& Factory::GetEntities()
{
	return m_vpEntities;
}

void Factory::RegisterEntityCreators()
{

}
