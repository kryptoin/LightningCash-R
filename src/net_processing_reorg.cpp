// Copyright (c) 2009-2025 The Bitcoin Core developers 
// Distributed under the MIT software license

#include "net_processing_reorg.h"
#include "validation.h"
#include "chainparams.h"

CReorgBehaviorTracker reorgTracker;

void CReorgBehaviorTracker::TrackReorgAttempt(NodeId peer, int depth) {
    LOCK(cs_reorgTracker);
    
    int64_t now = GetTime();
    auto& data = mapPeerData[peer];
    
    // Reset counters if outside window
    if (now - data.nLastReorgAttempt > REORG_WINDOW) {
        data.nReorgAttempts = 0;
        data.nDeepReorgAttempts = 0;
    }
    
    data.nLastReorgAttempt = now;
    data.nReorgAttempts++;
    
    if (depth > DEEP_REORG_THRESHOLD) {
        data.nDeepReorgAttempts++;
    }
}

bool CReorgBehaviorTracker::ShouldPunishPeer(NodeId peer) {
    LOCK(cs_reorgTracker);
    
    const auto& data = mapPeerData[peer];
    
    // Punish for too many reorg attempts
    if (data.nReorgAttempts > MAX_REORG_ATTEMPTS) {
        return true;
    }
    
    // Punish for deep reorg attempts 
    if (data.nDeepReorgAttempts > 1) {
        return true;
    }
    
    // Punish for excessive chain introspection
    if (data.fIntrospectionPenalty) {
        return true;
    }
    
    return false;
}

void CReorgBehaviorTracker::PunishPeer(NodeId peer) {
    LOCK(cs_reorgTracker);
    
    // Record introspection penalty 
    mapPeerData[peer].fIntrospectionPenalty = true;
    
    // Apply graduated punishment based on severity
    auto& data = mapPeerData[peer];
    int score = REORG_MISBEHAVIOR_SCORE;

    // Increase score for deep reorgs
    if (data.nDeepReorgAttempts > 0) {
        score *= (1 + data.nDeepReorgAttempts);
    }

    // Additional penalty for excessive attempts
    if (data.nReorgAttempts > MAX_REORG_ATTEMPTS) {
        score *= 2;
    }

    // Max penalty for introspection abuse
    if (data.fIntrospectionPenalty) {
        score = std::max(score, 150);
    }

    // Increment misbehavior score with calculated penalty
    Misbehaving(peer, score);
}

void CReorgBehaviorTracker::ResetPeer(NodeId peer) {
    LOCK(cs_reorgTracker);
    mapPeerData.erase(peer);
}