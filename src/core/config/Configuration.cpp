/**
 *  @author Simon Spannagel <simon.spannagel@cern.ch>
 *  @author Koen Wolters <koen.wolters@cern.ch>
 */

#include "Configuration.hpp"

#include <ostream>
#include <string>

#include "exceptions.h"

using namespace allpix;

Configuration::Configuration() : Configuration("") {}

Configuration::Configuration(std::string name) : name_(std::move(name)), config_() {}

bool Configuration::has(const std::string& key) const {
    return config_.find(key) != config_.cend();
}

std::string Configuration::getName() const {
    return name_;
}

std::string Configuration::getText(const std::string& key) const {
    try {
        // NOTE: returning literally including ""
        return config_.at(key);
    } catch(std::out_of_range& e) {
        throw MissingKeyError(getName(), key);
    }
}
std::string Configuration::getText(const std::string& key, const std::string& def) const {
    if(!has(key)) {
        return def;
    }
    return getText(key);
}
void Configuration::setText(const std::string& key, const std::string& val) {
    config_[key] = val;
}

unsigned int Configuration::countSettings() const {
    return static_cast<unsigned int>(config_.size());
}

void Configuration::merge(const Configuration& other) {
    for(auto config_pair : other.config_) {
        // only merge values that do not yet exist
        if(!has(config_pair.first)) {
            setText(config_pair.first, config_pair.second);
        }
    }
}

void Configuration::print(std::ostream& out) const {
    for(auto& element : config_) {
        out << element.first << " : " << element.second << std::endl;
    }
}
