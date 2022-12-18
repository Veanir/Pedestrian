#pragma once

#include "core.hpp"
#include "simulation_objects.hpp"

#include <cmath>
#include <random>
#include <iostream>

class AgentSpawnConfig{
	public:
	float speed_min;
	float speed_max;
	float reflex_min;
	float reflex_max;
	float impatience_time_min;
	float impatience_time_max;
	float rush_ratio_min;
	float rush_ratio_max;
};



template <typename T>
class AgentSpawner : public SimulationObject {

	int count;

	AgentSpawnConfig config;

	Core *core;
	std::shared_ptr<Light> light;
	std::shared_ptr<Crossing> crossing;

	std::mt19937 generator;
	float period;
	std::uniform_real_distribution<float> period_distribution;
	float time_until_next_spawn;

	void spawnAgent(){
		AgentConfig config;

		config.speed = this->getRandomNormal(this->config.speed_min, this->config.speed_max);
		config.reflex = this->getRandomNormal(this->config.reflex_min, this->config.reflex_max);
		config.impatience_time = this->getRandomNormal(this->config.impatience_time_min, this->config.impatience_time_max);
		config.rush_ratio = this->getRandomNormal(this->config.rush_ratio_min, this->config.rush_ratio_max);


		this->core->Instantiate(std::make_shared<T>(config, crossing, light));
	}

	float getRandomNormal(float min, float max){
		float mean = (min + max)/2;
		float stddev = (max-min)/2;

		std::normal_distribution<float> dist(mean, stddev);
		return dist(this->generator);
	}

	public:

	void Update() override{		
		this->time_until_next_spawn -= this->DeltaTime();

		if(this->time_until_next_spawn <= 0){
			this->count++;
			this->spawnAgent();
			this->time_until_next_spawn = this->period + period_distribution(generator);
		}
	}

	void Start() override {}

	void setSpawnRate(int spawn_rate){
		this->period = 60/spawn_rate;
		this->period_distribution = std::uniform_real_distribution<float>(-0.5 * this->period, 0.5 * this->period);
	}

	void setSpawnConfig(AgentSpawnConfig config){
		this->config = config;
	}

	AgentSpawner<T>(Core *core, std::shared_ptr<Light> light, std::shared_ptr<Crossing> crossing){
		this->core = core;
		this->light = light;
		this->crossing = crossing;

		std::random_device rd;
		std::mt19937 gen(rd());

		this->generator = gen;
	}

	~AgentSpawner<T>(){
	std::cout << this->count << " " << this->GetTime() << std::endl;
	}
	
};
