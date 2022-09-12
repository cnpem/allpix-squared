/**
 * @file
 * @brief Implementation of [DepositionLaser] module
 *
 * @copyright Copyright (c) 2017-2022 CERN and the Allpix Squared authors.
 * This software is distributed under the terms of the MIT License, copied verbatim in the file "LICENSE.md".
 * In applying this license, CERN does not waive the privileges and immunities granted to it by virtue of its status as an
 * Intergovernmental Organization or submit itself to any jurisdiction.
 * SPDX-License-Identifier: MIT
 */

#include "DepositionLaserModule.hpp"

#include <string>
#include <utility>

#include "core/utils/log.h"

using namespace allpix;

DepositionLaserModule::DepositionLaserModule(Configuration& config, Messenger* messenger, GeometryManager* geo_manager)
    : Module(config), geo_manager_(geo_manager), messenger_(messenger) {

    // ... Implement ... (Typically bounds the required messages and optionally sets configuration defaults)
    // Input required by this module

    // Read beam parameters from config

    if(config_.has("source_position")) {
        source_position_ = config_.get<ROOT::Math::XYZPoint>("source_position");
        LOG(DEBUG) << "Source position: " << source_position_;
    } else {
        throw MissingKeyError("source_position", "DepositionLaser");
    }

    if(config_.has("beam_direction")) {
        beam_direction_ = config_.get<ROOT::Math::XYZVector>("beam_direction");
        LOG(DEBUG) << "Beam direction: " << beam_direction_;
    } else {
        throw MissingKeyError("beam_direction", "DepositionLaser");
    }

    config_.setDefault<double>("beam_waist", 0.02);
    config_.setDefault<int>("photon_number", 1000);

    beam_waist_ = config_.get<double>("beam_waist");
    LOG(DEBUG) << "Beam waist: " << Units::convert(beam_waist_, "um") << " um";
    if(beam_waist_ < 0) {
        throw InvalidValueError(config_, "beam_waist", "Beam waist should be a positive value!");
    }

    photon_number_ = config_.get<int>("photon_number");
    LOG(DEBUG) << "Photon number: " << photon_number_;
    if(photon_number_ <= 0) {
        throw InvalidValueError(config_, "photon_number", "Photon number should be a positive nonzero value!");
    }
}

void DepositionLaserModule::initialize() {

    // Loop over detectors and do something
    std::vector<std::shared_ptr<Detector>> detectors = geo_manager_->getDetectors();
    for(auto& detector : detectors) {
        // Get the detector name
        std::string detectorName = detector->getName();
        LOG(DEBUG) << "Detector with name " << detectorName;
    }
}

void DepositionLaserModule::run(Event* event) {}
