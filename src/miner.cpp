// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2025 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#if !defined __MINGW32__
#include <sys/resource.h>
#endif

#include <miner.h>

#include <amount.h>
#include <chain.h>
#include <chainparams.h>
#include <coins.h>
#include <consensus/consensus.h>
#include <consensus/merkle.h>
#include <consensus/tx_verify.h>
#include <consensus/validation.h>
#include <crypto/scrypt.h>
#include <hash.h>
#include <net.h>
#include <policy/feerate.h>
#include <policy/policy.h>
#include <pow.h>
#include <primitives/transaction.h>
#include <script/standard.h>
#include <timedata.h>
#include <txmempool.h>
#include <util.h>
#include <utilmoneystr.h>
#include <validation.h>
#include <validationinterface.h>

#include <algorithm>
#include <boost/thread.hpp>
#include <queue>
#include <utility>

#include <wallet/wallet.h>

#include <rpc/server.h>

#include <base58.h>

#include <sync.h>

static CCriticalSection cs_solution_vars;
std::atomic<bool> solutionFound;

std::atomic<bool> earlyAbort;

CBeeRange solvingRange;

uint32_t solvingBee;

uint64_t nLastBlockTx = 0;
uint64_t nLastBlockWeight = 0;

int64_t UpdateTime(CBlockHeader *pblock,
                   const Consensus::Params &consensusParams,
                   const CBlockIndex *pindexPrev) {
  int64_t nOldTime = pblock->nTime;
  int64_t nNewTime =
      std::max(pindexPrev->GetMedianTimePast() + 1, GetAdjustedTime());

  if (nOldTime < nNewTime)
    pblock->nTime = nNewTime;

  return nNewTime - nOldTime;
}

BlockAssembler::Options::Options() {
  blockMinFeeRate = CFeeRate(DEFAULT_BLOCK_MIN_TX_FEE);
  nBlockMaxWeight = DEFAULT_BLOCK_MAX_WEIGHT;
}

BlockAssembler::BlockAssembler(const CChainParams &params,
                               const Options &options)
    : chainparams(params) {
  blockMinFeeRate = options.blockMinFeeRate;

  nBlockMaxWeight = std::max<size_t>(
      4000, std::min<size_t>(MAX_BLOCK_WEIGHT - 4000, options.nBlockMaxWeight));
}

static BlockAssembler::Options DefaultOptions(const CChainParams &params) {
  BlockAssembler::Options options;
  options.nBlockMaxWeight =
      gArgs.GetArg("-blockmaxweight", DEFAULT_BLOCK_MAX_WEIGHT);
  if (gArgs.IsArgSet("-blockmintxfee")) {
    CAmount n = 0;
    ParseMoney(gArgs.GetArg("-blockmintxfee", ""), n);
    options.blockMinFeeRate = CFeeRate(n);
  } else {
    options.blockMinFeeRate = CFeeRate(DEFAULT_BLOCK_MIN_TX_FEE);
  }
  return options;
}

BlockAssembler::BlockAssembler(const CChainParams &params)
    : BlockAssembler(params, DefaultOptions(params)) {}

void BlockAssembler::resetBlock() {
  inBlock.clear();

  nBlockWeight = 4000;
  nBlockSigOpsCost = 400;
  fIncludeWitness = false;
  fIncludeBCTs = true;

  nBlockTx = 0;
  nFees = 0;
}

std::unique_ptr<CBlockTemplate>
BlockAssembler::CreateNewBlock(const CScript &scriptPubKeyIn,
                               bool fMineWitnessTx,
                               const CScript *hiveProofScript) {
  int64_t nTimeStart = GetTimeMicros();

  resetBlock();

  pblocktemplate.reset(new CBlockTemplate());

  if (!pblocktemplate.get())
    return nullptr;
  pblock = &pblocktemplate->block;

  pblock->vtx.emplace_back();
  pblocktemplate->vTxFees.push_back(-1);

  pblocktemplate->vTxSigOpsCost.push_back(-1);

  LOCK2(cs_main, mempool.cs);
  CBlockIndex *pindexPrev = chainActive.Tip();
  assert(pindexPrev != nullptr);

  if (hiveProofScript && !IsHiveEnabled(pindexPrev, chainparams.GetConsensus()))
    throw std::runtime_error(
        "Error: The Hive is not yet enabled on the network");

  nHeight = pindexPrev->nHeight + 1;

  pblock->nVersion =
      ComputeBlockVersion(pindexPrev, chainparams.GetConsensus());

  if (chainparams.MineBlocksOnDemand())
    pblock->nVersion = gArgs.GetArg("-blockversion", pblock->nVersion);

  pblock->nTime = GetAdjustedTime();
  const int64_t nMedianTimePast = pindexPrev->GetMedianTimePast();

  nLockTimeCutoff = (STANDARD_LOCKTIME_VERIFY_FLAGS & LOCKTIME_MEDIAN_TIME_PAST)
                        ? nMedianTimePast
                        : pblock->GetBlockTime();

  fIncludeWitness = IsWitnessEnabled(pindexPrev, chainparams.GetConsensus()) &&
                    fMineWitnessTx;

  int nPackagesSelected = 0;
  int nDescendantsUpdated = 0;

  if (hiveProofScript)
    fIncludeBCTs = false;

  addPackageTxs(nPackagesSelected, nDescendantsUpdated);

  int64_t nTime1 = GetTimeMicros();

  nLastBlockTx = nBlockTx;
  nLastBlockWeight = nBlockWeight;

  if (hiveProofScript) {
    CMutableTransaction coinbaseTx;

    coinbaseTx.vin.resize(1);
    coinbaseTx.vin[0].prevout.SetNull();
    coinbaseTx.vin[0].scriptSig = CScript() << nHeight << OP_0;

    coinbaseTx.vout.resize(2);
    coinbaseTx.vout[0].scriptPubKey = *hiveProofScript;
    coinbaseTx.vout[0].nValue = 0;

    coinbaseTx.vout[1].scriptPubKey = scriptPubKeyIn;
    coinbaseTx.vout[1].nValue =
        nFees + GetBlockSubsidy(nHeight, chainparams.GetConsensus());

    pblock->vtx[0] = MakeTransactionRef(std::move(coinbaseTx));
    pblocktemplate->vchCoinbaseCommitment = GenerateCoinbaseCommitment(
        *pblock, pindexPrev, chainparams.GetConsensus());
    pblocktemplate->vTxFees[0] = -nFees;
  } else {
    CMutableTransaction coinbaseTx;
    coinbaseTx.vin.resize(1);
    coinbaseTx.vin[0].prevout.SetNull();
    coinbaseTx.vout.resize(1);
    coinbaseTx.vout[0].scriptPubKey = scriptPubKeyIn;
    coinbaseTx.vout[0].nValue =
        nFees + GetBlockSubsidy(nHeight, chainparams.GetConsensus());
    coinbaseTx.vin[0].scriptSig = CScript() << nHeight << OP_0;
    pblock->vtx[0] = MakeTransactionRef(std::move(coinbaseTx));
    pblocktemplate->vchCoinbaseCommitment = GenerateCoinbaseCommitment(
        *pblock, pindexPrev, chainparams.GetConsensus());
    pblocktemplate->vTxFees[0] = -nFees;
  }

  LogPrintf("CreateNewBlock(): block weight: %u txs: %u fees: %ld sigops %d\n",
            GetBlockWeight(*pblock), nBlockTx, nFees, nBlockSigOpsCost);

  pblock->hashPrevBlock = pindexPrev->GetBlockHash();
  UpdateTime(pblock, chainparams.GetConsensus(), pindexPrev);

  if (hiveProofScript)
    pblock->nBits =
        GetNextHiveWorkRequired(pindexPrev, chainparams.GetConsensus());
  else
    pblock->nBits =
        GetNextWorkRequired(pindexPrev, pblock, chainparams.GetConsensus());

  pblock->nNonce =
      hiveProofScript ? chainparams.GetConsensus().hiveNonceMarker : 0;
  pblocktemplate->vTxSigOpsCost[0] =
      WITNESS_SCALE_FACTOR * GetLegacySigOpCount(*pblock->vtx[0]);

  CValidationState state;
  if (!TestBlockValidity(state, chainparams, *pblock, pindexPrev, false,
                         false)) {
    throw std::runtime_error(strprintf("%s: TestBlockValidity failed: %s",
                                       __func__, FormatStateMessage(state)));
  }

  int64_t nTime2 = GetTimeMicros();

  LogPrint(BCLog::BENCH,
           "CreateNewBlock() packages: %.2fms (%d packages, %d updated "
           "descendants), validity: %.2fms (total %.2fms)\n",
           0.001 * (nTime1 - nTimeStart), nPackagesSelected,
           nDescendantsUpdated, 0.001 * (nTime2 - nTime1),
           0.001 * (nTime2 - nTimeStart));

  return std::move(pblocktemplate);
}

void BlockAssembler::onlyUnconfirmed(CTxMemPool::setEntries &testSet) {
  for (CTxMemPool::setEntries::iterator iit = testSet.begin();
       iit != testSet.end();) {
    if (inBlock.count(*iit)) {
      testSet.erase(iit++);
    } else {
      iit++;
    }
  }
}

bool BlockAssembler::TestPackage(uint64_t packageSize,
                                 int64_t packageSigOpsCost) const {
  if (nBlockWeight + WITNESS_SCALE_FACTOR * packageSize >= nBlockMaxWeight)
    return false;
  if (nBlockSigOpsCost + packageSigOpsCost >= MAX_BLOCK_SIGOPS_COST)
    return false;
  return true;
}

bool BlockAssembler::TestPackageTransactions(
    const CTxMemPool::setEntries &package) {
  const Consensus::Params &consensusParams = Params().GetConsensus();

  for (const CTxMemPool::txiter it : package) {
    if (!IsFinalTx(it->GetTx(), nHeight, nLockTimeCutoff))
      return false;
    if (!fIncludeWitness && it->GetTx().HasWitness())
      return false;

    if (!fIncludeBCTs &&
        it->GetTx().IsBCT(consensusParams,
                          GetScriptForDestination(DecodeDestination(
                              consensusParams.beeCreationAddress))))
      return false;
  }
  return true;
}

void BlockAssembler::AddToBlock(CTxMemPool::txiter iter) {
  pblock->vtx.emplace_back(iter->GetSharedTx());
  pblocktemplate->vTxFees.push_back(iter->GetFee());
  pblocktemplate->vTxSigOpsCost.push_back(iter->GetSigOpCost());
  nBlockWeight += iter->GetTxWeight();
  ++nBlockTx;
  nBlockSigOpsCost += iter->GetSigOpCost();
  nFees += iter->GetFee();
  inBlock.insert(iter);

  bool fPrintPriority =
      gArgs.GetBoolArg("-printpriority", DEFAULT_PRINTPRIORITY);
  if (fPrintPriority) {
    LogPrintf("fee %s txid %s\n",
              CFeeRate(iter->GetModifiedFee(), iter->GetTxSize()).ToString(),
              iter->GetTx().GetHash().ToString());
  }
}

int BlockAssembler::UpdatePackagesForAdded(
    const CTxMemPool::setEntries &alreadyAdded,
    indexed_modified_transaction_set &mapModifiedTx) {
  int nDescendantsUpdated = 0;
  for (const CTxMemPool::txiter it : alreadyAdded) {
    CTxMemPool::setEntries descendants;
    mempool.CalculateDescendants(it, descendants);

    for (CTxMemPool::txiter desc : descendants) {
      if (alreadyAdded.count(desc))
        continue;
      ++nDescendantsUpdated;
      modtxiter mit = mapModifiedTx.find(desc);
      if (mit == mapModifiedTx.end()) {
        CTxMemPoolModifiedEntry modEntry(desc);
        modEntry.nSizeWithAncestors -= it->GetTxSize();
        modEntry.nModFeesWithAncestors -= it->GetModifiedFee();
        modEntry.nSigOpCostWithAncestors -= it->GetSigOpCost();
        mapModifiedTx.insert(modEntry);
      } else {
        mapModifiedTx.modify(mit, update_for_parent_inclusion(it));
      }
    }
  }
  return nDescendantsUpdated;
}

bool BlockAssembler::SkipMapTxEntry(
    CTxMemPool::txiter it, indexed_modified_transaction_set &mapModifiedTx,
    CTxMemPool::setEntries &failedTx) {
  assert(it != mempool.mapTx.end());
  return mapModifiedTx.count(it) || inBlock.count(it) || failedTx.count(it);
}

void BlockAssembler::SortForBlock(
    const CTxMemPool::setEntries &package, CTxMemPool::txiter entry,
    std::vector<CTxMemPool::txiter> &sortedEntries) {
  sortedEntries.clear();
  sortedEntries.insert(sortedEntries.begin(), package.begin(), package.end());
  std::sort(sortedEntries.begin(), sortedEntries.end(),
            CompareTxIterByAncestorCount());
}

void BlockAssembler::addPackageTxs(int &nPackagesSelected,
                                   int &nDescendantsUpdated) {
  indexed_modified_transaction_set mapModifiedTx;

  CTxMemPool::setEntries failedTx;

  UpdatePackagesForAdded(inBlock, mapModifiedTx);

  CTxMemPool::indexed_transaction_set::index<ancestor_score>::type::iterator
      mi = mempool.mapTx.get<ancestor_score>().begin();
  CTxMemPool::txiter iter;

  const int64_t MAX_CONSECUTIVE_FAILURES = 1000;
  int64_t nConsecutiveFailed = 0;

  while (mi != mempool.mapTx.get<ancestor_score>().end() ||
         !mapModifiedTx.empty()) {
    if (mi != mempool.mapTx.get<ancestor_score>().end() &&
        SkipMapTxEntry(mempool.mapTx.project<0>(mi), mapModifiedTx, failedTx)) {
      ++mi;
      continue;
    }

    bool fUsingModified = false;

    modtxscoreiter modit = mapModifiedTx.get<ancestor_score>().begin();
    if (mi == mempool.mapTx.get<ancestor_score>().end()) {
      iter = modit->iter;
      fUsingModified = true;
    } else {
      iter = mempool.mapTx.project<0>(mi);
      if (modit != mapModifiedTx.get<ancestor_score>().end() &&
          CompareTxMemPoolEntryByAncestorFee()(*modit,
                                               CTxMemPoolModifiedEntry(iter))) {
        iter = modit->iter;
        fUsingModified = true;
      } else {
        ++mi;
      }
    }

    assert(!inBlock.count(iter));

    uint64_t packageSize = iter->GetSizeWithAncestors();
    CAmount packageFees = iter->GetModFeesWithAncestors();
    int64_t packageSigOpsCost = iter->GetSigOpCostWithAncestors();
    if (fUsingModified) {
      packageSize = modit->nSizeWithAncestors;
      packageFees = modit->nModFeesWithAncestors;
      packageSigOpsCost = modit->nSigOpCostWithAncestors;
    }

    if (packageFees < blockMinFeeRate.GetFee(packageSize)) {
      return;
    }

    if (!TestPackage(packageSize, packageSigOpsCost)) {
      if (fUsingModified) {
        mapModifiedTx.get<ancestor_score>().erase(modit);
        failedTx.insert(iter);
      }

      ++nConsecutiveFailed;

      if (nConsecutiveFailed > MAX_CONSECUTIVE_FAILURES &&
          nBlockWeight > nBlockMaxWeight - 4000) {
        break;
      }
      continue;
    }

    CTxMemPool::setEntries ancestors;
    uint64_t nNoLimit = std::numeric_limits<uint64_t>::max();
    std::string dummy;
    mempool.CalculateMemPoolAncestors(*iter, ancestors, nNoLimit, nNoLimit,
                                      nNoLimit, nNoLimit, dummy, false);

    onlyUnconfirmed(ancestors);
    ancestors.insert(iter);

    if (!TestPackageTransactions(ancestors)) {
      if (fUsingModified) {
        mapModifiedTx.get<ancestor_score>().erase(modit);
        failedTx.insert(iter);
      }
      continue;
    }

    nConsecutiveFailed = 0;

    std::vector<CTxMemPool::txiter> sortedEntries;
    SortForBlock(ancestors, iter, sortedEntries);

    for (size_t i = 0; i < sortedEntries.size(); ++i) {
      AddToBlock(sortedEntries[i]);

      mapModifiedTx.erase(sortedEntries[i]);
    }

    ++nPackagesSelected;

    nDescendantsUpdated += UpdatePackagesForAdded(ancestors, mapModifiedTx);
  }
}

CBlockTemplate *CreateNewBlock(const CChainParams &chainparams,
                               const CScript &scriptPubKeyIn) {
  BlockAssembler assembler(chainparams);
  return assembler.CreateNewBlock(scriptPubKeyIn).release();
}

void IncrementExtraNonce(CBlock *pblock, const CBlockIndex *pindexPrev,
                         unsigned int &nExtraNonce) {
  static uint256 hashPrevBlock;
  if (hashPrevBlock != pblock->hashPrevBlock) {
    nExtraNonce = 0;
    hashPrevBlock = pblock->hashPrevBlock;
  }
  ++nExtraNonce;
  unsigned int nHeight = pindexPrev->nHeight + 1;

  CMutableTransaction txCoinbase(*pblock->vtx[0]);
  txCoinbase.vin[0].scriptSig =
      (CScript() << nHeight << CScriptNum(nExtraNonce)) + COINBASE_FLAGS;
  assert(txCoinbase.vin[0].scriptSig.size() <= 100);

  pblock->vtx[0] = MakeTransactionRef(std::move(txCoinbase));
  pblock->hashMerkleRoot = BlockMerkleRoot(*pblock);
}

void BeeKeeper(const CChainParams &chainparams) {
  const Consensus::Params &consensusParams = chainparams.GetConsensus();

  LogPrintf("BeeKeeper: Thread started\n");
  RenameThread("hive-beekeeper");

  int height;
  {
    LOCK(cs_main);
    height = chainActive.Tip()->nHeight;
  }

  try {
    while (true) {
      int sleepTime =
          std::max((int64_t)1,
                   gArgs.GetArg("-hivecheckdelay", DEFAULT_HIVE_CHECK_DELAY));
      MilliSleep(sleepTime);

      int newHeight;
      {
        LOCK(cs_main);
        newHeight = chainActive.Tip()->nHeight;
      }
      if (newHeight != height) {
        height = newHeight;
        try {
          BusyBees(consensusParams, height);
        } catch (const std::runtime_error &e) {
          LogPrintf("! BeeKeeper: Error: %s\n", e.what());
        }
      }
    }
  } catch (const boost::thread_interrupted &) {
    LogPrintf("!!! BeeKeeper: FATAL: Thread interrupted\n");
    throw;
  }
}

void AbortWatchThread(int height) {
  while (true) {
    MilliSleep(1);

    if (solutionFound.load() || earlyAbort.load())
      return;

    int newHeight;
    {
      LOCK(cs_main);
      newHeight = chainActive.Tip()->nHeight;
    }

    if (newHeight != height) {
      earlyAbort.store(true);
      return;
    }
  }
}

void CheckBin(int threadID, std::vector<CBeeRange> bin,
              std::string deterministicRandString,
              arith_uint256 beeHashTarget) {
  int checkCount = 0;
  for (std::vector<CBeeRange>::const_iterator it = bin.begin(); it != bin.end();
       it++) {
    CBeeRange beeRange = *it;

    for (int i = beeRange.offset; i < beeRange.offset + beeRange.count; i++) {
      if (checkCount++ % 1000 == 0) {
        if (solutionFound.load() || earlyAbort.load()) {
          return;
        }
      }

      std::string hashHex = (CHashWriter(SER_GETHASH, 0)
                             << deterministicRandString << beeRange.txid << i)
                                .GetHash()
                                .GetHex();
      arith_uint256 beeHash = arith_uint256(hashHex);

      if (beeHash < beeHashTarget) {
        LOCK(cs_solution_vars);

        solutionFound.store(true);
        solvingRange = beeRange;
        solvingBee = i;
        return;
      }
    }
  }
}

bool BusyBees(const Consensus::Params &consensusParams, int height) {
  bool verbose = LogAcceptCategory(BCLog::HIVE);

  CBlockIndex *pindexPrev = chainActive.Tip();
  assert(pindexPrev != nullptr);

  if (!IsHiveEnabled(pindexPrev, consensusParams)) {
    LogPrint(BCLog::HIVE, "BusyBees: Skipping hive check: The Hive is not "
                          "enabled on the network\n");
    return false;
  }
  if (!g_connman) {
    LogPrint(BCLog::HIVE, "BusyBees: Skipping hive check: Peer-to-peer "
                          "functionality missing or disabled\n");
    return false;
  }
  if (g_connman->GetNodeCount(CConnman::CONNECTIONS_ALL) == 0) {
    LogPrint(BCLog::HIVE, "BusyBees: Skipping hive check (not connected)\n");
    return false;
  }
  if (IsInitialBlockDownload()) {
    LogPrint(BCLog::HIVE,
             "BusyBees: Skipping hive check (in initial block download)\n");
    return false;
  }

  if (IsHive12Enabled(pindexPrev->nHeight)) {
    int hiveBlocksAtTip = 0;
    CBlockIndex *pindexTemp = pindexPrev;
    while (pindexTemp->GetBlockHeader().IsHiveMined(consensusParams)) {
      assert(pindexTemp->pprev);
      pindexTemp = pindexTemp->pprev;
      hiveBlocksAtTip++;
    }
    if (hiveBlocksAtTip >= consensusParams.maxConsecutiveHiveBlocks) {
      return false;
    }
  } else {
    if (pindexPrev->GetBlockHeader().IsHiveMined(consensusParams)) {
      return false;
    }
  }

  JSONRPCRequest request;
  CWallet *const pwallet = GetWalletForJSONRPCRequest(request);
  if (!EnsureWalletIsAvailable(pwallet, true)) {
    LogPrint(BCLog::HIVE,
             "BusyBees: Skipping hive check (wallet unavailable)\n");
    return false;
  }
  if (pwallet->IsLocked()) {
    LogPrint(BCLog::HIVE, "BusyBees: Skipping hive check, wallet is locked\n");
    return false;
  }

  LogPrintf("********************* Hive: Bees at work *********************\n");

  std::string deterministicRandString = GetDeterministicRandString(pindexPrev);
  if (verbose)
    LogPrintf("BusyBees: deterministicRandString   = %s\n",
              deterministicRandString);

  arith_uint256 beeHashTarget;
  beeHashTarget.SetCompact(
      GetNextHiveWorkRequired(pindexPrev, consensusParams));
  if (verbose)
    LogPrintf("BusyBees: beeHashTarget             = %s\n",
              beeHashTarget.ToString());

  std::vector<CBeeCreationTransactionInfo> bcts;

  if ((chainActive.Tip()->nHeight) >= nSpeedFork) {
    bcts = pwallet->GetBCTs(false, false, consensusParams);
  }

  if ((consensusParams.variableBeecost) &&
      (((chainActive.Tip()->nHeight) - 1) >=
       (consensusParams.variableForkBlock)) &&
      (((chainActive.Tip()->nHeight) - 1) >=
       (consensusParams.remvariableForkBlock)) &&
      ((chainActive.Tip()->nHeight) < nSpeedFork)) {
    bcts = pwallet->GetBCTs3(false, false, consensusParams);
  }
  if ((consensusParams.variableBeecost) &&
      (((chainActive.Tip()->nHeight) - 1) >=
       (consensusParams.variableForkBlock)) &&
      (((chainActive.Tip()->nHeight) - 1) <
       (consensusParams.remvariableForkBlock)) &&
      ((chainActive.Tip()->nHeight) < nSpeedFork)) {
    bcts = pwallet->GetBCTs2(false, false, consensusParams);
  }
  if ((consensusParams.variableBeecost) &&
      (((chainActive.Tip()->nHeight) - 1) <
       (consensusParams.variableForkBlock)) &&
      ((chainActive.Tip()->nHeight) < nSpeedFork)) {
    bcts = pwallet->GetBCTs(false, false, consensusParams);
  }

  int totalBees = 0;

  for (std::vector<CBeeCreationTransactionInfo>::const_iterator it =
           bcts.begin();
       it != bcts.end(); it++) {
    CBeeCreationTransactionInfo bct = *it;

    if (bct.beeStatus != "mature")
      continue;
    totalBees += bct.beeCount;
  }

  if (totalBees == 0) {
    LogPrint(BCLog::HIVE, "BusyBees: No mature bees found\n");
    return false;
  }

  int coreCount = GetNumVirtualCores();
  int threadCount = gArgs.GetArg("-hivecheckthreads", DEFAULT_HIVE_THREADS);
  if (threadCount == -2)
    threadCount = std::max(1, coreCount - 1);
  else if (threadCount < 0 || threadCount > coreCount)
    threadCount = coreCount;
  else if (threadCount == 0)
    threadCount = 1;

  int beesPerBin = ceil(totalBees / (float)threadCount);

  if (verbose)
    LogPrint(BCLog::HIVE,
             "BusyBees: Binning %i bees in %i bins (%i bees per bin)\n",
             totalBees, threadCount, beesPerBin);
  std::vector<CBeeCreationTransactionInfo>::const_iterator bctIterator =
      bcts.begin();
  CBeeCreationTransactionInfo bct = *bctIterator;
  std::vector<std::vector<CBeeRange>> beeBins;
  int beeOffset = 0;

  while (bctIterator != bcts.end()) {
    std::vector<CBeeRange> currentBin;

    int beesInBin = 0;
    while (bctIterator != bcts.end()) {
      int spaceLeft = beesPerBin - beesInBin;
      if (bct.beeCount - beeOffset <= spaceLeft) {
        CBeeRange range = {bct.txid, bct.honeyAddress, bct.communityContrib,
                           beeOffset, bct.beeCount - beeOffset};
        currentBin.push_back(range);

        beesInBin += bct.beeCount - beeOffset;
        beeOffset = 0;

        do {
          bctIterator++;
          if (bctIterator == bcts.end())
            break;
          bct = *bctIterator;
        } while (bct.beeStatus != "mature");
      } else {
        CBeeRange range = {bct.txid, bct.honeyAddress, bct.communityContrib,
                           beeOffset, spaceLeft};
        currentBin.push_back(range);
        beeOffset += spaceLeft;
        break;
      }
    }
    beeBins.push_back(currentBin);
  }

  if (verbose)
    LogPrintf("BusyBees: Running bins\n");
  solutionFound.store(false);
  earlyAbort.store(false);
  std::vector<std::vector<CBeeRange>>::const_iterator beeBinIterator =
      beeBins.begin();
  std::vector<boost::thread> binThreads;
  int64_t checkTime = GetTimeMillis();
  int binID = 0;
  while (beeBinIterator != beeBins.end()) {
    std::vector<CBeeRange> beeBin = *beeBinIterator;

    if (verbose) {
      LogPrintf("BusyBees: Bin #%i\n", binID);
      std::vector<CBeeRange>::const_iterator beeRangeIterator = beeBin.begin();
      while (beeRangeIterator != beeBin.end()) {
        CBeeRange beeRange = *beeRangeIterator;
        LogPrintf("offset = %i, count = %i, txid = %s\n", beeRange.offset,
                  beeRange.count, beeRange.txid);
        beeRangeIterator++;
      }
    }
    binThreads.push_back(boost::thread(CheckBin, binID++, beeBin,
                                       deterministicRandString, beeHashTarget));

    beeBinIterator++;
  }

  bool useEarlyAbortThread =
      gArgs.GetBoolArg("-hiveearlyout", DEFAULT_HIVE_EARLY_OUT);
  if (verbose && useEarlyAbortThread)
    LogPrintf("BusyBees: Will use early-abort thread\n");

  boost::thread *earlyAbortThread;
  if (useEarlyAbortThread)
    earlyAbortThread = new boost::thread(AbortWatchThread, height);

  for (auto &t : binThreads)
    t.join();

  checkTime = GetTimeMillis() - checkTime;

  if (useEarlyAbortThread) {
    if (earlyAbort.load()) {
      LogPrintf("BusyBees: Chain state changed (check aborted after %ims)\n",
                checkTime);
      return false;
    } else {
      earlyAbort.store(true);
      earlyAbortThread->join();
    }
  }

  if (!solutionFound.load()) {
    LogPrintf("BusyBees: No bee meets hash target (%i bees checked with %i "
              "threads in %ims)\n",
              totalBees, threadCount, checkTime);
    return false;
  }
  LogPrintf("BusyBees: Bee meets hash target (check aborted after %ims). "
            "Solution with bee #%i from BCT %s. Honey address is %s.\n",
            checkTime, solvingBee, solvingRange.txid,
            solvingRange.honeyAddress);

  std::vector<unsigned char> messageProofVec;
  std::vector<unsigned char> txidVec(solvingRange.txid.begin(),
                                     solvingRange.txid.end());
  CScript hiveProofScript;
  uint32_t bctHeight;
  {
    LOCK2(cs_main, pwallet->cs_wallet);

    CTxDestination dest = DecodeDestination(solvingRange.honeyAddress);
    if (!IsValidDestination(dest)) {
      LogPrintf("BusyBees: Honey destination invalid\n");
      return false;
    }

    const CKeyID *keyID = boost::get<CKeyID>(&dest);
    if (!keyID) {
      LogPrintf(
          "BusyBees: Wallet doesn't have privkey for honey destination\n");
      return false;
    }

    CKey key;
    if (!pwallet->GetKey(*keyID, key)) {
      LogPrintf("BusyBees: Privkey unavailable\n");
      return false;
    }

    CHashWriter ss(SER_GETHASH, 0);
    ss << deterministicRandString;
    uint256 mhash = ss.GetHash();
    if (!key.SignCompact(mhash, messageProofVec)) {
      LogPrintf("BusyBees: Couldn't sign the bee proof!\n");
      return false;
    }
    if (verbose)
      LogPrintf("BusyBees: messageSig                = %s\n",
                HexStr(&messageProofVec[0],
                       &messageProofVec[messageProofVec.size()]));

    COutPoint out(uint256S(solvingRange.txid), 0);
    Coin coin;
    if (!pcoinsTip || !pcoinsTip->GetCoin(out, coin)) {
      LogPrintf("BusyBees: Couldn't get the bct utxo!\n");
      return false;
    }
    bctHeight = coin.nHeight;
  }

  unsigned char beeNonceEncoded[4];
  WriteLE32(beeNonceEncoded, solvingBee);
  std::vector<unsigned char> beeNonceVec(beeNonceEncoded, beeNonceEncoded + 4);

  unsigned char bctHeightEncoded[4];
  WriteLE32(bctHeightEncoded, bctHeight);
  std::vector<unsigned char> bctHeightVec(bctHeightEncoded,
                                          bctHeightEncoded + 4);

  opcodetype communityContribFlag =
      solvingRange.communityContrib ? OP_TRUE : OP_FALSE;
  hiveProofScript << OP_RETURN << OP_BEE << beeNonceVec << bctHeightVec
                  << communityContribFlag << txidVec << messageProofVec;

  CScript honeyScript =
      GetScriptForDestination(DecodeDestination(solvingRange.honeyAddress));

  std::unique_ptr<CBlockTemplate> pblocktemplate(
      BlockAssembler(Params()).CreateNewBlock(honeyScript, true,
                                              &hiveProofScript));
  if (!pblocktemplate.get()) {
    LogPrintf("BusyBees: Couldn't create block\n");
    return false;
  }
  CBlock *pblock = &pblocktemplate->block;
  pblock->hashMerkleRoot = BlockMerkleRoot(*pblock);

  {
    LOCK(cs_main);
    if (pblock->hashPrevBlock != chainActive.Tip()->GetBlockHash()) {
      LogPrintf("BusyBees: Generated block is stale.\n");
      return false;
    }
  }

  if (verbose) {
    LogPrintf("BusyBees: Block created:\n");
    LogPrintf("%s", pblock->ToString());
  }

  std::shared_ptr<const CBlock> shared_pblock =
      std::make_shared<const CBlock>(*pblock);
  if (!ProcessNewBlock(Params(), shared_pblock, true, nullptr)) {
    LogPrintf("BusyBees: Block wasn't accepted\n");
    return false;
  }

  LogPrintf("BusyBees: ** Block mined\n");
  return true;
}

struct MinerInfo {
  MinerInfo() {
    nHashes = 0;
    nNonceOffset = 0;
    fKill = 0;
  }

  std::atomic<int64_t> nHashes;
  std::atomic<int> nNonceOffset;
  std::atomic<int> fKill;
};

static std::atomic<int> fMinerRunning;

static std::atomic<int64_t> nMinerStartTime;

static std::vector<MinerInfo *> vMiners;

#if !defined(__MINGW32__)
#ifndef PRIO_MAX
#define PRIO_MAX 20
#endif
#define THREAD_PRIORITY_LOWEST PRIO_MAX
#define THREAD_PRIORITY_BELOW_NORMAL 2
#define THREAD_PRIORITY_NORMAL 0
#define THREAD_PRIORITY_ABOVE_NORMAL (-2)
#endif

static void SetThreadPriority(int nPriority) {
#ifdef WIN32
  SetThreadPriority(GetCurrentThread(), nPriority);
#else

#ifdef PRIO_THREAD
  setpriority(PRIO_THREAD, 0, nPriority);
#else

  setpriority(PRIO_PROCESS, 0, nPriority);
#endif

#endif
}

static void MinerResetStats() {
  fMinerRunning = 0;
  nMinerStartTime = 0;
  for (auto *miner : vMiners)
    delete miner;
  vMiners.clear();
}

double EstimateMinerHashesPerSecond() {
  if (fMinerRunning <= 0 || nMinerStartTime <= 0)
    return 0.0;

  const double nDeltaTime = (double)(GetTimeMillis() - nMinerStartTime);
  int64_t nMinerTotalHashes = 0;
  for (const auto *const miner : vMiners)
    nMinerTotalHashes += miner->nHashes;

  return 1000.0 * ((double)nMinerTotalHashes / nDeltaTime);
}

bool static ScanHash(MinerInfo *miner, const CBlockHeader *pblock,
                     uint32_t &nNonce, uint256 *phash) {
  assert(miner != nullptr && pblock != nullptr && phash != nullptr);
  CBlockHeader &block = *const_cast<CBlockHeader *>(pblock);
  const auto start = GetTimeMillis();

  while (true) {
    nNonce++;
    block.nNonce = nNonce;
    *phash = block.GetHashYespower();
    miner->nHashes += 1;

    if (((uint16_t *)phash)[15] <= 32)
      return true;

    if ((nNonce & 0xfff) == 0 || miner->fKill > 0) {
      if (miner->fKill > 0)
        LogPrintf("LightningCashr miner kill flag > 0\n");
      return false;
    }
  }

  return false;
}

static bool ProcessBlockFound(const CBlock *pblock,
                              const CChainParams &chainparams) {
  LogPrintf("%s\n", pblock->ToString());
  LogPrintf("generated %s\n", FormatMoney(pblock->vtx[0]->vout[0].nValue));

  {
    LOCK(cs_main);
    if (pblock->hashPrevBlock != chainActive.Tip()->GetBlockHash()) {
      LogPrintf("LightningCashr Miner: generated block is stale\n");
      return false;
    }
  }

  bool result = true;
  std::shared_ptr<const CBlock> ptr(new CBlock(*pblock));
  if (!ProcessNewBlock(chainparams, ptr, true, nullptr)) {
    LogPrintf("LightningCashr Miner: ProcessNewBlock, block not accepted\n");
    result = false;
  }

  return result;
}

void static LNCRMiner(MinerInfo *miner, const CChainParams &chainparams) {
  LogPrintf("LightningCashr Miner started\n");
  SetThreadPriority(THREAD_PRIORITY_LOWEST);
  RenameThread("lncr-miner");

  unsigned int nExtraNonce = 0;

  std::shared_ptr<CReserveScript> coinbaseScript;

  try {
    while (true) {
      if (vpwallets.size() > 0 && coinbaseScript == nullptr) {
        vpwallets[0]->GetScriptForMining(coinbaseScript);
      }

      if (!coinbaseScript || coinbaseScript->reserveScript.empty())
        throw std::runtime_error(
            "No coinbase script available (mining requires a wallet)");

      unsigned int nTransactionsUpdatedLast = mempool.GetTransactionsUpdated();
      CBlockIndex *pindexPrev = chainActive.Tip();
      std::unique_ptr<CBlockTemplate> pblocktemplate;

      try {
        pblocktemplate.reset(
            CreateNewBlock(chainparams, coinbaseScript->reserveScript));
      } catch (const std::runtime_error &e) {
        LogPrintf("LightningCashr Miner runtime error: %s\n", e.what());
        LogPrintf("LightningCashr Miner: Keypool ran out, please call "
                  "keypoolrefill before restarting the mining thread\n");
        MilliSleep(4 * 1000);
        vpwallets[0]->GetScriptForMining(coinbaseScript);
        continue;
      }

      CBlock *pblock = &pblocktemplate->block;
      IncrementExtraNonce(pblock, pindexPrev, nExtraNonce);

      LogPrintf("Running LightningCashr Miner with %u transactions in block "
                "(%u bytes)\n",
                pblock->vtx.size(),
                ::GetSerializeSize(*pblock, SER_NETWORK, PROTOCOL_VERSION));

      int64_t nStart = GetTime();
      arith_uint256 hashTarget = arith_uint256().SetCompact(pblock->nBits);
      uint256 hash;
      uint32_t nNonce = static_cast<uint32_t>((int)miner->nNonceOffset);
      bool fBlockFound = false;

      while (true) {
        if (ScanHash(miner, pblock, nNonce, &hash)) {
          if (UintToArith256(hash) <= hashTarget) {
            pblock->nNonce = nNonce;
            LogPrintf("LightningCashr Miner: proof-of-work found  \n  hash: %s "
                      " \ntarget: %s\n",
                      hash.GetHex(), hashTarget.GetHex());
            assert(hash == pblock->GetHashYespower());
            SetThreadPriority(THREAD_PRIORITY_NORMAL);
            fBlockFound = ProcessBlockFound(pblock, chainparams);
            SetThreadPriority(THREAD_PRIORITY_LOWEST);
            if (fBlockFound)
              coinbaseScript->KeepScript();

            if (chainparams.MineBlocksOnDemand())
              throw boost::thread_interrupted();

            break;
          }
        }

        boost::this_thread::interruption_point();

        const bool fvNodesEmpty =
            g_connman->GetNodeCount(CConnman::CONNECTIONS_ALL) <= 0;

        if (nNonce >= 0xffff0000)
          break;
        if (mempool.GetTransactionsUpdated() != nTransactionsUpdatedLast &&
            GetTime() - nStart > 60)
          break;
        if (pindexPrev != chainActive.Tip())
          break;

        if (UpdateTime(pblock, chainparams.GetConsensus(), pindexPrev) < 0)
          break;

        if (chainparams.GetConsensus().fPowAllowMinDifficultyBlocks) {
          hashTarget.SetCompact(pblock->nBits);
        }
      }
    }
  } catch (const boost::thread_interrupted &) {
    LogPrintf("LightningCashr Miner terminated\n");
    throw;
  } catch (const std::runtime_error &e) {
    LogPrintf("LightningCashr Miner runtime error: %s\n", e.what());
    return;
  }
}

void GenerateLNCR(bool fGenerate, int nThreads,
                  const CChainParams &chainparams) {
  static boost::thread_group *minerThreads = nullptr;

  if (nThreads < 0)
    nThreads = GetNumCores();

  if (minerThreads != nullptr) {
    minerThreads->interrupt_all();
    for (auto *const miner : vMiners)
      miner->fKill = 1;
    minerThreads->join_all();
    delete minerThreads;
    minerThreads = nullptr;
  }

  MinerResetStats();

  if (nThreads <= 0 || !fGenerate)
    return;

  const int nNonceMultiplier = 0xffff0000 / nThreads;
  for (int i = 0; i < nThreads; i++) {
    auto *const miner = new MinerInfo();
    miner->nNonceOffset = i * nNonceMultiplier;
    vMiners.push_back(miner);
  }

  nMinerStartTime = GetTimeMillis();
  minerThreads = new boost::thread_group();
  for (int i = 0; i < nThreads; i++)
    minerThreads->create_thread(
        boost::bind(&LNCRMiner, vMiners[i], boost::cref(chainparams)));

  fMinerRunning = 1;
}
