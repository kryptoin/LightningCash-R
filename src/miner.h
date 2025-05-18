// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Copyright (c) 2018 The Litecoin Cash Core developers
// Copyright (c) 2019-2022 Antoine Brûlé
// Copyright (c) 2025 The LightningCash-R Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_MINER_H
#define BITCOIN_MINER_H

#include "wallet/wallet.h"
#include <atomic>

#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index_container.hpp>

#include <memory>
#include <primitives/block.h>
#include <stdint.h>
#include <txmempool.h>

class CBlockIndex;
class CChainParams;
class CScript;
class CWallet;

class arith_uint256; // Hive mining optimizations
struct CBeeRange;    // Hive mining optimizations

namespace Consensus {
struct Params;
};

static const bool DEFAULT_GENERATE = false;
static const int DEFAULT_GENERATE_THREADS = 1;

static const bool DEFAULT_PRINTPRIORITY = false;

// Hive mining optimizations: defaults for Hive parameters
static const int DEFAULT_HIVE_CHECK_DELAY = 1;
static const int DEFAULT_HIVE_THREADS = -2;
static const bool DEFAULT_HIVE_EARLY_OUT = true;

struct CBlockTemplate {
  CBlock block;
  std::vector<CAmount> vTxFees;
  std::vector<int64_t> vTxSigOpsCost;
  std::vector<unsigned char> vchCoinbaseCommitment;
};

// Used for block assembly from mempool
struct CTxMemPoolModifiedEntry {
  explicit CTxMemPoolModifiedEntry(CTxMemPool::txiter entry) {
    iter = entry;
    nSizeWithAncestors = entry->GetSizeWithAncestors();
    nModFeesWithAncestors = entry->GetModFeesWithAncestors();
    nSigOpCostWithAncestors = entry->GetSigOpCostWithAncestors();
  }

  int64_t GetModifiedFee() const { return iter->GetModifiedFee(); }
  uint64_t GetSizeWithAncestors() const { return nSizeWithAncestors; }
  CAmount GetModFeesWithAncestors() const { return nModFeesWithAncestors; }
  size_t GetTxSize() const { return iter->GetTxSize(); }
  const CTransaction &GetTx() const { return iter->GetTx(); }

  CTxMemPool::txiter iter;
  uint64_t nSizeWithAncestors;
  CAmount nModFeesWithAncestors;
  int64_t nSigOpCostWithAncestors;
};

struct CompareCTxMemPoolIter {
  bool operator()(const CTxMemPool::txiter &a,
                  const CTxMemPool::txiter &b) const {
    return &(*a) < &(*b);
  }
};

struct modifiedentry_iter {
  typedef CTxMemPool::txiter result_type;
  result_type operator()(const CTxMemPoolModifiedEntry &entry) const {
    return entry.iter;
  }
};

struct CompareTxIterByAncestorCount {
  bool operator()(const CTxMemPool::txiter &a,
                  const CTxMemPool::txiter &b) const {
    if (a->GetCountWithAncestors() != b->GetCountWithAncestors())
      return a->GetCountWithAncestors() < b->GetCountWithAncestors();
    return CTxMemPool::CompareIteratorByHash()(a, b);
  }
};

typedef boost::multi_index_container<
    CTxMemPoolModifiedEntry,
    boost::multi_index::indexed_by<
        boost::multi_index::ordered_unique<modifiedentry_iter,
                                           CompareCTxMemPoolIter>,
        boost::multi_index::ordered_non_unique<
            boost::multi_index::tag<ancestor_score>,
            boost::multi_index::identity<CTxMemPoolModifiedEntry>,
            CompareTxMemPoolEntryByAncestorFee>>>
    indexed_modified_transaction_set;

typedef indexed_modified_transaction_set::nth_index<0>::type::iterator
    modtxiter;
typedef indexed_modified_transaction_set::index<ancestor_score>::type::iterator
    modtxscoreiter;

struct update_for_parent_inclusion {
  explicit update_for_parent_inclusion(CTxMemPool::txiter it) : iter(it) {}
  void operator()(CTxMemPoolModifiedEntry &e) {
    e.nModFeesWithAncestors -= iter->GetFee();
    e.nSizeWithAncestors -= iter->GetTxSize();
    e.nSigOpCostWithAncestors -= iter->GetSigOpCost();
  }

  CTxMemPool::txiter iter;
};

/** Generate a new block, without valid proof-of-work */
class BlockAssembler {
private:
  std::unique_ptr<CBlockTemplate> pblocktemplate;
  CBlock *pblock;
  bool fIncludeWitness;
  bool fIncludeBCTs;
  unsigned int nBlockMaxWeight;
  CFeeRate blockMinFeeRate;

  uint64_t nBlockWeight;
  uint64_t nBlockTx;
  uint64_t nBlockSigOpsCost;
  CAmount nFees;
  CTxMemPool::setEntries inBlock;

  int nHeight;
  int64_t nLockTimeCutoff;
  const CChainParams &chainparams;

public:
  struct Options {
    Options();
    size_t nBlockMaxWeight;
    CFeeRate blockMinFeeRate;
  };

  explicit BlockAssembler(const CChainParams &params);
  BlockAssembler(const CChainParams &params, const Options &options);

  std::unique_ptr<CBlockTemplate>
  CreateNewBlock(const CScript &scriptPubKeyIn, bool fMineWitnessTx = true,
                 const CScript *hiveProofScript = nullptr);

private:
  void resetBlock();
  void AddToBlock(CTxMemPool::txiter iter);
  void addPackageTxs(int &nPackagesSelected, int &nDescendantsUpdated);
  void onlyUnconfirmed(CTxMemPool::setEntries &testSet);
  bool TestPackage(uint64_t packageSize, int64_t packageSigOpsCost) const;
  bool TestPackageTransactions(const CTxMemPool::setEntries &package);
  bool SkipMapTxEntry(CTxMemPool::txiter it,
                      indexed_modified_transaction_set &mapModifiedTx,
                      CTxMemPool::setEntries &failedTx);
  void SortForBlock(const CTxMemPool::setEntries &package,
                    CTxMemPool::txiter entry,
                    std::vector<CTxMemPool::txiter> &sortedEntries);
  int UpdatePackagesForAdded(const CTxMemPool::setEntries &alreadyAdded,
                             indexed_modified_transaction_set &mapModifiedTx);
};

// --- Hive Mining Functions ---

#define MAX_HIVE_NONCE_SIZE 32

std::vector<CBeeCreationTransactionInfo>
GetMatureBees(const Consensus::Params &consensusParams, CWallet *pwallet,
              int heightTip, int &totalBeesOut);

std::vector<std::vector<CBeeRange>>
BinBees(const std::vector<CBeeCreationTransactionInfo> &matureBcts,
        int totalBees, int threadCount);

struct SolutionState {
  std::atomic<bool> found;
  std::atomic<bool> earlyAbort;
  CBeeCreationTransactionInfo solvingBee;
  CBeeRange solvingRange;

  SolutionState()
      : found(false), earlyAbort(false), solvingBee(), solvingRange() {}
};

void CheckBinsForSolution(const std::vector<std::vector<CBeeRange>> &beeBins,
                          const std::string &deterministicRandString,
                          const arith_uint256 &beeHashTarget, int threadCount,
                          SolutionState &solution);

bool GenerateAndSubmitBlock(CWallet *pwallet, const SolutionState &solution,
                            const Consensus::Params &consensusParams,
                            const std::string &deterministicRandString,
                            int heightTip, bool verbose);

bool CheckHiveConditions(const Consensus::Params &consensusParams,
                         const CBlockIndex *pindexPrev);

void GenerateLNCR(bool fGenerate, int nThreads,
                  const CChainParams &chainparams);

void RunBeeThreads(const std::vector<std::vector<CBeeRange>> &beeBins,
                   const std::string &deterministicRandString,
                   const arith_uint256 &beeHashTarget, int height,
                   int totalBees, int threadCount, SolutionState &solution);

CBlockTemplate *CreateNewBlock(const CChainParams &chainparams,
                               const CScript &scriptPubKeyIn);
double EstimateMinerHashesPerSecond();
void IncrementExtraNonce(CBlock *pblock, const CBlockIndex *pindexPrev,
                         unsigned int &nExtraNonce);
int64_t UpdateTime(CBlockHeader *pblock,
                   const Consensus::Params &consensusParams,
                   const CBlockIndex *pindexPrev);

void BeeKeeper(const CChainParams &chainparams); // Bee maintenance thread

bool BusyBees(const Consensus::Params &consensusParams,
              int height); // Hive mining entry point

void CheckBin(int threadID, std::vector<CBeeRange> bin,
              std::string deterministicRandString,
              arith_uint256 beeHashTarget); // Worker thread for Bee mining

void AbortWatchThread(int height); // Early-abort monitor thread

// Shared atomic state (defined in miner.cpp)
#include <cstdint>

#endif // LIGHTNINGCASH-R_MINER_H