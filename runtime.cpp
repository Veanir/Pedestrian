#include "simulation_objects.hpp"
#include "core.hpp"
#include <iostream>
#include <utility>

int main(){

  //randomize seed
  srand(static_cast<unsigned>(time(0)));

  Core core;

  core.Instantiate(std::make_shared<Crossing>(5));
  core.Update(1.0f);

  std::cout << "dupa" << std::endl;

  return 1;
}
