// Copyright (c) 2009-2025 The Bitcoin Core developers
// Distributed under the MIT software license

#ifndef BITCOIN_NET_PROCESSING_REORG_H
#define BITCOIN_NET_PROCESSING_REORG_H

#include "net.h"
#include "net_processing.h"

/** Track peer behavior related to chain reorgs */
class CReorgBehaviorTracker {
private:
    struct PeerReorgData {
        int64_t nLastReorgAttempt;
        int nReorgAttempts;
        int nDeepReorgAttempts;
        bool fIntrospectionPenalty;
    };
    
    std::map<NodeId, PeerReorgData> mapPeerData;
    CCriticalSection cs_reorgTracker;

public:
    static const int MAX_REORG_ATTEMPTS = 2;  // Max reorg attempts before penalty (reduced from 3)
    static const int REORG_WINDOW = 30 * 60;  // 30 minute window (reduced from 1 hour)
    static const int DEEP_REORG_THRESHOLD = 6; // Blocks considered a deep reorg (reduced from 12)
    static const int64_t REORG_MISBEHAVIOR_SCORE = 100; // Misbehavior score for reorg attempts
    
    void TrackReorgAttempt(NodeId peer, int depth);
    bool ShouldPunishPeer(NodeId peer);
    void PunishPeer(NodeId peer);
    void ResetPeer(NodeId peer);
};

extern CReorgBehaviorTracker reorgTracker;

#endif // BITCOIN_NET_PROCESSING_REORG_H