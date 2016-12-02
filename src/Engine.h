#pragma once

#include <vector>

#include "System.h"

class Engine
{
public:
	Engine();
	~Engine();

	void Run();

	void Update(float dt);

	void AddSystem(System *system);

	void Shutdown();

private:
	std::vector<System*> m_vpSystems;
	bool m_bRunning;
};
