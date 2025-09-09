// Copyright (c) 2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_RPC_BLOCKCHAIN_H
#define BITCOIN_RPC_BLOCKCHAIN_H

class CBlock;
class CBlockIndex;
class UniValue;

// LightningCashr: Hive: If optional argument getHiveDifficulty is true, will return Hive difficulty as close to blockindex or tip as possible.
// If getHiveDifficulty is false, will return PoW difficulty as close to blockindex or tip as possible.
double GetDifficulty(const CBlockIndex* blockindex = nullptr, bool getHiveDifficulty = false);

void RPCNotifyBlockChange(bool ibd, const CBlockIndex *);

UniValue blockToJSON(const CBlock& block, const CBlockIndex* blockindex, bool txDetails = false);

UniValue mempoolInfoToJSON();

UniValue mempoolToJSON(bool fVerbose = false);

UniValue blockheaderToJSON(const CBlockIndex* blockindex);

#endif

