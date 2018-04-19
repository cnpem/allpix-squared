/**
 * @file
 * @brief Implementation of the TrackInfoManager class
 * @copyright Copyright (c) 2018 CERN and the Allpix Squared authors.
 * This software is distributed under the terms of the MIT License, copied verbatim in the file "LICENSE.md".
 * In applying this license, CERN does not waive the privileges and immunities granted to it by virtue of its status as an
 * Intergovernmental Organization or submit itself to any jurisdiction.
 */

#include "TrackInfoManager.hpp"

using namespace allpix;

TrackInfoManager::TrackInfoManager() : counter_(1) {}

std::unique_ptr<TrackInfoG4> TrackInfoManager::makeTrackInfo(const G4Track* const track) {
    auto custom_id = counter_++;
    auto G4ParentID = track->GetParentID();
    auto parent_track_id = G4ParentID == 0 ? G4ParentID : g4_to_custom_id_.at(G4ParentID);
    g4_to_custom_id_[track->GetTrackID()] = custom_id;
    track_id_to_parent_id_[custom_id] = parent_track_id;
    return std::make_unique<TrackInfoG4>(custom_id, parent_track_id, track);
}

void TrackInfoManager::setTrackInfoToBeStored(int track_id) {
    auto element = std::find(to_store_track_ids_.begin(), to_store_track_ids_.end(), track_id);
    // If track id is not present we add it, otherwise skip as we only need each track once
    if(element == to_store_track_ids_.end()) {
        to_store_track_ids_.emplace_back(track_id);
    }
}

void TrackInfoManager::storeTrackInfo(std::unique_ptr<TrackInfoG4> the_track_info) {
    auto track_id = the_track_info->getID();
    auto element = std::find(to_store_track_ids_.begin(), to_store_track_ids_.end(), track_id);
    if(element != to_store_track_ids_.end()) {
        stored_track_infos_.push_back(std::move(the_track_info));
        to_store_track_ids_.erase(element);
    }
}

void TrackInfoManager::resetTrackInfoManager() {
    counter_ = 1;
    stored_tracks_.clear();
    to_store_track_ids_.clear();
    g4_to_custom_id_.clear();
    track_id_to_parent_id_.clear();
    stored_track_infos_.clear();
    stored_track_ids_.clear();
    id_to_track_.clear();
}

void TrackInfoManager::dispatchMessage(Module* module, Messenger* messenger) {
    setAllTrackParents();
    auto mc_track_message = std::make_shared<MCTrackMessage>(std::move(stored_tracks_));
    messenger->dispatchMessage(module, mc_track_message);
}

MCTrack const* TrackInfoManager::findMCTrack(int track_id) const {
    auto it = id_to_track_.find(track_id);
    if(it == id_to_track_.end()) {
        return nullptr;
    } else {
        return &stored_tracks_.at(it->second);
    }
}

void TrackInfoManager::createMCTracks() {
    for(auto& track_info : stored_track_infos_) {
        stored_tracks_.emplace_back(track_info->getStartPoint(),
                                    track_info->getEndPoint(),
                                    track_info->getOriginatingVolumeName(),
                                    track_info->getCreationProcessName(),
                                    track_info->getCreationProcessType(),
                                    track_info->getParticleID(),
                                    track_info->getKineticEnergyInitial(),
                                    track_info->getKineticEnergyFinal(),
                                    track_info->getTotalEnergyInitial(),
                                    track_info->getTotalEnergyFinal());

        id_to_track_[track_info->getID()] = stored_tracks_.size() - 1;
        stored_track_ids_.emplace_back(track_info->getID());
    }
}

void TrackInfoManager::setAllTrackParents() {
    for(size_t ix = 0; ix < stored_track_ids_.size(); ++ix) {
        auto track_id = stored_track_ids_[ix];
        auto parent_id = track_id_to_parent_id_[track_id];
        stored_tracks_[ix].setParent(findMCTrack(parent_id));
    }
}
