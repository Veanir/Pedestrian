#pragma once

#include "core.hpp"
#include "simulation_objects.hpp"

#include <cmath>
#include <random>
#include <iostream>
#include <algorithm>
#include <thread>
#include <string>

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

	void print(){
		std::cout << "Speed: " << this->speed_min << " - " << this->speed_max << std::endl;
		std::cout << "Reflex: " << this->reflex_min << " - " << this->reflex_max << std::endl;
		std::cout << "Impatience: " << this->impatience_time_min << " - " << this->impatience_time_max << std::endl;
		std::cout << "Rush: " << this->rush_ratio_min << " - " << this->rush_ratio_max << std::endl;
	}
};



template <typename T>
class AgentSpawner : public SimulationObject {
	private:
	Core *core;

	float spawn_period;
	float time_until_next_spawn;

	std::shared_ptr<Light> light;
	std::shared_ptr<Crossing> crossing;

	AgentSpawnConfig config;

	int spawn_count;

	float getRandomFloat(float min, float max){
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<> dis(min, max);
		return dis(gen);
	}

	float getRandomFloatNormalDistribution(float mean, float std_dev){
		std::random_device rd;
		std::mt19937 gen(rd());
		std::normal_distribution<> dis(mean, std_dev);
		return dis(gen);
	}

	void spawnAgent(){
		AgentConfig agent_config;
		agent_config.speed = getRandomFloat(config.speed_min, config.speed_max);
		agent_config.reflex = getRandomFloat(config.reflex_min, config.reflex_max);
		agent_config.impatience_time = getRandomFloat(config.impatience_time_min, config.impatience_time_max);
		agent_config.rush_ratio = getRandomFloat(config.rush_ratio_min, config.rush_ratio_max);

		auto agent = this->core->Instantiate<T>(agent_config, crossing, light);
		this->spawn_count++;
		#ifdef DEBUG
		std::cout << "Spawned agent [" << agent.get() << "]" << std::endl;
		#endif
	}

	public:
	AgentSpawner(Core *core, std::shared_ptr<Light> light, std::shared_ptr<Crossing> crossing) : core(core), light(light), crossing(crossing){
		this->spawn_period = 1;
		this->time_until_next_spawn = 0;
		this->spawn_count = 0;
	}

	void setSpawnRate(int rate){
		this->spawn_period = 60 / rate;
	}

	void setSpawnConfig(AgentSpawnConfig config){
		this->config = config;
	} 

	void Update() override {
		this->time_until_next_spawn -= this->DeltaTime();
		if(this->time_until_next_spawn <= 0){
			this->spawnAgent();
			this->time_until_next_spawn = this->spawn_period + getRandomFloatNormalDistribution(0, this->spawn_period/4);
		}
	}

	void Start() override {}
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
	}
};

void test(){
	std::cout << "Test" << std::endl;
}

class SimulationMaster {
	public:
	std::vector<SimulationNode> nodes;
	std::vector<std::thread> threads;


	AgentSpawnConfig pedestrian_config;
	AgentSpawnConfig car_config;

	int population_size;

	void Simulate(float time, float length, int pedestrian_rate, int car_rate){
		for(auto &node : nodes){
			node.pedestrian_config = this->pedestrian_config;
			node.car_config = this->car_config;
			this->threads.push_back(std::thread(&SimulationNode::Simulate, &node, time, length, pedestrian_rate, car_rate));
		}
		for(auto &thread : threads){
			thread.join();
		}
		this->threads.clear();
	}

	void AddNode(LightConfig pedestrian_light, LightConfig car_light){
		SimulationNode node;
		node.pedestrian_light = pedestrian_light;
		node.car_light = car_light;

		this->nodes.push_back(node);
	}

	void AddNodesInitial(){
		for(int i = 0; i < this->population_size; i++){
			this->AddNode(LightConfig(), LightConfig());
		}
	}

	SimulationMaster(){
		this->population_size = 10;
	}

	SimulationMaster(int population_size){
		this->population_size = population_size;
	}

	void SortByScore(){
		std::sort(this->nodes.begin(), this->nodes.end(), [](SimulationNode &a, SimulationNode &b){
			return a.Score.Score() < b.Score.Score();
		});
	}

	void TakeBestPercent(float percent){
		int count = this->nodes.size() * percent;
		this->nodes.erase(this->nodes.begin() + count, this->nodes.end());
	}

	void FillRestWithBest(){
		int count = this->nodes.size();
		for(int i = 0; i < this->population_size - count; i++){
			this->nodes.push_back(this->nodes[i % count]);
		}
	}

	void MutateNodes(float mutation_rate){
		for(auto &node : this->nodes){
			node.pedestrian_light.Mutate(mutation_rate);
			node.car_light.Mutate(mutation_rate);
		}
	}

	void printNodes(){
		std::cout << "Nodes: " << this->nodes.size() << std::endl;
		for(auto &node : this->nodes){
			std::cout << "Score: " << node.Score.Score() << std::endl;
		}
	}

};
