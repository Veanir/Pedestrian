#pragma once 

#include <vector>
#include <memory>

class Time {
	public:
	float time;
	float deltaTime;
};

class CoreObject
{
	public:
	void FixedUpdate(float deltaTime);
	void Yeet();
	bool isYeeted();
	float GetTime();
	float DeltaTime();

	private:
	Time time;
	bool yeet_flag;
};

class SimulationObject: public CoreObject {
	public:
	virtual void Update() = 0;
	virtual void Start() = 0;
};

class Core
{
	public:
	std::vector<std::shared_ptr<SimulationObject>> objects;

	void Update(float deltaTime);
	template<typename T>
	std::shared_ptr<T> Instantiate(T* object);

	void Cleanup();
};



