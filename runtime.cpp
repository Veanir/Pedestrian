#define DEBUG

#include "simulation_objects.hpp"
#include "core.hpp"
#include "gamemaster.hpp"
#include <iostream>
#include <memory>
#include <utility>

int main() {
  
  //SimulationNode node;

  //node.Simulate(8, 5, 20, 5);

  Core core;

  LightConfig config;

  config.green_time = 10;
  config.yellow_green_time = 5;
  config.red_time = 10;
  config.yellow_red_time = 5;
  config.initial_color = LightColor::YellowGreen;

  LightConfig config2 = config;
  config2.initial_color = LightColor::YellowGreen;

  auto light = core.Instantiate<Light>(config);
  auto light2 = core.Instantiate<Light>(config2);

  auto crossing = core.Instantiate<Crossing>(5);

  auto spawner = core.Instantiate<AgentSpawner<Pedestrian>>(&core, light, crossing);
  spawner->setSpawnRate(20);
  auto spawner2 = core.Instantiate<AgentSpawner<Car>>(&core, light2, crossing);
  spawner2->setSpawnRate(5);

  for(int i = 0; i < 100; i++){
    core.Update(0.5f);
    if((int)core.getTime() % 3 == 0)
      std::cout << "time -> " << core.getTime()<< std::endl;
  }

  return 1;
}