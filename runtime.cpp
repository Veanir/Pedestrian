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

  std::ifstream config_file("config.json");
  if(!config_file.is_open()){
    std::cerr << "Could not open config file" << std::endl;
    return 1;
  }

  nlohmann::json config_data;
  config_file >> config_data;

  SimulationMaster master;
  master.population_size = 1000;

  master.pedestrian_config = parseSpawnConfig(config_data["pedestrian_config"]);
  master.car_config = parseSpawnConfig(config_data["car_config"]);

  master.AddNodesInitial();

  for(int i = 0; i < 100; i++){
    master.Simulate(0.5, 5, 20, 4);

    std::cout << "Generation: " << i << std::endl;

    master.SortByScore();
    master.TakeBestPercent(0.2);
    master.printNodes();
    master.FillRestWithBest();
    master.MutateNodes(0.2);
  }

  return 1;
}