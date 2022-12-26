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

	AgentSpawnConfig(){
		this->speed_min = 1;
		this->speed_max = 5;
		this->impatience_time_min = 60;
		this->impatience_time_max = 180;
		this->reflex_min = 0.5;
		this->reflex_max = 4;
		this->rush_ratio_min = 0.1;
		this->rush_ratio_max = 0.5;
	}
};



template <typename T>
class AgentSpawner : public SimulationObject {


	AgentSpawnConfig config;

	Core* core;
	std::shared_ptr<Light> light;
	std::shared_ptr<Crossing> crossing;

	std::mt19937 generator;
	float period;
	std::uniform_real_distribution<float> period_distribution;
	float time_until_next_spawn;

	void spawnAgent(){
		AgentConfig config;

		this->core->template Instantiate<T>(config, this->crossing, this->light);
	}

	float getRandomNormal(float min, float max){
		return 1;
	}

	public:
	int count;

	void Update() override{		
		this->time_until_next_spawn -= this->DeltaTime();

		if(this->time_until_next_spawn <= 0){
			this->count++;
			this->spawnAgent();
			this->time_until_next_spawn = this->period + 1;
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

	AgentSpawner<T>(Core* core, std::shared_ptr<Light> light, std::shared_ptr<Crossing> crossing) : core(core), light(light), crossing(crossing){
		std::random_device rd;
		std::mt19937 gen(rd());

		this->generator = gen;
		this->time_until_next_spawn = 0;
	}
	
};

class SimulationNode {
	public:
	LightConfig pedestrian_light;
	LightConfig car_light;
	CrossingScore Score;

	AgentSpawnConfig pedestrian_config;
	AgentSpawnConfig car_config;

	void Simulate(float time, float length, int pedestrian_rate, int car_rate){
		Core core;

		auto crossing = core.Instantiate<Crossing>(length);

		auto light_p = core.Instantiate<Light>(pedestrian_light);
		auto light_c = core.Instantiate<Light>(car_light);

		auto spawner_p = core.Instantiate<AgentSpawner<Pedestrian>>(&core, light_p, crossing);
		spawner_p ->setSpawnConfig(pedestrian_config);
		spawner_p->setSpawnRate(pedestrian_rate);

		auto spawner_c = core.Instantiate<AgentSpawner<Car>>(&core, light_c, crossing);
		spawner_c->setSpawnConfig(car_config);
		spawner_c->setSpawnRate(car_rate);

		int iterations = (time * 3600)/0.5;
		for(int i = 0; i < iterations; i++)
			core.Update(0.5f);

		this->Score = crossing->getScore();
		std::cout << this->Score.Score() << std::endl;
	}
};
