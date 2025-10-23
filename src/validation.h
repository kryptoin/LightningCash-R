// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2025 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_VALIDATION_H
#define BITCOIN_VALIDATION_H

#if defined(HAVE_CONFIG_H)
#include <config/bitcoin-config.h>
#endif

#include <amount.h>
#include <coins.h>
#include <fs.h>
#include <protocol.h>

#include <policy/feerate.h>
#include <script/script_error.h>
#include <sync.h>
#include <util.h>
#include <versionbits.h>

#include <algorithm>
#include <exception>
#include <map>
#include <set>
#include <stdint.h>
#include <string>
#include <utility>
#include <vector>

#include <atomic>

class CBlockIndex;
class CBlockTreeDB;
class CChainParams;
class CCoinsViewDB;
class CInv;
class CConnman;
class CScriptCheck;
class CBlockPolicyEstimator;
class CTxMemPool;
class CValidationState;
struct ChainTxData;

struct PrecomputedTransactionData;
struct LockPoints;

static const bool DEFAULT_WHITELISTRELAY = true;

static const bool DEFAULT_WHITELISTFORCERELAY = true;

static const unsigned int DEFAULT_MIN_RELAY_TX_FEE = 10000 / COIN_SCALE / 10;

static const CAmount DEFAULT_TRANSACTION_MAXFEE = 10 * COIN * COIN_SCALE;

static const CAmount HIGH_TX_FEE_PER_KB = 0.01 * COIN * COIN_SCALE;

static const CAmount HIGH_MAX_TX_FEE = 100 * HIGH_TX_FEE_PER_KB;

static const unsigned int DEFAULT_ANCESTOR_LIMIT = 25;

static const unsigned int DEFAULT_ANCESTOR_SIZE_LIMIT = 101;

static const unsigned int DEFAULT_DESCENDANT_LIMIT = 25;

static const unsigned int DEFAULT_DESCENDANT_SIZE_LIMIT = 101;

static const unsigned int DEFAULT_MEMPOOL_EXPIRY = 336;

static const unsigned int MAX_DISCONNECTED_TX_POOL_SIZE = 20000;

static const unsigned int MAX_BLOCKFILE_SIZE = 0x8000000;

static const unsigned int BLOCKFILE_CHUNK_SIZE = 0x1000000;

static const unsigned int UNDOFILE_CHUNK_SIZE = 0x100000;

static const int SKIP_BLOCKHEADER_POW = 5472830;

static const int MAX_SCRIPTCHECK_THREADS = 16;

static const int DEFAULT_SCRIPTCHECK_THREADS = 0;

static const int MAX_BLOCKS_IN_TRANSIT_PER_PEER = 2048;

static const unsigned int BLOCK_STALLING_TIMEOUT = 4;

static const unsigned int MAX_HEADERS_RESULTS = 2000;

static const int MAX_CMPCTBLOCK_DEPTH = 5;

static const int MAX_BLOCKTXN_DEPTH = 10;

static const unsigned int BLOCK_DOWNLOAD_WINDOW = 1024;

static const unsigned int DATABASE_WRITE_INTERVAL = 60 * 60;

static const unsigned int DATABASE_FLUSH_INTERVAL = 24 * 60 * 60;

static const unsigned int MAX_REJECT_MESSAGE_LENGTH = 111;

static const unsigned int AVG_LOCAL_ADDRESS_BROADCAST_INTERVAL = 24 * 60 * 60;

static const unsigned int AVG_ADDRESS_BROADCAST_INTERVAL = 30;

static const unsigned int INVENTORY_BROADCAST_INTERVAL = 5;

static const unsigned int INVENTORY_BROADCAST_MAX =
    7 * INVENTORY_BROADCAST_INTERVAL;

static const unsigned int AVG_FEEFILTER_BROADCAST_INTERVAL = 10 * 60;

static const unsigned int MAX_FEEFILTER_CHANGE_DELAY = 5 * 60;

static const int64_t BLOCK_DOWNLOAD_TIMEOUT_BASE = 1000000;

static const int64_t BLOCK_DOWNLOAD_TIMEOUT_PER_PEER = 500000;

static const int64_t DEFAULT_MAX_TIP_AGE = 12 * 60;

static const int64_t MAX_FEE_ESTIMATION_TIP_AGE = 3 * 60 * 60;

static const bool DEFAULT_PERMIT_BAREMULTISIG = true;
static const bool DEFAULT_CHECKPOINTS_ENABLED = true;
static const bool DEFAULT_TXINDEX = false;
static const unsigned int DEFAULT_BANSCORE_THRESHOLD = 100;

static const bool DEFAULT_PERSIST_MEMPOOL = true;

static const bool DEFAULT_ENABLE_REPLACEMENT = false;

static const bool DEFAULT_FEEFILTER = true;

static const unsigned int MAX_BLOCKS_TO_ANNOUNCE = 8;

static const int MAX_UNCONNECTING_HEADERS = 10;

static const bool DEFAULT_PEERBLOOMFILTERS = true;

static const int DEFAULT_STOPATHEIGHT = 0;

struct BlockHasher {
  size_t operator()(const uint256 &hash) const { return hash.GetCheapHash(); }
};

extern CScript COINBASE_FLAGS;
extern CCriticalSection cs_main;
extern CBlockPolicyEstimator feeEstimator;
extern CTxMemPool mempool;
typedef std::unordered_map<uint256, CBlockIndex *, BlockHasher> BlockMap;
extern BlockMap &mapBlockIndex;
extern uint64_t nLastBlockTx;
extern uint64_t nLastBlockWeight;
extern const std::string strMessageMagic;
extern CWaitableCriticalSection csBestBlock;
extern CConditionVariable cvBlockChange;
extern std::atomic_bool fImporting;
extern std::atomic_bool fReindex;
extern int nScriptCheckThreads;
extern bool fTxIndex;
extern bool fIsBareMultisigStd;
extern bool fRequireStandard;
extern bool fCheckBlockIndex;
extern bool fCheckpointsEnabled;
extern size_t nCoinCacheUsage;

extern CFeeRate minRelayTxFee;

extern CAmount maxTxFee;

extern int64_t nMaxTipAge;
extern bool fEnableReplacement;
extern const int nYesPowerFork;
extern const int nSpeedFork;
extern const int nAdjustFork;
extern const int nLightFork;
extern const int nHiveRepairFork;
extern const int nContribFork;

extern uint256 hashAssumeValid;

extern arith_uint256 nMinimumChainWork;

extern CBlockIndex *pindexBestHeader;

static const uint64_t nMinDiskSpace = 52428800;

extern bool fHavePruned;

extern bool fPruneMode;

extern uint64_t nPruneTarget;

static const unsigned int MIN_BLOCKS_TO_KEEP = 288;

static const unsigned int NODE_NETWORK_LIMITED_MIN_BLOCKS = 288;

static const signed int DEFAULT_CHECKBLOCKS = 6 * 4;
static const unsigned int DEFAULT_CHECKLEVEL = 3;

static const uint64_t MIN_DISK_SPACE_FOR_BLOCK_FILES = 550 * 1024 * 1024;

bool ProcessNewBlock(const CChainParams &chainparams,
                     const std::shared_ptr<const CBlock> pblock,
                     bool fForceProcessing, bool *fNewBlock);

bool ProcessNewBlockHeaders(const std::vector<CBlockHeader> &block,
                            CValidationState &state,
                            const CChainParams &chainparams,
                            const CBlockIndex **ppindex = nullptr,
                            CBlockHeader *first_invalid = nullptr);

bool CheckDiskSpace(uint64_t nAdditionalBytes = 0);

FILE *OpenBlockFile(const CDiskBlockPos &pos, bool fReadOnly = false);

fs::path GetBlockPosFilename(const CDiskBlockPos &pos, const char *prefix);

bool LoadExternalBlockFile(const CChainParams &chainparams, FILE *fileIn,
                           CDiskBlockPos *dbp = nullptr);

bool LoadGenesisBlock(const CChainParams &chainparams);

bool LoadBlockIndex(const CChainParams &chainparams);

bool LoadChainTip(const CChainParams &chainparams);

void UnloadBlockIndex();

void ThreadScriptCheck();

bool IsInitialBlockDownload();

bool GetTransaction(const uint256 &hash, CTransactionRef &tx,
                    const Consensus::Params &params, uint256 &hashBlock,
                    bool fAllowSlow = false, CBlockIndex *blockIndex = nullptr);

bool ActivateBestChain(
    CValidationState &state, const CChainParams &chainparams,
    std::shared_ptr<const CBlock> pblock = std::shared_ptr<const CBlock>());
CAmount GetBlockSubsidy(int nHeight, const Consensus::Params &consensusParams);
CAmount GetBeeCost(int nHeight, const Consensus::Params &consensusParams);

double GuessVerificationProgress(const ChainTxData &data,
                                 const CBlockIndex *pindex);

uint64_t CalculateCurrentUsage();

void PruneOneBlockFile(const int fileNumber);

void UnlinkPrunedFiles(const std::set<int> &setFilesToPrune);

void FlushStateToDisk();

void PruneAndFlush();

void PruneBlockFilesManual(int nManualPruneHeight);

bool AcceptToMemoryPool(CTxMemPool &pool, CValidationState &state,
                        const CTransactionRef &tx, bool *pfMissingInputs,
                        std::list<CTransactionRef> *plTxnReplaced,
                        bool bypass_limits, const CAmount nAbsurdFee);

std::string FormatStateMessage(const CValidationState &state);

ThresholdState VersionBitsTipState(const Consensus::Params &params,
                                   Consensus::DeploymentPos pos);

BIP9Stats VersionBitsTipStatistics(const Consensus::Params &params,
                                   Consensus::DeploymentPos pos);

int VersionBitsTipStateSinceHeight(const Consensus::Params &params,
                                   Consensus::DeploymentPos pos);

void UpdateCoins(const CTransaction &tx, CCoinsViewCache &inputs, int nHeight);

bool CheckFinalTx(const CTransaction &tx, int flags = -1);

bool TestLockPointValidity(const LockPoints *lp);

bool CheckSequenceLocks(const CTransaction &tx, int flags,
                        LockPoints *lp = nullptr,
                        bool useExistingLockPoints = false);

class CScriptCheck {
private:
  CTxOut m_tx_out;
  const CTransaction *ptxTo;
  unsigned int nIn;
  unsigned int nFlags;
  bool cacheStore;
  ScriptError error;
  PrecomputedTransactionData *txdata;

public:
  CScriptCheck()
      : ptxTo(nullptr), nIn(0), nFlags(0), cacheStore(false),
        error(SCRIPT_ERR_UNKNOWN_ERROR) {}
  CScriptCheck(const CTxOut &outIn, const CTransaction &txToIn,
               unsigned int nInIn, unsigned int nFlagsIn, bool cacheIn,
               PrecomputedTransactionData *txdataIn)
      : m_tx_out(outIn), ptxTo(&txToIn), nIn(nInIn), nFlags(nFlagsIn),
        cacheStore(cacheIn), error(SCRIPT_ERR_UNKNOWN_ERROR), txdata(txdataIn) {
  }

  bool operator()();

  void swap(CScriptCheck &check) {
    std::swap(ptxTo, check.ptxTo);
    std::swap(m_tx_out, check.m_tx_out);
    std::swap(nIn, check.nIn);
    std::swap(nFlags, check.nFlags);
    std::swap(cacheStore, check.cacheStore);
    std::swap(error, check.error);
    std::swap(txdata, check.txdata);
  }

  ScriptError GetScriptError() const { return error; }
};

void InitScriptExecutionCache();

bool ReadBlockFromDisk(CBlock &block, const CDiskBlockPos &pos,
                       const Consensus::Params &consensusParams);
bool ReadBlockFromDisk(CBlock &block, const CBlockIndex *pindex,
                       const Consensus::Params &consensusParams);

bool CheckBlock(const CBlock &block, CValidationState &state,
                const Consensus::Params &consensusParams, bool fCheckPOW = true,
                bool fCheckMerkleRoot = true);

bool TestBlockValidity(CValidationState &state, const CChainParams &chainparams,
                       const CBlock &block, CBlockIndex *pindexPrev,
                       bool fCheckPOW = true, bool fCheckMerkleRoot = true);

bool IsWitnessEnabled(const CBlockIndex *pindexPrev,
                      const Consensus::Params &params);

bool IsHiveEnabled(const CBlockIndex *pindexPrev,
                   const Consensus::Params &params);

bool IsHive11Enabled(const CBlockIndex *pindexPrev,
                     const Consensus::Params &params);

bool IsHive12Enabled(int nHeight);

std::string GetDeterministicRandString(const CBlockIndex *pindexPrev);

bool GetTxByHashAndHeight(const uint256 txHash, const int nHeight,
                          CTransactionRef &txNew, CBlockIndex &foundAtOut,
                          CBlockIndex *pindex,
                          const Consensus::Params &consensusParams);

bool RewindBlockIndex(const CChainParams &params);

void UpdateUncommittedBlockStructures(CBlock &block,
                                      const CBlockIndex *pindexPrev,
                                      const Consensus::Params &consensusParams);

std::vector<unsigned char>
GenerateCoinbaseCommitment(CBlock &block, const CBlockIndex *pindexPrev,
                           const Consensus::Params &consensusParams);

class CVerifyDB {
public:
  CVerifyDB();
  ~CVerifyDB();
  bool VerifyDB(const CChainParams &chainparams, CCoinsView *coinsview,
                int nCheckLevel, int nCheckDepth);
};

bool ReplayBlocks(const CChainParams &params, CCoinsView *view);

CBlockIndex *FindForkInGlobalIndex(const CChain &chain,
                                   const CBlockLocator &locator);

bool PreciousBlock(CValidationState &state, const CChainParams &params,
                   CBlockIndex *pindex);

bool InvalidateBlock(CValidationState &state, const CChainParams &chainparams,
                     CBlockIndex *pindex);

bool ResetBlockFailureFlags(CBlockIndex *pindex);

extern CChain &chainActive;

extern std::unique_ptr<CCoinsViewDB> pcoinsdbview;

extern std::unique_ptr<CCoinsViewCache> pcoinsTip;

extern std::unique_ptr<CBlockTreeDB> pblocktree;

int GetSpendHeight(const CCoinsViewCache &inputs);

extern VersionBitsCache versionbitscache;

int32_t ComputeBlockVersion(const CBlockIndex *pindexPrev,
                            const Consensus::Params &params);

static const unsigned int REJECT_INTERNAL = 0x100;

static const unsigned int REJECT_HIGHFEE = 0x100;

CBlockFileInfo *GetBlockFileInfo(size_t n);

bool DumpMempool();

bool LoadMempool();

bool IsYesPower(int nHeight);

#endif
