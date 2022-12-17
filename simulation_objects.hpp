#pragma once

#include "core.hpp"

enum LightColor {
	YellowGreen,
	Green,
	YellowRed,
	Red
};

class LightConfig{
	public:
	float mutation_ratio;

	float yellow_green_time;
	float green_time;
	float yellow_red_time;
	float red_time;

	LightColor initial_color;

	void Mutate();
};

class Light : public SimulationObject {
	private:
	LightConfig config;
	LightColor color;
	float time_until_next_change;
	void changeColor();
	float getWaitingTime();
	public:
	void Update() override;
	void Start() override;

	LightColor getColor();
	Light(LightConfig config);
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
};

class Crossing;

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
	float getWaitingTime();
	void changeState();

	State getState();

	void Update() override;
	void Start() override;

	Agent(AgentConfig config, std::shared_ptr<Crossing> crossing, std::shared_ptr<Light> light);
	~Agent();
};

class Pedestrian : public Agent {
	public:
	void Update() override;
	void Start() override;

	Pedestrian(AgentConfig config, std::shared_ptr<Crossing> crossing, std::shared_ptr<Light> light) : Agent(config, crossing, light) {}
};

class Car : public Agent {
	public:
	void Update() override;
	void Start() override;

	Car(AgentConfig config, std::shared_ptr<Crossing> crossing, std::shared_ptr<Light> light) : Agent(config, crossing, light) {}
};

class Crossing: public SimulationObject{
	private:
	std::vector<std::shared_ptr<Agent>> agents;

	float length;

	float waiting_time;
	int accident_count;
	int casualties_count;

	void Crash();

	public:
	void Update() override;
	void Start() override;

	float getWaitingTime();
	int getAccidentCount();
	int getCasualtiesCount();
	float getLength();

	template<typename T>
	void hookAgent(std::shared_ptr<T> agent);

	void addWaitingTime(float waiting_time);

	Crossing(float length);
};
