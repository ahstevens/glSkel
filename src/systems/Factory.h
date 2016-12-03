#pragma once

#include <string>
#include <vector>
#include <map>

#include "System.h"
#include "Entity.h"

class Factory : public System
{
public:

	virtual void Init();
	virtual void Update(float dt, std::vector<Entity> &entities);
	virtual void SendMessage(Message *msg);

	virtual Entity* CreateEntity(std::string objID);

	std::vector<Entity*>& GetEntities();

private:
	std::vector<Entity*> m_vpEntities;
	std::map<std::string, void*> m_mapCreators;

	Factory();
	virtual ~Factory();

	void RegisterEntityCreators();
};
