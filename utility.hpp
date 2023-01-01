#pragma once

#include "gamemaster.hpp"
#include <json.hpp>

AgentSpawnConfig parseSpawnConfig(const nlohmann::json& config){
    AgentSpawnConfig spawn_config;
    spawn_config.speed_min = config["speed_min"];
    spawn_config.speed_max = config["speed_max"];
    spawn_config.reflex_min = config["reflex_min"];
    spawn_config.reflex_max = config["reflex_max"];
    spawn_config.impatience_time_min = config["impatience_time_min"];
    spawn_config.impatience_time_max = config["impatience_time_max"];
    spawn_config.rush_ratio_min = config["rush_ratio_min"];
    spawn_config.rush_ratio_max = config["rush_ratio_max"];
    return spawn_config;
}