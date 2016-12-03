#include "Engine.h"
#include "Factory.h"

#include <algorithm>

Engine::Engine()
	: m_bRunning(false)
{
}

Engine::~Engine()
{
}

void Engine::Run()
{
}

void Engine::Update(float dt)
{
	Factory* f = static_cast<Factory*>(*std::find_if(m_vpSystems.begin(), m_vpSystems.end(), [](System  &s) { return s.GetName().compare("Factory") == 0; }));

	for (auto& sys : m_vpSystems)
		if (f)
			sys->Update(dt, f->GetEntities());
}

void Engine::AddSystem(System *system)
{
	m_vpSystems.push_back(system);
}

void Engine::Shutdown()
{
	for (auto& sys : m_vpSystems)
		delete sys;
}
