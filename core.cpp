#include "simulation_objects.hpp"
#include "core.hpp"
#include <iostream>
#include <utility>

void Core::Update(float deltaTime)
{
  for(auto &object:this->objects){
    //Yeet object if it wants to
    if(object->isYeeted()){
      object.reset();
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
std::shared_ptr<T> Core::Instantiate(std::shared_ptr<T> object)
{
  objects.push_back(object);
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

