#pragma once 

#include <vector>
#include <memory>

class Time {
	public:
	float time;
	float deltaTime;
};

class CoreObject
{
	public:
	void FixedUpdate(float deltaTime){
	  this->time.time += deltaTime;
	  this->time.deltaTime = deltaTime;
	}
	void Yeet(){
	  this->yeet_flag = true;
	}
	bool isYeeted(){
	  return this->yeet_flag;
	}
	float GetTime(){
	  return this->time.time;
	}
	float DeltaTime(){
	  return this->time.deltaTime;
	}

	private:
	Time time;
	bool yeet_flag;
};

class SimulationObject : public CoreObject {
	public:
	virtual void Update() = 0;
	virtual void Start() = 0;
};

class Core
{
	public:
	std::vector<std::shared_ptr<SimulationObject>> objects;

	void Update(float deltaTime){
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
	std::shared_ptr<T> Instantiate(std::shared_ptr<T> object){
	  objects.push_back(object);
	  (*objects.back()).Start();

	  return object;
	}
};




