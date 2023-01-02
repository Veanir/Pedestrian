#pragma once
#include "simulation_objects.hpp"
#include <cstdlib>
#include <iostream>
#include <memory>
#include <random>
#include <ctime>
#include <json.hpp>
 
//Light Config

float mutateFloat(float x, float ratio){
	std::random_device rd;
	std::mt19937 engine(rd());
	float deviation = x * ratio;
	std::uniform_real_distribution<float> distribution(-deviation, deviation);
	return x + distribution(engine);
}
void LightConfig::Mutate(float mutation_ratio){
	this->green_time = mutateFloat(this->green_time, mutation_ratio);
	this->yellow_green_time = mutateFloat(this->yellow_green_time, mutation_ratio);
	this->red_time = mutateFloat(this->red_time, mutation_ratio);
	this->yellow_red_time = mutateFloat(this->yellow_red_time, mutation_ratio);
}

nlohmann::json LightConfig::to_json(){
	nlohmann::json json;
	json["yellow_green_time"] = this->yellow_green_time;
	json["green_time"] = this->green_time;
	json["yellow_red_time"] = this->yellow_red_time;
	json["red_time"] = this->red_time;
	json["initial_color"] = this->initial_color;
	return json;
}

LightConfig::LightConfig(){
	std::random_device rd;
	std::mt19937 engine(rd());

  std::uniform_int_distribution<int> color_distribution(0, 3);
	std::uniform_real_distribution<float> time_distribution(0.0, 180.0);

  this->initial_color = static_cast<LightColor>(color_distribution(engine));
	this->green_time = time_distribution(engine);
	this->yellow_green_time = time_distribution(engine);
	this->red_time = time_distribution(engine);
	this->yellow_red_time = time_distribution(engine);
}

void LightConfig::print(){
	std::cout << this->yellow_green_time << " | ";
	std::cout << this->green_time << " | ";
	std::cout << this->yellow_red_time << " | ";
	std::cout << this->red_time << std::endl;
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
		#ifdef DEBUG
		std::cout << "Light [" << this << "] zmienil kolor na -> " << this->color << std::endl;
		#endif
	}
}

//Agent

AgentConfig::AgentConfig(){
	this->speed = 1;
	this->impatience_time = 60;
	this->reflex = 1;
	this->rush_ratio = 0;
}

//Function that returns true with probability of x
bool trigger(float probability){
	std::random_device rd;
	std::mt19937 engine(rd());

	std::uniform_real_distribution<float> dis(0.0, 1.0);
	float x = dis(engine);
	return x < probability;
}

float Agent::getWaitingTime(){
	return this->waiting_time;
}

void Agent::printAction(){
	std::cout << "Agent [" << this << "] zmienil stan na -> " <<  this->getState() << std::endl;
}
void Agent::changeState(){
	switch(static_cast<int>(this->state))
	{
		case State::stationary:
			if(this->config.impatience_time <= 0){
				this->state = State::crossing;
				this->time_until_next_change = this->crossing->getLength()/this->config.speed;
				#ifdef DEBUG
				this->printAction();
				#endif
				return;
			}
			
			if(this->light->getColor() == LightColor::Green){
				this->state = State::reflex;
				this->time_until_next_change = this->config.reflex;
				#ifdef DEBUG
				this->printAction();
				#endif
			}
			else
				this->time_until_next_change = 0;
			break;
		case State::reflex:
			if(this->light->getColor() == LightColor::Green){
				this->state = State::crossing;
				this->time_until_next_change = this->crossing->getLength()/this->config.speed;
				#ifdef DEBUG
				this->printAction();
				#endif
			}
			else{
				this->state = State::stationary;
				this->time_until_next_change = 0;
				#ifdef DEBUG
				this->printAction();
				#endif
			}
			break;
		case State::crossing:
			this->state = State::crossed;
			#ifdef DEBUG
			this->printAction();
			#endif
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

Agent::Agent(AgentConfig config, std::shared_ptr<Crossing> crossing, std::shared_ptr<Light> light) : config(config), crossing(crossing), light(light){
	this->state = State::stationary;
	this->time_until_next_change = 0;
	this->waiting_time = 0;
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
	this->crossing->hookAgent(this);
	if(trigger(this->config.rush_ratio))
		this->config.impatience_time = 0;
}

void Pedestrian::printAction(){
	std::cout << "Pedestrian [" << this << "] zmienil stan ma -> " <<  this->getState() << std::endl;
}

//Car

void Car::Update(){
	Agent::Update();

	if(this->time_until_next_change <= 0)
		this->changeState();
}

void Car::Start(){
	this->crossing->hookAgent(this);
	if(trigger(this->config.rush_ratio))
		this->config.impatience_time = 0;
}

void Car::printAction(){
	std::cout << "Car [" << this << "] zmienil stan na -> " <<  this->getState() << std::endl;
}

//Crossing
void Crossing::Update(){
	bool car_crossing = false;
	bool pedestrian_crossing = false;
	for (int i = this->agents.size() - 1; i >= 0; i--){
		if(this->agents[i]->isYeeted()){
			this->agents.erase(this->agents.begin() + i);
			continue;
		}
	}
	for(auto &agent:this->agents){
		if(agent->getState() != State::crossing){
			continue;
			}
		if(dynamic_cast<Pedestrian*>(agent))
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
		if(dynamic_cast<Pedestrian*>(agent) && agent->getState() == State::crossing){
			this->score.casualties_count++;
			agent->Yeet();
		}
	}

	#ifdef DEBUG
	std::cout << "Crossing [" << this << "] zderzenie" << std::endl;
	#endif
}

float Crossing::getLength(){
	return this->length;
}

template<typename T>
void Crossing::hookAgent(T* agent){ 
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

float CrossingScore::Score(){
	return (this->waiting_time/1000) * (accident_count + 1);
}

CrossingScore::CrossingScore(){
	this->waiting_time = 0;
	this->accident_count = 0;
	this->casualties_count = 0;
}

nlohmann::json CrossingScore::to_json(){
	nlohmann::json json;
	json["waiting_time"] = this->waiting_time;
	json["accident_count"] = this->accident_count;
	json["casualties_count"] = this->casualties_count;
	json["score"] = this->Score();
	return json;
}


