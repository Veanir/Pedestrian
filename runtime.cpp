#include "simulation_objects.hpp"
#include "core.hpp"
#include "gamemaster.hpp"
#include <iostream>
#include <memory>
#include <utility>

int main(){

  //randomize seed
  srand(static_cast<unsigned>(time(0)));

  Core core;

  auto crossing = core.Instantiate<Crossing>(5);

  LightConfig config;
  config.green_time = 10;
  config.red_time = 10;
  config.initial_color = LightColor::Red;

  auto light = core.Instantiate<Light>(config);

  AgentSpawnConfig spawn_config;
  spawn_config.speed_min = 1;
  spawn_config.speed_max = 4;
  spawn_config.impatience_time_min = 60;
  spawn_config.impatience_time_max = 180;
  spawn_config.reflex_min = 0.5;
  spawn_config.reflex_max = 4;
  spawn_config.rush_ratio_min = 0.05;
  spawn_config.rush_ratio_max = 0.4;

  AgentConfig agent_config;
  agent_config.impatience_time = 120;
  agent_config.reflex = 2;
  agent_config.rush_ratio = 0.3;
  agent_config.speed = 2;

  auto spawner = core.Instantiate<AgentSpawner<Pedestrian>>(&core, light, crossing);
  
  spawner->setSpawnRate(10);
  spawner->setSpawnConfig(spawn_config);

  for(int i=0; i < 100; i++)
    core.Update(0.5f);

  std::cout << crossing->getScore().waiting_time << std::endl;



  return 1;
}
