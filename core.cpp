#include "core.hpp"
#include "simulation_objects.hpp"
#include <iostream>
#include <utility>

void Core::Update(float deltaTime)
{
  for(auto iter = this->objects.begin(); iter != this->objects.end(); ++iter){
    //Yeet object if it wants to
    if((*iter)->isYeeted()){
      iter->swap(objects.back());
      objects.pop_back();      
      return;
    }

    //Update object
    (*iter)->FixedUpdate(deltaTime);
    (*iter)->Update();
  }
}

template<typename T>
std::shared_ptr<T> Core::Instantiate(std::shared_ptr<T> &&object)
{
  objects.push_back(std::move(object));
  (*objects.back()).Start();

  return object;
}

void CoreObject::FixedUpdate(float deltaTime){
  this->time.time += deltaTime;
  this->time.deltaTime = deltaTime;
}

float CoreObject::GetTime(){
  return this->time.time;
}

float CoreObject::DeltaTime(){
  return this->time.deltaTime;
}

void CoreObject::Yeet(){
  this->yeet_flag = true;
}

bool CoreObject::isYeeted(){
  return this->yeet_flag;
}


int main(){

  //randomize seed
  srand(static_cast<unsigned>(time(0)));

  Core core;

  LightConfig config;
  config.green_time = 10;
  config.red_time = 10;
  config.yellow_green_time = 3;
  config.yellow_red_time = 3;

  AgentConfig aconfig;
  aconfig.speed = 69;

  auto light = core.Instantiate(std::make_shared<Light>(config)); // Come back here. Pointer jest złego typu  
  auto crossing = core.Instantiate(std::make_shared<Crossing>(new Crossing()));
  auto pedestrian = core.Instantiate(std::make_shared<Pedestrian>(new Pedestrian(aconfig, crossing, light)));

  // for(int i = 0; i < 100; i++)
  //   core.Update(0.5f);

  return 1;
}
