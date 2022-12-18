#include "simulation_objects.hpp"
#include "core.hpp"
#include "gamemaster.hpp"
#include <iostream>
#include <memory>
#include <utility>

int main() {
  //Simulation Agents Configuration
  AgentSpawnConfig config_pedestrian;
  config_pedestrian.speed_min = 2;
  config_pedestrian.speed_max = 5;
  config_pedestrian.impatience_time_min = 60;
  config_pedestrian.impatience_time_max = 180;
  config_pedestrian.reflex_min = 0.1;
  config_pedestrian.reflex_max = 4;
  config_pedestrian.rush_ratio_min = 0.25;
  config_pedestrian.rush_ratio_max = 0.5;

  AgentSpawnConfig config_car;
  config_car.speed_min = 9;
  config_car.speed_max = 22;
  config_car.impatience_time_min = 120;
  config_car.impatience_time_max = 360;
  config_car.reflex_min = 1;
  config_car.reflex_max = 8;
  config_car.rush_ratio_min = 0.05;
  config_car.rush_ratio_max = 0.2;

  for(int i = 0; i < 100; i++){
    std::cout << i << std::endl;
    SimulationNode node;
    node.pedestrian_config = config_pedestrian;
    node.car_config = config_car;
    node.Simulate(8, 5, 10,4);
  }

  return 1;
}