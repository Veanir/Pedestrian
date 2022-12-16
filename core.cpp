#include "core.hpp"
#include "simulation_objects.hpp"
#include <iostream>
#include <utility>

void Core::Update(float deltaTime)
{
  for(auto &object:this->objects){
    //Yeet object if it wants to
    if(object->isYeeted()){
      std::swap(this->objects.back(), object);
      objects.pop_back();      
      return;
    }

    //Update object
    object->FixedUpdate(deltaTime);
    object->Update();
  }
}

template<typename T>
std::shared_ptr<T> Core::Instantiate(T* object)
{
  std::shared_ptr<T> pointer = std::make_shared<T>(*object);
  objects.push_back(pointer);
  (*objects.back()).Start();

  return pointer;
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
  aconfig.speed = 2;
  aconfig.reflex = 2;
  aconfig.impatience_time = 120;

  auto light = core.Instantiate(new Light(config));
  core.Update(0.5f);
  core.Update(11.0f);
  auto crossing = core.Instantiate(new Crossing(5));
  auto pedestrian = core.Instantiate(new Pedestrian(aconfig, crossing, light));
  
  for(int i = 0; i < 1000; i++)
    core.Update(0.5f);

  return 1;
}
