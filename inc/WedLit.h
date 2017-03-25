#pragma once

#include "config.h"
#include "spdlog/spdlog.h"
#include <memory>

class NodeController;
class AnimationController;

struct WedLitContext
{
	WedLitContext() : exitReq(false){}
	std::shared_ptr<AnimationController> anim_controller;
	std::shared_ptr<NodeController> node_controller;
	volatile std::atomic<bool> exitReq;
};

extern WedLitContext appContext;

//extern std::shared_ptr<spdlog::sinks::ansicolor_sink> color_sink;

//#define INIT_LOG(NAME) static auto LOG = std::make_shared<spdlog::logger>(#NAME, color_sink);
#define INIT_LOG(NAME) static auto LOG = spdlog::stdout_color_st(#NAME)
#define CRITICAL(TEXT, ...) LOG->critical("{}() " TEXT, __FUNCTION__, ##__VA_ARGS__)
#define ERROR(TEXT, ...) LOG->error("{}() " TEXT, __FUNCTION__, ##__VA_ARGS__)
#define WARN(TEXT, ...) LOG->warn("{}() " TEXT, __FUNCTION__, ##__VA_ARGS__)
#define INFO(TEXT, ...) LOG->info("{}() " TEXT, __FUNCTION__, ##__VA_ARGS__)
#define DEBUG(TEXT, ...) LOG->debug("{}() " TEXT, __FUNCTION__, ##__VA_ARGS__)
#define TRACE(TEXT, ...) LOG->trace("{}() " TEXT, __FUNCTION__, ##__VA_ARGS__)

static constexpr const char* FOLDER_DATA_ROOT = "data/";
constexpr const char* FOLDER_ANIMATIONS = "data/animations/";

constexpr int N_KEYS_ON_INIT = 1000;
//cosntexpr int N_KEYS_ON_UPDATE = 40;
constexpr size_t MAX_KEYS_PER_PACKET_INIT_MODE = 100;
constexpr size_t MAX_KEYS_ON_UPDATE = 160;

constexpr int MAX_SERVER_CONNECTIONS = 1;


