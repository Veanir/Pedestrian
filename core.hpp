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
		#ifdef DEBUG
		std::cout << "Yeeting object: " << this << std::endl;
		#endif
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
	private:
	float time;

	public:
	std::vector<std::shared_ptr<SimulationObject>> objects;

	void Update(float deltaTime){
		this->time += deltaTime;

		for (int i = this->objects.size() - 1; i >= 0; i--){
			if(this->objects[i]->isYeeted()){
				this->objects.erase(this->objects.begin() + i);
				continue;
			}
			this->objects[i]->FixedUpdate(deltaTime);
			this->objects[i]->Update();

		}
	}


	template <typename T, typename... Args>
	std::shared_ptr<T> Instantiate(Args&&... args){
		auto object = std::make_shared<T>(std::forward<Args>(args)...);
		this->objects.push_back(object);
		object->Start();
		return object;
	}

	float getTime(){
		return this->time;
	}

	Core(){
		this->time = 0;
	}
};




