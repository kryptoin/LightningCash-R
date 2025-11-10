#ifndef BITCOIN_VALIDATION_REORG_H
#define BITCOIN_VALIDATION_REORG_H

#include "chain.h"
#include "net.h"

/** Check if a proposed reorg should be allowed based on policy and peer behavior */
bool IsReorgSafe(const CBlockIndex* pindexTip, const CBlockIndex* pindexFork, NodeId nodeId);

#endif