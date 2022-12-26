#include "simulation_objects.hpp"
#include "core.hpp"
#include "gamemaster.hpp"
#include <iostream>
#include <memory>
#include <utility>

int main() {
  Core core;

  auto light = core.Instantiate<Light>(LightConfig());
  auto crossing = core.Instantiate<Crossing>(5);

  // AgentSpawnConfig config;
  // auto spawner = core.Instantiate<AgentSpawner<Pedestrian>>(&core, light, crossing );
  // spawner->setSpawnConfig(config);
  // spawner->setSpawnRate(10);


  for(int i = 0; i < 50; i ++)
  {
    core.Update(0.5f);
    if(i % 10 == 0)
      {
      std::cout << "instantiate" << std::endl;
      core.Instantiate<Pedestrian>(AgentConfig(), crossing, light);
      }
  }
  return 1;
}