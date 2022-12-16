#include "simulation_objects.hpp"
#include <cstdlib>
#include <iostream>
#include <memory>

 
//Light Config
void LightConfig::Mutate(){
	//Yet to be implemented
}

//Light
LightColor Light::getColor(){
	return this->color;
}

Light::Light(LightConfig config){
	this->config = config;
}

void Light::changeColor(){
	if(this->color + 1 > LightColor::Red)
		this->color = LightColor::YellowGreen;
	else
		this->color = static_cast<LightColor>(this->color + 1);
}

float Light::getWaitingTime(){
	switch(this->getColor()){
		case LightColor::YellowGreen:
			return this->config.yellow_green_time;
		case LightColor::Green:
			return this->config.green_time;
		case LightColor::YellowRed:
			return this->config.yellow_red_time;
		case LightColor::Red:
			return this->config.red_time;
		default:
			return 0;
	}
}

void Light::Start(){
	//blank, but needs to be implemented
}

void Light::Update(){
	this->time_until_next_change -= this->DeltaTime();

	if(this->time_until_next_change <= 0){
		std::cout << "[" << this->GetTime() << "]" << this->getColor() << " -> ";
		this->changeColor();
		this->time_until_next_change = this->getWaitingTime();
		std::cout << this->getColor() << std::endl;
	}
}

//Agent

float randomfInRange(float low, float high){
	return low + static_cast<float> (rand()) / (static_cast<float>(static_cast<float>(RAND_MAX)/(high - low)));
};

bool trigger(float probability){
	return randomfInRange(0,1) < probability;
}

float Agent::getWaitingTime(){
	switch(this->state){
		case State::stationary:
			return this->config.reflex;
		default:
			return 0;
	}
}

void Agent::changeState(){
	switch(static_cast<int>(this->state))
	{
		case State::stationary:
			if(this->config.impatience_time <= 0){
				this->state = State::crossing;
				this->time_until_next_change = this->crossing->getLength()/this->config.speed;
				std::cout << this->GetTime() << " >> 1->3" << std::endl;
				return;
			}
			
			if(this->light->getColor() == LightColor::Green){
				this->state = State::reflex;
				this->time_until_next_change = this->config.reflex;
				std::cout << this->GetTime() << " >> 1->2" << std::endl;
			}
			else
				this->time_until_next_change = 0;
			break;
		case State::reflex:
			if(this->light->getColor() == LightColor::Green){
				this->state = State::crossing;
				this->time_until_next_change = this->crossing->getLength()/this->config.speed;
				std::cout << this->GetTime() << " >> 2->3" << std::endl;
			}
			else{
				this->state = State::stationary;
				this->time_until_next_change = 0;
				std::cout << this->GetTime() << " >> 2->1" << std::endl;
			}
			break;
		case State::crossing:
			this->state = State::crossed;
				std::cout << this->GetTime() << " >> 3->4" << std::endl;
			break;
		case State::crossed:
			this->Yeet();
			return;
	}
}

State Agent::getState(){
	return this->state;
}

void Agent::Update(){
	//blank for now
}

void Agent::Start(){
	//blank for now
}

Agent::Agent(AgentConfig config, std::shared_ptr<Crossing> crossing, std::shared_ptr<Light> light){
	std::cout << "constructing agent" << std::endl;
	this->config = config;
	this->crossing = crossing;
	this->light = light;

	crossing->hookAgent(std::shared_ptr<Agent>(this));
}

// Pedestrian
void Pedestrian::Update(){
	this->time_until_next_change -= this->DeltaTime();
	this->config.impatience_time -= this->DeltaTime();

	if(this->time_until_next_change <= 0)
		this->changeState();
}

void Pedestrian::Start(){
	if(trigger(this->config.rush_ratio))
		this->config.impatience_time = 0;
}

//Car

void Car::Update(){
	std::cout << "Dupa samochodu" << std::endl;
}

void Car::Start(){
	std::cout << "Samochod Start" << std::endl;
}

//Crossing
void Crossing::Update(){
	bool car_crossing = false;
	bool pedestrian_crossing = false;
	for(auto &agent:this->agents){
		std::cout << agent->GetTime()  << std::endl;
		if(agent->getState() != State::crossing){
			std::cout << "Not crossing" << std::endl;
			continue;
			}
		if(dynamic_cast<Pedestrian*>(agent.get()) != nullptr)
			std::cout << "Pedestrian crossing" << std::endl;
		else
			std::cout << "Not pedestrian crossing" << std::endl; 
	}
}

void Crossing::Start(){
	
}

float Crossing::getWaitingTime(){
	return this->waitingTime;
}

int Crossing::getAccidentCount(){
	return this->accidentCount;
}

float Crossing::getLength(){
	return this->length;
}

template<typename T>
void Crossing::hookAgent(std::shared_ptr<T> agent){ 
	this->agents.push_back(agent);
}

Crossing::Crossing(float length){
	this->length = length;
}


