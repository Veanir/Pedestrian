//#define DEBUG

#include "simulation_objects.hpp"
#include "core.hpp"
#include "gamemaster.hpp"
#include "utility.hpp"

#include <iostream>
#include <memory>
#include <utility>

#include <json.hpp>
#include <fstream>

int main() {

  SimulationNode node;

  std::ifstream config_file("config.json");
  if(!config_file.is_open()){
    std::cerr << "Could not open config file" << std::endl;
    return 1;
  }

  nlohmann::json config_data;
  config_file >> config_data;

  SimulationMaster master;

  master.pedestrian_config = parseSpawnConfig(config_data["pedestrian_config"]);
  master.car_config = parseSpawnConfig(config_data["car_config"]);
  master.parseConfig(config_data["simulation_config"]);

  master.AddNodesInitial();

  master.Simulate();

  return 1;
}