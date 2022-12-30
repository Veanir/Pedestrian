#include "simulation_objects.hpp"
#include "core.hpp"
#include "gamemaster.hpp"
#include <iostream>
#include <memory>
#include <utility>

int main() {
  
  //SimulationNode node;
  //node.Simulate(1, 5, 20, 8);

  Core core;
  auto light = core.Instantiate<Light>(LightConfig());
  auto crossing = core.Instantiate<Crossing>(5);

  auto spawner = core.Instantiate<AgentSpawner<Pedestrian>>(&core, light, crossing);
  spawner->setSpawnRate(20);


  for(int i = 0; i < 2 * 3600; i++){
    core.Update(0.5f);
  }


  return 1;
}