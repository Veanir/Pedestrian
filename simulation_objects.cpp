#include "simulation_objects.hpp"
#include <cstdlib>
#include <iostream>
#include <memory>
#include <random>

 
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
	this->color = config.initial_color;
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

bool trigger(float probability){
	std::random_device rd;
	std::mt19937 gen(rd());

	std::bernoulli_distribution dis(probability);
	return dis(gen);
}

float Agent::getWaitingTime(){
	return this->waiting_time;
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
	this->time_until_next_change -= this->DeltaTime();
	this->config.impatience_time -= this->DeltaTime();
	this->waiting_time += this->DeltaTime();
}

void Agent::Start(){
	//blank for now
}

Agent::Agent(AgentConfig config, std::shared_ptr<Crossing> crossing, std::shared_ptr<Light> light){
	this->config = config;
	this->crossing = crossing;
	this->light = light;
}

Agent::~Agent(){
	this->crossing->addWaitingTime(this->waiting_time);
}

// Pedestrian
void Pedestrian::Update(){
	std::cout << "Update " << this << std::endl;
	Agent::Update();

	if(this->time_until_next_change <= 0)
		this->changeState();
}

void Pedestrian::Start(){
	std::cout << "Start " << this << std::endl;
	this->crossing->hookAgent(shared_from_this());
	if(trigger(this->config.rush_ratio))
		this->config.impatience_time = 0;
}

//Car

void Car::Update(){
	Agent::Update();

	if(this->time_until_next_change <= 0)
		this->changeState();
}

void Car::Start(){
	this->crossing->hookAgent(shared_from_this());
	if(trigger(this->config.rush_ratio))
		this->config.impatience_time = 0;
}

//Crossing
void Crossing::Update(){
	bool car_crossing = false;
	bool pedestrian_crossing = false;
	for(auto &agent:this->agents){
		if(agent->isYeeted()){
			std::swap(this->agents.back(),agent);
			agents.pop_back();
			continue;
			}
		if(agent->getState() != State::crossing){
			continue;
			}
		if(std::dynamic_pointer_cast<Pedestrian>(agent))
			pedestrian_crossing = true;
		else
			car_crossing = true;
		if(pedestrian_crossing && car_crossing){
			this->Crash();
			break;
		}
	}
}

void Crossing::Start(){
	//Blank but needs to be implemented	
}

void Crossing::Crash(){
	std::cout << "Crash" << std::endl;
	this->score.accident_count++;
	for(auto &agent: this->agents){
		if(std::dynamic_pointer_cast<Pedestrian>(agent) && agent->getState() == State::crossing){
			this->score.casualties_count++;
			agent->Yeet();
		}
	}
}

float Crossing::getLength(){
	return this->length;
}

template<typename T>
void Crossing::hookAgent(std::shared_ptr<T> agent){ 
	this->agents.push_back(agent);
}

void Crossing::addWaitingTime(float waiting_time){
	this->score.waiting_time += waiting_time;
}

CrossingScore Crossing::getScore(){
	return this->score;
}

Crossing::Crossing(float length){
	this->length = length;
}


