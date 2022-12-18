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

  LightConfig light_config;
  light_config.green_time = 10;
  light_config.red_time = 10;
  light_config.yellow_green_time = 3;
  light_config.yellow_red_time = 3;
  light_config.initial_color = LightColor::YellowRed;

  auto light = core.Instantiate(std::make_shared<Light>(light_config));
  auto crossing = core.Instantiate(std::make_shared<Crossing>(5));

  auto agentSpawner = core.Instantiate(std::make_shared<AgentSpawner<Pedestrian>>(&core,light, crossing ));

  agentSpawner->setSpawnRate(30);

  AgentSpawnConfig spawn_config;
  spawn_config.speed_min = 0.5;
  spawn_config.speed_max = 4;
  spawn_config.impatience_time_min = 60;
  spawn_config.impatience_time_max = 180;
  spawn_config.reflex_min = 0.5;
  spawn_config.reflex_max = 4;
  spawn_config.rush_ratio_min = 0.05;
  spawn_config.rush_ratio_max = 0.5;
  agentSpawner->setSpawnConfig(spawn_config);

  for(int i = 0; i <100; i++)
    core.Update(0.5f);


  return 1;
}
