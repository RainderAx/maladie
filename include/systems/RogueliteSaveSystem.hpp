#pragma once

#include "RogueliteSystem.hpp"
#include <string>

namespace RogueliteSaveSystem {
bool saveProfile(const RogueliteProfile& profile, const std::string& file = "sauvegarde/roguelite_profile.txt");
bool loadProfile(RogueliteProfile& profile, const std::string& file = "sauvegarde/roguelite_profile.txt");
bool saveRun(const RogueliteRun& run, const std::string& file = "sauvegarde/roguelite_run.txt");
bool loadRun(RogueliteRun& run, const std::string& file = "sauvegarde/roguelite_run.txt");
}
