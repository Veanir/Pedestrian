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
#include <unistd.h>

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
	int population_size_initial;
	int iterations;
	float mutation_rate_initial;
	float mutation_rate_final;
	float best_percent_initial;
	float best_percent_final;

	int pedestrian_rate;
	int car_rate;
	float time;
	float length;

	int threads;
};

class SimulationScore{
	public:
	float score_best;
	float score_average;
	float score_average_best_percent;
	SimulationNode best_node;
};

class SimulationMaster {
	private:
	int thread_count;
	SimulationScore score;

	void joinThreads(){
		for(auto& thread : this->threads)
			thread.join();
		this->threads.clear();
		this->thread_count = 0;
	}

	public:
	std::string fileName;

	std::vector<SimulationNode> nodes;
	std::vector<std::thread> threads;


	AgentSpawnConfig pedestrian_config;
	AgentSpawnConfig car_config;

	SimulationMasterConfig config;

	void parseConfig(const nlohmann::json& config){
		this->config.population_size = config["population_size"];
		this->config.population_size_initial = config["population_size_initial"];
		this->config.iterations = config["iterations"];
		this->config.mutation_rate_initial = config["mutation_rate_initial"];
		this->config.mutation_rate_final = config["mutation_rate_final"];
		this->config.best_percent_initial = config["best_percent_initial"];
		this->config.best_percent_final = config["best_percent_final"];

		this->config.pedestrian_rate = config["pedestrian_rate"];
		this->config.car_rate = config["car_rate"];
		this->config.time = config["time"];
		this->config.length = config["length"];
		if(config["threads"] != -1)
			this->config.threads = config["threads"];
	}

	void Simulate(){
		std::cout << "Number of threads -> " << this->config.threads << std::endl;

		for(int i = 0; i < this->config.iterations; i++){
			std::cout << "Iteration " << i << "/" << this->config.iterations;
			std::cout << " |m_rate: " << sigmoidal_progress(i, this->config.iterations, this->config.mutation_rate_initial, this->config.mutation_rate_final);
			std::cout << " |b_percent: " << sigmoidal_progress(i, this->config.iterations, this->config.best_percent_initial, this->config.best_percent_final) << std::endl;
			for(auto &node : nodes){
				node.pedestrian_config = this->pedestrian_config;
				node.car_config = this->car_config;
				this->threads.push_back(std::thread(&SimulationNode::Simulate, &node, this->config.time, this->config.length, this->config.pedestrian_rate, this->config.car_rate));

				this->thread_count++;
				if(this->thread_count >= this->config.threads)
					this->joinThreads();
			}

			this->joinThreads();

			this->SortByScore();
			if(this->score.best_node.Score.Score() == 0 || this->score.best_node.Score.Score() > this->nodes[0].Score.Score())
				this->score.best_node = this->nodes[0];
			this->score.score_best = this->getBestScore();
			this->score.score_average = this->getAverageScore();
			this->TakeBestPercent(this->sigmoidal_progress(i, this->config.iterations, this->config.best_percent_initial, this->config.best_percent_final));
			this->score.score_average_best_percent = this->getAverageScore();
			this->FillRestWithBest();
			this->MutateNodes(this->sigmoidal_progress(i, this->config.iterations, this->config.mutation_rate_initial, this->config.mutation_rate_final));

			this->SaveResults();

		}
	}

	double sigmoidal_progress(int iteration, int max_iterations, double initial_rate, double final_rate) {
    	double x = (double)(max_iterations - iteration) / max_iterations;
    	return final_rate + (initial_rate - final_rate) / (1 + exp(-4 * x + 2));
	}

	void SaveResults(){
		std::ifstream in(this->fileName);
		nlohmann::json json;
		in >> json;
		in.close();

		json["score_best"].push_back(this->score.score_best);
		json["score_avg"].push_back(this->score.score_average);
		json["score_avg_best_percent"].push_back(this->score.score_average_best_percent);
		json["best_node"] = this->score.best_node.to_json();

		std::ofstream out(this->fileName);
		out << json.dump(4);
		out.close();
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
		for(int i = 0; i < this->config.population_size_initial; i++){
			this->AddNode(LightConfig(), LightConfig());
		}
	}

	SimulationMaster(){
		this->config.population_size = 10;
		this->config.threads = std::thread::hardware_concurrency() == 0 ? 1 : std::thread::hardware_concurrency();
		this->fileName = "simulation_results.json";

		this->InitResultFile();
	}

	SimulationMaster(int population_size){
		this->config.population_size = population_size;
		this->config.threads = std::thread::hardware_concurrency() == 0 ? 1 : std::thread::hardware_concurrency();
		this->fileName = "simulation_results.json";

		this->InitResultFile();
	}

	void InitResultFile(){
		std::ofstream file(this->fileName, std::ios::out | std::ios::trunc);
		
		nlohmann::json json;
		json["score_best"] = nlohmann::json::array();
		json["score_avg"] = nlohmann::json::array();
		json["score_avg_best_percent"] = nlohmann::json::array();
		json["best_node"] = NULL;
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
