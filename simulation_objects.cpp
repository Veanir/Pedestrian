#include "simulation_objects.hpp"
#include <cstdlib>
#include <iostream>
#include <memory>
#include <random>

 
//Light Config

void mutate(float *x, float ratio){
	std::random_device rd;
	std::mt19937 engine(rd());

	std::uniform_real_distribution<float> dis(-1.0, 1.0);
	*x += *x * ratio * dis(engine);
	*x = *x > 0 ? *x : 0;
}
void LightConfig::Mutate(){

	mutate(&this->green_time, this->mutation_ratio);
	mutate(&this->red_time, this->mutation_ratio);
	mutate(&this->yellow_green_time, this->mutation_ratio);
	mutate(&this->yellow_red_time, this->mutation_ratio);
}

LightConfig::LightConfig(){
	std::random_device rd;
	std::mt19937 engine(rd());

  std::uniform_int_distribution<int> color_distribution(0, 3);
	std::uniform_real_distribution<float> time_distribution(0.0, 180.0);

  this->initial_color =static_cast<LightColor>(color_distribution(engine));
	this->green_time = time_distribution(engine);
	this->yellow_green_time = time_distribution(engine);
	this->red_time = time_distribution(engine);
	this->yellow_red_time = time_distribution(engine);
}

//Light
LightColor Light::getColor(){
	return this->color;
}

Light::Light(LightConfig config){
	this->config = config;
	this->color = config.initial_color;
	this->time_until_next_change = this->getWaitingTime();
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

void Light::printLightConfig(){
	std::cout << this->config.yellow_green_time << " | ";
	std::cout << this->config.green_time << " | ";
	std::cout << this->config.yellow_red_time << " | ";
	std::cout << this->config.red_time << std::endl;
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

AgentConfig::AgentConfig(){
	this->speed = 1;
	this->impatience_time = 60;
	this->reflex = 1;
	this->rush_ratio = 0.1;
}

bool trigger(float probability){
	//TODO IMPLEMENT THIS SHIT
	return false;
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
				return;
			}
			
			if(this->light->getColor() == LightColor::Green){
				this->state = State::reflex;
				this->time_until_next_change = this->config.reflex;
			}
			else
				this->time_until_next_change = 0;
			break;
		case State::reflex:
			if(this->light->getColor() == LightColor::Green){
				this->state = State::crossing;
				this->time_until_next_change = this->crossing->getLength()/this->config.speed;
			}
			else{
				this->state = State::stationary;
				this->time_until_next_change = 0;
			}
			break;
		case State::crossing:
			this->state = State::crossed;
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

	this->state = State::stationary;
	this->time_until_next_change = 0;
}

Agent::~Agent(){
	this->crossing->addWaitingTime(this->waiting_time);
}

// Pedestrian
void Pedestrian::Update(){
	Agent::Update();

	if(this->time_until_next_change <= 0)
		this->changeState();
}

void Pedestrian::Start(){
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

float CrossingScore::Score(){
	return (this->waiting_time/1000) * accident_count;
}

Crossing::Crossing(float length){
	this->length = length;
}


