#include <cassert>
#include <unordered_map>
#include "Level.h"

#define LEVEL_TO_NAME(name) \
    case Level::name:       \
        return #name;

std::string convertLevelToName(Level level) {
	switch(level) {
        LEVEL_TO_NAME(DEBUG)
        LEVEL_TO_NAME(INFO)
        LEVEL_TO_NAME(WARN)
        LEVEL_TO_NAME(ERROR)
        LEVEL_TO_NAME(CRITICAL)
        LEVEL_TO_NAME(NONE)
        LEVEL_TO_NAME(UNKNOWN)  
    }
    return "UNKNOWN";
}

#define NAME_TO_LEVEL(name) \
    { #name, Level::name }

Level convertNameToLevel(const std::string& in) {
    static std::unordered_map<std::string, Level> nameToLevel = {NAME_TO_LEVEL(DEBUG),
                                                                 NAME_TO_LEVEL(INFO),
                                                                 NAME_TO_LEVEL(WARN),
                                                                 NAME_TO_LEVEL(ERROR),
                                                                 NAME_TO_LEVEL(CRITICAL),
                                                                 NAME_TO_LEVEL(NONE),
                                                                 NAME_TO_LEVEL(UNKNOWN)};
    auto it = nameToLevel.find(in);
    if (it != nameToLevel.end()) {
        return it->second;
    }
    return Level::UNKNOWN;
}