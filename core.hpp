#pragma once 

#include <vector>
#include <memory>
#include <iostream>

class Time {
	public:
	float time;
	float deltaTime;

	Time(){
		this->time = 0;
		this->deltaTime = 0;
	}
};

class CoreObject
{
	public:
	CoreObject(){
		this->yeet_flag = false;
	}

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
			if(object == nullptr)
				continue;
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

	template<typename T, typename... Args>
	std::shared_ptr<T> Instantiate(Args... args){
		auto sharedPointer = std::make_shared<T>(std::forward<Args>(args)...);
		this->objects.push_back(sharedPointer);
	  (*objects.back()).Start();

	  return sharedPointer;
	}

};




