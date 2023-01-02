#pragma once

#include "core.hpp"
#include "simulation_objects.hpp"

#include <cmath>
#include <random>

#include <algorithm>
#include <thread>

#include <iostream>
#include <string>
#include <fstream>
#include <json.hpp>

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

	nlohmann::json to_json(){
		nlohmann::json json;
		json["speed_min"] = this->speed_min;
		json["speed_max"] = this->speed_max;
		json["reflex_min"] = this->reflex_min;
		json["reflex_max"] = this->reflex_max;
		json["impatience_time_min"] = this->impatience_time_min;
		json["impatience_time_max"] = this->impatience_time_max;
		json["rush_ratio_min"] = this->rush_ratio_min;
		json["rush_ratio_max"] = this->rush_ratio_max;
		return json;
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

	nlohmann::json to_json(){
		nlohmann::json json;
		json["pedestrian_light"] = this->pedestrian_light.to_json();
		json["car_light"] = this->car_light.to_json();
		json["pedestrian_config"] = this->pedestrian_config.to_json();
		json["car_config"] = this->car_config.to_json();
		json["score"] = this->Score.to_json();
		return json;
	}

};

class SimulationMasterConfig {
	public:
	int population_size;
	int iterations;
	float mutation_rate_initial;
	float mutation_rate_final;
	float best_percent;

	int pedestrian_rate;
	int car_rate;
	float time;
	float length;
};

class SimulationMaster {
	public:
	std::string fileName;

	std::vector<SimulationNode> nodes;
	std::vector<std::thread> threads;


	AgentSpawnConfig pedestrian_config;
	AgentSpawnConfig car_config;

	SimulationMasterConfig config;

	void parseConfig(const nlohmann::json& config){
		this->config.population_size = config["population_size"];
		this->config.iterations = config["iterations"];
		this->config.mutation_rate_initial = config["mutation_rate_initial"];
		this->config.mutation_rate_final = config["mutation_rate_final"];
		this->config.best_percent = config["best_percent"];

		this->config.pedestrian_rate = config["pedestrian_rate"];
		this->config.car_rate = config["car_rate"];
		this->config.time = config["time"];
		this->config.length = config["length"];
	}

	void Simulate(){
		for(int i = 0; i < this->config.iterations; i++){
			std::cout << "Iteration " << i << "/" << this->config.iterations << std::endl;
			for(auto &node : nodes){
				node.pedestrian_config = this->pedestrian_config;
				node.car_config = this->car_config;
				this->threads.push_back(std::thread(&SimulationNode::Simulate, &node, this->config.time, this->config.length, this->config.pedestrian_rate, this->config.car_rate));
			}
			for(auto &thread : threads){
				thread.join();
			}
			this->threads.clear();

			this->ManageResults();

			this->MutateNodes(this->sigmoidal_mutation_rate(i));

		}
	}

	double sigmoidal_mutation_rate(int iteration){
		double x = (double)iteration / this->config.iterations;
    	return this->config.mutation_rate_final + (this->config.mutation_rate_initial - this->config.mutation_rate_final) / (1 + exp(-4 * x + 2));
	}

	//Manage Simulation Nodes and save results to file as json
	void ManageResults(){
		//#Manage Results
		this->SortByScore(); //Sort by score
		this->TakeBestPercent(this->config.best_percent); //Take best percent
		this->FillRestWithBest();
	}

	void AddNode(LightConfig pedestrian_light, LightConfig car_light){
		SimulationNode node;
		node.pedestrian_light = pedestrian_light;
		node.car_light = car_light;

		this->nodes.push_back(node);
	}

	float getAverageScore(){
		float sum = 0;
		for(auto &node : this->nodes){
			sum += node.Score.Score();
		}
		return sum / this->nodes.size();
	}

	float getBestScore(){
		if(this->nodes.size() == 0)
			return 0;
		return this->nodes[0].Score.Score();
	}

	void AddNodesInitial(){
		for(int i = 0; i < this->config.population_size; i++){
			this->AddNode(LightConfig(), LightConfig());
		}
	}

	SimulationMaster(){
		this->config.population_size = 10;
		this->fileName = "simulation_results.json";

		this->InitResultFile();
	}

	SimulationMaster(int population_size){
		this->config.population_size = population_size;
		this->fileName = "simulation_results.json";

		this->InitResultFile();
	}

	void InitResultFile(){
		std::ofstream file(this->fileName, std::ios::out | std::ios::trunc);
		
		nlohmann::json json;
		json["score_best"] = nlohmann::json::array();
		json["score_avg"] = nlohmann::json::array();
		json["score_avg_best_percent"] = nlohmann::json::array();
		json["best_node"] = SimulationNode().to_json();
		file << json;
		file.close();
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
		for(int i = 0; i < this->config.population_size - count; i++){
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
