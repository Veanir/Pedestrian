#include "simulation_objects.hpp"
#include "core.hpp"
//#include "gamemaster.hpp"
#include <iostream>
#include <memory>
#include <utility>

int main() {
  Core core;

  LightConfig light_config = LightConfig();
  light_config.green_time = 10;
  light_config.red_time = 10;
  light_config.yellow_green_time = 2;
  light_config.yellow_red_time = 2;

  auto light = core.Instantiate<Light>(light_config);
  auto crossing = core.Instantiate<Crossing>(5);

  // AgentSpawnConfig config;
  // auto spawner = core.Instantiate<AgentSpawner<Pedestrian>>(&core, light, crossing );
  // spawner->setSpawnConfig(config);
  // spawner->setSpawnRate(10);

  for(int i = 0; i < 100; i++)
  {
    auto pedestrian = core.Instantiate<Pedestrian>(AgentConfig(), crossing, light);
    crossing->hookAgent(pedestrian);
  }


  return 1;
}