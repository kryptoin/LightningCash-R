#include "validation.h"
#include "chain.h"
#include "net_processing_reorg.h"
#include "chainparams.h"

bool IsReorgSafe(const CBlockIndex* pindexTip, const CBlockIndex* pindexFork, NodeId nodeId) {
    AssertLockHeld(cs_main);

    // Check reorg depth against anchor
    int reorgDepth = pindexTip->nHeight - pindexFork->nHeight;
    if (reorgDepth > DEFAULT_REORG_ANCHOR_DEPTH) {
        LogPrint(BCLog::NET, "Deep reorg of %d blocks detected from peer=%d\n", reorgDepth, nodeId);
        reorgTracker.TrackReorgAttempt(nodeId, reorgDepth);
        return false;
    }

    // Track attempt in reorg tracker
    reorgTracker.TrackReorgAttempt(nodeId, reorgDepth);

    // Check if this peer needs punishment
    if (reorgTracker.ShouldPunishPeer(nodeId)) {
        LogPrint(BCLog::NET, "Punishing peer=%d for excessive reorg behavior\n", nodeId);
        reorgTracker.PunishPeer(nodeId);
        return false;
    }

    // Allow reorg if it meets policy requirements
    const bool fAllowDeepReorg = gArgs.GetBoolArg("-allowdeepreorg", false);
    if (!fAllowDeepReorg && reorgDepth > DEFAULT_REORG_ANCHOR_DEPTH) {
        LogPrint(BCLog::NET, "Deep reorg blocked by policy (depth=%d)\n", reorgDepth);
        return false;
    }

    return true;
}