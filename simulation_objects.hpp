#pragma once

#include "core.hpp"
#include "crossing.hpp"
#include <memory>
#include <random>

enum LightColor {
	YellowGreen,
	Green,
	YellowRed,
	Red
};

void mutate(float *x, float ratio){
	std::random_device rd;
	std::mt19937 engine(rd());

	std::uniform_real_distribution<float> dis(-1.0, 1.0);
	*x += *x * ratio * dis(engine);
	*x = *x > 0 ? *x : 0;
}

class LightConfig{
	public:
	float mutation_ratio;

	float yellow_green_time;
	float green_time;
	float yellow_red_time;
	float red_time;

	LightColor initial_color;

	void Mutate(){
		mutate(&this->green_time, this->mutation_ratio);
		mutate(&this->red_time, this->mutation_ratio);
		mutate(&this->yellow_green_time, this->mutation_ratio);
		mutate(&this->yellow_red_time, this->mutation_ratio);
	}

	LightConfig(){
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
};

class Light : public SimulationObject {
	private:
	LightConfig config;
	LightColor color;
	float time_until_next_change;
	void changeColor(){
		if(this->color + 1 > LightColor::Red)
			this->color = LightColor::YellowGreen;
		else
			this->color = static_cast<LightColor>(this->color + 1);
	}
	float getWaitingTime(){
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
	public:
	void Update() override{
		this->time_until_next_change -= this->DeltaTime();

		if(this->time_until_next_change <= 0){
			this->changeColor();
			this->time_until_next_change = this->getWaitingTime();
		}
	}

	void Start() override{};

	void printLightConfig(){
		std::cout << this->config.yellow_green_time << " | ";
		std::cout << this->config.green_time << " | ";
		std::cout << this->config.yellow_red_time << " | ";
		std::cout << this->config.red_time << std::endl;
	}

	LightColor getColor(){
		return this->color;
	}
	Light(LightConfig config){
		this->config = config;
		this->color = config.initial_color;
		this->time_until_next_change = this->getWaitingTime();
	}
};

enum State {
	stationary,
	reflex,
	crossing,
	crossed
};

class AgentConfig {
	public:
	float speed; 
	float reflex;
	float impatience_time;
	float rush_ratio;

	AgentConfig(){
		this->speed = 5;
		this->impatience_time = 60;
		this->reflex = 1;
		this->rush_ratio = 0.1;
	}
};



bool trigger(float probability){
	//TODO IMPLEMENT THIS SHIT
	return false;
}

class Crossing; // Forward declaration

class Agent : public SimulationObject, public std::enable_shared_from_this<Agent>  {
	private:
	State state;

	protected:
	std::shared_ptr<Light> light;
	std::shared_ptr<Crossing> crossing;
	AgentConfig config;
	float time_until_next_change;
	float waiting_time;

	public:
	float getWaitingTime(){
		return this->waiting_time;
	}

	void changeState(){
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

	State getState(){
		return this->state;
	}

	void Update() override{
		this->time_until_next_change -= this->DeltaTime();
		this->config.impatience_time -= this->DeltaTime();
		this->waiting_time += this->DeltaTime();
	}
	void Start() override{};

	Agent(AgentConfig config, std::shared_ptr<Crossing> crossing, std::shared_ptr<Light> light){
		this->state = State::stationary;
		this->time_until_next_change = 0;
	}
	~Agent(){
		this->crossing->addWaitingTime(this->waiting_time);
	}
};

class Pedestrian : public Agent {
	public:
	void Update() override{
		Agent::Update();

		if(this->time_until_next_change <= 0)
			this->changeState();
	}

	void Start() override{
		if(trigger(this->config.rush_ratio))
			this->config.impatience_time = 0;
	}

	Pedestrian(AgentConfig config, std::shared_ptr<Crossing> crossing, std::shared_ptr<Light> light) : Agent(config, crossing, light) {}
};

class Car : public Agent {
	public:
	void Update() override{
		Agent::Update();

		if(this->time_until_next_change <= 0)
			this->changeState();
	}
	void Start() override{
		this->crossing->hookAgent(shared_from_this());
		if(trigger(this->config.rush_ratio))
			this->config.impatience_time = 0;
	}

	Car(AgentConfig config, std::shared_ptr<Crossing> crossing, std::shared_ptr<Light> light) : Agent(config, crossing, light) {}
};