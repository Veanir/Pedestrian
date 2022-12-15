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
		this->changeColor();
		this->time_until_next_change = this->getWaitingTime();
	}
}

//Agent

float randomfInRange(float low, float high){
	return low + static_cast<float> (rand()) / (static_cast<float>(static_cast<float>(RAND_MAX)/(high - low)));
};

bool trigger(float probability){
	return randomfInRange(0,1) > probability;
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
			if(this->light->getColor() == LightColor::Green){
				this->state = State::reflex;
				this->time_until_next_change = this->config.reflex;
			}
			break;
		case State::reflex:
			if(this->light->getColor() == LightColor::Green){
				this->state = State::crossing;
				this->time_until_next_change = 0; // <--- Come back Here
			}
			break;
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
	this->config = config;
	this->crossing = crossing;
	this->light = light;
}

// Pedestrian
void Pedestrian::Update(){

}

void Pedestrian::Start(){

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
		agent->getState();
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




