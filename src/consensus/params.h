// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2025 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_CONSENSUS_PARAMS_H
#define BITCOIN_CONSENSUS_PARAMS_H

#include <uint256.h>
#include <limits>
#include <map>
#include <string>

#include <script/script.h>  // LightningCashr: Needed for CScript
#include <amount.h>         // LightningCashr: Needed for CAmount

namespace Consensus {

enum DeploymentPos
{
    DEPLOYMENT_TESTDUMMY,
    DEPLOYMENT_CSV, // Deployment of BIP68, BIP112, and BIP113.
    DEPLOYMENT_SEGWIT, // Deployment of BIP141, BIP143, and BIP147.
    DEPLOYMENT_HIVE,    // LightningCashr: Hive: Deployment
    DEPLOYMENT_HIVE_1_1,    // LightningCashr: Hive: 1.1 Deployment
    DEPLOYMENT_HIVE_1_2,    // LightningCashr: Hive: 1.2 Deployment
    // NOTE: Also add new deployments to VersionBitsDeploymentInfo in versionbits.cpp
    MAX_VERSION_BITS_DEPLOYMENTS
};

struct BIP9Deployment {

    int bit;

    int64_t nStartTime;

    int64_t nTimeout;


    static constexpr int64_t NO_TIMEOUT = std::numeric_limits<int64_t>::max();


    static constexpr int64_t ALWAYS_ACTIVE = -1;
};

struct Params {
    uint256 hashGenesisBlock;
    int nSubsidyHalvingInterval;
    int nSubsidyHalvingInterval2;

    int BIP16Height;

    int BIP34Height;
    uint256 BIP34Hash;

    int BIP65Height;

    int BIP66Height;

    uint32_t nRuleChangeActivationThreshold;
    uint32_t nMinerConfirmationWindow;
    BIP9Deployment vDeployments[MAX_VERSION_BITS_DEPLOYMENTS];

    uint256 powLimit;
    uint256 powLimit2;
    bool fPowAllowMinDifficultyBlocks;
    bool fPowNoRetargeting;
    int64_t nPowTargetSpacing;
    int64_t nPowTargetSpacing2;
    int64_t nPowTargetTimespan;
    int64_t DifficultyAdjustmentInterval() const { return nPowTargetTimespan / nPowTargetSpacing; }
    uint256 nMinimumChainWork;
    uint256 defaultAssumeValid;

    // LightningCashr: General consensus params
    uint32_t powForkTime;               // Time of PoW hash method change
    int lastScryptBlock;                // Height of last scrypt block
    int slowStartBlocks;                // Scale post-fork block reward over this many blocks
    int totalMoneySupplyHeight;         // Height at which TMS is reached, do not issue rewards past this point
    uint256 powLimitSHA;                // Initial hash target at fork
//    CAmount premineAmount;              // Premine amount
//    CScript premineOutputScript;        // Premine output script

    // LightningCashr: Hive-related consensus params
    CAmount minBeeCost;                 // Minimum cost of a bee, used when no more block rewards
    int beeCostFactor;                  // Bee cost is block_reward/beeCostFactor
    std::string beeCreationAddress;     // Unspendable address for bee creation
    std::string hiveCommunityAddress;   // Community fund address
    std::string hiveCommunityAddress2;  // New community fund address
    int communityContribFactor;         // Optionally, donate bct_value/maxCommunityContribFactor to community fund
    int communityContribFactor2;         // Optionally, donate bct_value/maxCommunityContribFactor to community fund
    int beeGestationBlocks;             // The number of blocks for a new bee to mature
    int beeLifespanBlocks;              // The number of blocks a bee lives for after maturation
    uint256 powLimitHive;               // Highest (easiest) bee hash target
    uint256 powLimitHive2;               // Highest (easiest) bee hash target
    uint32_t hiveNonceMarker;           // Nonce marker for hivemined blocks
    int minHiveCheckBlock;              // Don't bother checking below this height for Hive blocks (not used for consensus/validation checks, just efficiency when looking for potential BCTs)
    int hiveTargetAdjustAggression;     // Snap speed for bee hash target adjustment EMA
    int hiveBlockSpacingTarget;         // Target Hive block frequency (1 out of this many blocks should be Hive)

    // LightningCashr: Hive 1.1-related consensus fields
    int minK;                           // Minimum chainwork scale for Hive blocks (see Hive whitepaper section 5)
    int maxK;                           // Maximum chainwork scale for Hive blocks (see Hive whitepaper section 5)
    int maxK2;                           // Maximum chainwork scale for Hive blocks (see Hive whitepaper section 5)
    double maxHiveDiff;                 // Hive difficulty at which max chainwork bonus is awarded
    double maxHiveDiff2;                 // Hive difficulty at which max chainwork bonus is awarded
    int maxKPow;                        // Maximum chainwork scale for PoW blocks
    double powSplit1;                   // Below this Hive difficulty threshold, PoW block chainwork bonus is halved
    double powSplit2;                   // Below this Hive difficulty threshold, PoW block chainwork bonus is halved again
    double powSplit12;                   // Below this Hive difficulty threshold, PoW block chainwork bonus is halved
    double powSplit22;                   // Below this Hive difficulty threshold, PoW block chainwork bonus is halved again
    int maxConsecutiveHiveBlocks;       // Maximum hive blocks that can occur consecutively before a PoW block is required
    int hiveDifficultyWindow;           // How many blocks the SMA averages over in hive difficulty adjust

    bool variableBeecost;
    int variableForkBlock;
    bool isTestnet;
    int ratioForkBlock;
    int beeLifespanBlocks2;
    int beeLifespanBlocks3;
    int remvariableForkBlock;

};
} // namespace Consensus

#endif // BITCOIN_CONSENSUS_PARAMS_H
