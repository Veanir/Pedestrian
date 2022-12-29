#pragma once

#include "simulation_objects.hpp"
#include "core.hpp"
#include <memory>
#include <vector>

class Agent;
class Pedestrian;
class Car;


class CrossingScore{
	public:
	float waiting_time;
	int accident_count;
	int casualties_count;

	float Score(){
		return (this->waiting_time/1000) * accident_count;
	}
};

class Crossing: public SimulationObject{
	private:
	std::vector<std::shared_ptr<Agent>> agents;

	float length;

	CrossingScore score;

	void Crash(){
		this->score.accident_count++;
		for(auto &agent: this->agents){
			if(std::dynamic_pointer_cast<Pedestrian>(agent) && agent->getState() == State::crossing){
				this->score.casualties_count++;
				agent->Yeet();
			}
		}
	}

	public:
	void Update() override{
		bool car_crossing = false;
		bool pedestrian_crossing = false;
		for(auto &agent:this->agents){
			if(agent -> getState() == State::crossed){
				std::swap(agent, this->agents.back());
				this->agents.pop_back();
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
	void Start() override {}

	CrossingScore getScore(){
		return this->score;
	}

	float getLength(){
		return this->length;
	}

	template<typename T>
	void hookAgent(std::shared_ptr<T> agent){
		this->agents.push_back(agent);
	}

	void addWaitingTime(float waiting_time){
		this->score.waiting_time += waiting_time;
	}

	Crossing(float length) : length(length){
		this->score.waiting_time = 0;
		this->score.accident_count = 0;
		this->score.casualties_count = 0;
	}
};