// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2025 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <pow.h>

#include <arith_uint256.h>
#include <chain.h>
#include <core_io.h>
#include <primitives/block.h>
#include <uint256.h>
#include <util.h>

#include <script/standard.h>

#include <base58.h>

#include <pubkey.h>

#include <hash.h>

#include <sync.h>

#include <validation.h>

#include <utilstrencodings.h>

BeePopGraphPoint beePopGraph[1024 * 40];

CAmount totalMatureBees;
CBlockIndex *pindexRem;

int switchHmem;
int switchLmem;

int thematurebees;

int beesDying;

int wototo;
int priceState;
int preforkpriceState;
int threshold;
int preforkmatureBees;
int preforkimmatureBees;
int preforkimmatureBCTs;
int preforkmatureBCTs;
int startingWallet;
int numPowBlocks;
int remTipHeight;
int firstRun;
int rpriceState;
int rimmatureBees;
int rimmatureBCTs;
int rmatureBees;
int rmatureBCTs;
int bon;

unsigned int DarkGravityWave(const CBlockIndex *pindexLast,
                             const CBlockHeader *pblock,
                             const Consensus::Params &params) {
  const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);

  int64_t nPastBlocks = 24;

  if (params.fPowAllowMinDifficultyBlocks &&
      pblock->GetBlockTime() >
          pindexLast->GetBlockTime() + params.nPowTargetSpacing * 10)
    return bnPowLimit.GetCompact();

  if (!pindexLast || pindexLast->nHeight - params.lastScryptBlock < nPastBlocks)
    return bnPowLimit.GetCompact();

  const CBlockIndex *pindex = pindexLast;
  arith_uint256 bnPastTargetAvg;

  for (unsigned int nCountBlocks = 1; nCountBlocks <= nPastBlocks;
       nCountBlocks++) {
    while (pindex->GetBlockHeader().IsHiveMined(params)) {
      assert(pindex->pprev);

      pindex = pindex->pprev;
    }

    arith_uint256 bnTarget = arith_uint256().SetCompact(pindex->nBits);
    if (nCountBlocks == 1) {
      bnPastTargetAvg = bnTarget;
    } else {
      bnPastTargetAvg =
          (bnPastTargetAvg * nCountBlocks + bnTarget) / (nCountBlocks + 1);
    }

    if (nCountBlocks != nPastBlocks) {
      assert(pindex->pprev);

      pindex = pindex->pprev;
    }
  }

  arith_uint256 bnNew(bnPastTargetAvg);

  int64_t nActualTimespan = pindexLast->GetBlockTime() - pindex->GetBlockTime();

  int64_t nTargetTimespan = nPastBlocks * params.nPowTargetSpacing;

  if (nActualTimespan < nTargetTimespan / 3)
    nActualTimespan = nTargetTimespan / 3;
  if (nActualTimespan > nTargetTimespan * 3)
    nActualTimespan = nTargetTimespan * 3;

  bnNew *= nActualTimespan;
  bnNew /= nTargetTimespan;

  if (pblock->GetBlockTime() >
      pindexLast->GetBlockTime() + params.nPowTargetSpacing * 30) {
    bnNew = bnPowLimit;

  }

  else if (pblock->GetBlockTime() >
           pindexLast->GetBlockTime() + params.nPowTargetSpacing * 25) {
    bnNew *= 100000;

  }

  else if (pblock->GetBlockTime() >
           pindexLast->GetBlockTime() + params.nPowTargetSpacing * 20) {
    bnNew *= 10000;

  }

  else if (pblock->GetBlockTime() >
           pindexLast->GetBlockTime() + params.nPowTargetSpacing * 15) {
    bnNew *= 1000;

  }

  else if (pblock->GetBlockTime() >
           pindexLast->GetBlockTime() + params.nPowTargetSpacing * 10) {
    bnNew *= 100;

  }

  else {
    bnNew = bnNew;
  }

  if (bnNew > bnPowLimit) {
    bnNew = bnPowLimit;
  }

  return bnNew.GetCompact();
}

unsigned int DarkGravityWave2(const CBlockIndex *pindexLast,
                              const CBlockHeader *pblock,
                              const Consensus::Params &params) {
  const arith_uint256 bnPowLimit2 = UintToArith256(params.powLimit2);

  int64_t nPastBlocks = 24;

  if (params.fPowAllowMinDifficultyBlocks &&
      pblock->GetBlockTime() >
          pindexLast->GetBlockTime() + params.nPowTargetSpacing2 * 10)
    return bnPowLimit2.GetCompact();

  if (IsHive12Enabled(pindexLast->nHeight)) {
    while (pindexLast->GetBlockHeader().IsHiveMined(params)) {
      assert(pindexLast->pprev);

      pindexLast = pindexLast->pprev;
    }
  }

  if (!pindexLast || pindexLast->nHeight - params.lastScryptBlock < nPastBlocks)
    return bnPowLimit2.GetCompact();

  const CBlockIndex *pindex = pindexLast;
  arith_uint256 bnPastTargetAvg;

  for (unsigned int nCountBlocks = 1; nCountBlocks <= nPastBlocks;
       nCountBlocks++) {
    while (pindex->GetBlockHeader().IsHiveMined(params)) {
      assert(pindex->pprev);

      pindex = pindex->pprev;
    }

    arith_uint256 bnTarget = arith_uint256().SetCompact(pindex->nBits);
    if (nCountBlocks == 1) {
      bnPastTargetAvg = bnTarget;
    } else {
      bnPastTargetAvg =
          (bnPastTargetAvg * nCountBlocks + bnTarget) / (nCountBlocks + 1);
    }

    if (nCountBlocks != nPastBlocks) {
      assert(pindex->pprev);

      pindex = pindex->pprev;
    }
  }

  arith_uint256 bnNew(bnPastTargetAvg);

  int64_t nActualTimespan = pindexLast->GetBlockTime() - pindex->GetBlockTime();

  int64_t nTargetTimespan = nPastBlocks * params.nPowTargetSpacing2;

  if (nActualTimespan < nTargetTimespan / 3)
    nActualTimespan = nTargetTimespan / 3;
  if (nActualTimespan > nTargetTimespan * 3)
    nActualTimespan = nTargetTimespan * 3;

  bnNew *= nActualTimespan;
  bnNew /= nTargetTimespan;

  if (pblock->GetBlockTime() >
      pindexLast->GetBlockTime() + params.nPowTargetSpacing2 * 300) {
    bnNew = bnPowLimit2;

  }

  else if (pblock->GetBlockTime() >
           pindexLast->GetBlockTime() + params.nPowTargetSpacing2 * 250) {
    bnNew *= 100000;

  }

  else if (pblock->GetBlockTime() >
           pindexLast->GetBlockTime() + params.nPowTargetSpacing2 * 200) {
    bnNew *= 10000;

  }

  else if (pblock->GetBlockTime() >
           pindexLast->GetBlockTime() + params.nPowTargetSpacing2 * 150) {
    bnNew *= 1000;

  }

  else if (pblock->GetBlockTime() >
           pindexLast->GetBlockTime() + params.nPowTargetSpacing2 * 100) {
    bnNew *= 100;

  }

  else {
    bnNew = bnNew;
  }

  if (bnNew > bnPowLimit2) {
    bnNew = bnPowLimit2;
  }

  return bnNew.GetCompact();
}

unsigned int GetNextWorkRequiredLTC(const CBlockIndex *pindexLast,
                                    const CBlockHeader *pblock,
                                    const Consensus::Params &params) {
  assert(pindexLast != nullptr);
  unsigned int nProofOfWorkLimit = UintToArith256(params.powLimit).GetCompact();

  if ((pindexLast->nHeight + 1) % params.DifficultyAdjustmentInterval() != 0) {
    if (params.fPowAllowMinDifficultyBlocks) {
      if (pblock->GetBlockTime() >
          pindexLast->GetBlockTime() + params.nPowTargetSpacing * 2)
        return nProofOfWorkLimit;
      else {
        const CBlockIndex *pindex = pindexLast;
        while (pindex->pprev &&
               pindex->nHeight % params.DifficultyAdjustmentInterval() != 0 &&
               pindex->nBits == nProofOfWorkLimit)
          pindex = pindex->pprev;
        return pindex->nBits;
      }
    }
    return pindexLast->nBits;
  }

  int blockstogoback = params.DifficultyAdjustmentInterval() - 1;
  if ((pindexLast->nHeight + 1) != params.DifficultyAdjustmentInterval())
    blockstogoback = params.DifficultyAdjustmentInterval();

  const CBlockIndex *pindexFirst = pindexLast;
  for (int i = 0; pindexFirst && i < blockstogoback; i++)
    pindexFirst = pindexFirst->pprev;

  assert(pindexFirst);

  return CalculateNextWorkRequired(pindexLast, pindexFirst->GetBlockTime(),
                                   params);
}

unsigned int GetNextWorkRequired(const CBlockIndex *pindexLast,
                                 const CBlockHeader *pblock,
                                 const Consensus::Params &params) {
  assert(pindexLast != nullptr);

  if ((pindexLast->nHeight + 1 >= nYesPowerFork - 5) &&
      (pindexLast->nHeight + 1 < nYesPowerFork))
    return UintToArith256(params.powLimit).GetCompact();

  if ((pindexLast->nHeight + 1 >= nSpeedFork) &&
      (pindexLast->nHeight + 1 <= nSpeedFork + 5))
    return UintToArith256(params.powLimit2).GetCompact();

  if (pindexLast->nHeight + 1 >= nSpeedFork)
    return DarkGravityWave2(pindexLast, pblock, params);
  else if (pindexLast->nHeight >= params.lastScryptBlock)
    return DarkGravityWave(pindexLast, pblock, params);
  else
    return GetNextWorkRequiredLTC(pindexLast, pblock, params);
}

unsigned int CalculateNextWorkRequired(const CBlockIndex *pindexLast,
                                       int64_t nFirstBlockTime,
                                       const Consensus::Params &params) {
  if (params.fPowNoRetargeting)
    return pindexLast->nBits;

  int64_t nActualTimespan = pindexLast->GetBlockTime() - nFirstBlockTime;
  if (nActualTimespan < params.nPowTargetTimespan / 4)
    nActualTimespan = params.nPowTargetTimespan / 4;
  if (nActualTimespan > params.nPowTargetTimespan * 4)
    nActualTimespan = params.nPowTargetTimespan * 4;

  arith_uint256 bnNew;
  arith_uint256 bnOld;
  bnNew.SetCompact(pindexLast->nBits);
  bnOld = bnNew;

  const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
  bool fShift = bnNew.bits() > bnPowLimit.bits() - 1;
  if (fShift)
    bnNew >>= 1;
  bnNew *= nActualTimespan;
  bnNew /= params.nPowTargetTimespan;
  if (fShift)
    bnNew <<= 1;

  if (bnNew > bnPowLimit)
    bnNew = bnPowLimit;

  return bnNew.GetCompact();
}

bool CheckProofOfWork(uint256 hash, unsigned int nBits,
                      const Consensus::Params &params) {
  bool fNegative;
  bool fOverflow;
  arith_uint256 bnTarget;

  bnTarget.SetCompact(nBits, &fNegative, &fOverflow);

  if (fNegative || bnTarget == 0 || fOverflow ||
      bnTarget > UintToArith256(params.powLimit))
    return false;

  if (UintToArith256(hash) > bnTarget)
    return false;

  return true;
}

bool CheckProofOfWork2(uint256 hash, unsigned int nBits,
                       const Consensus::Params &params) {
  bool fNegative;
  bool fOverflow;
  arith_uint256 bnTarget;

  bnTarget.SetCompact(nBits, &fNegative, &fOverflow);

  if (fNegative || bnTarget == 0 || fOverflow ||
      bnTarget > UintToArith256(params.powLimit2))
    return false;

  if (UintToArith256(hash) > bnTarget)
    return false;

  return true;
}

unsigned int GetNextHive12WorkRequired(const CBlockIndex *pindexLast,
                                       const Consensus::Params &params) {
  const arith_uint256 bnPowLimit = UintToArith256(params.powLimitHive2);

  arith_uint256 beeHashTarget = 0;
  int hiveBlockCount = 0;
  int targetBlockCount =
      params.hiveDifficultyWindow / params.hiveBlockSpacingTarget;

  CBlockHeader block;
  for (int i = 0; i < params.hiveDifficultyWindow; i++) {
    if (!pindexLast->pprev || pindexLast->nHeight < params.minHiveCheckBlock) {
      LogPrintf(
          "GetNextHive12WorkRequired: Not enough blocks in sampling window.\n");
      return bnPowLimit.GetCompact();
    }

    block = pindexLast->GetBlockHeader();
    if (block.IsHiveMined(params)) {
      beeHashTarget += arith_uint256().SetCompact(pindexLast->nBits);
      hiveBlockCount++;
    }
    pindexLast = pindexLast->pprev;
  }

  if (hiveBlockCount == 0)
    return bnPowLimit.GetCompact();

  beeHashTarget /= hiveBlockCount;

  beeHashTarget *= targetBlockCount;
  beeHashTarget /= hiveBlockCount;

  if (beeHashTarget > bnPowLimit)
    beeHashTarget = bnPowLimit;

  return beeHashTarget.GetCompact();
}

unsigned int GetNextHiveWorkRequired(const CBlockIndex *pindexLast,
                                     const Consensus::Params &params) {
  if (IsHive12Enabled(pindexLast->nHeight))
    return GetNextHive12WorkRequired(pindexLast, params);

  const arith_uint256 bnPowLimit = UintToArith256(params.powLimitHive);
  const arith_uint256 bnPowLimit2 = UintToArith256(params.powLimitHive2);
  const arith_uint256 bnImpossible = 0;
  arith_uint256 beeHashTarget;

  numPowBlocks = 0;
  CBlockHeader block;
  while (true) {
    if (!pindexLast->pprev || pindexLast->nHeight < params.minHiveCheckBlock) {
      LogPrintf(
          "GetNextHiveWorkRequired: No hivemined blocks found in history\n");

      if ((pindexLast->nHeight + 1) >= nSpeedFork)
        return bnPowLimit2.GetCompact();
      else
        return bnPowLimit.GetCompact();
    }

    block = pindexLast->GetBlockHeader();
    if (block.IsHiveMined(params)) {
      beeHashTarget.SetCompact(block.nBits);
      break;
    }

    pindexLast = pindexLast->pprev;
    numPowBlocks++;
  }

  if (numPowBlocks == 0)
    return bnImpossible.GetCompact();

  int interval =
      params.hiveTargetAdjustAggression / params.hiveBlockSpacingTarget;
  beeHashTarget *= (interval - 1) * params.hiveBlockSpacingTarget +
                   numPowBlocks + numPowBlocks;
  beeHashTarget /= (interval + 1) * params.hiveBlockSpacingTarget;

  if ((pindexLast->nHeight + 1 >= nSpeedFork) && (beeHashTarget > bnPowLimit2))
    beeHashTarget = bnPowLimit2;
  if ((pindexLast->nHeight + 1 < nSpeedFork) && (beeHashTarget > bnPowLimit))
    beeHashTarget = bnPowLimit;

  if ((pindexLast->nHeight + 1 >= nSpeedFork) &&
      (pindexLast->nHeight + 1 <= nSpeedFork + 25))

    beeHashTarget = bnPowLimit2;

  return beeHashTarget.GetCompact();
}

bool GetNetworkHiveInfo(int &immatureBees, int &immatureBCTs, int &matureBees,
                        int &matureBCTs, CAmount &potentialLifespanRewards,
                        const Consensus::Params &consensusParams,
                        bool recalcGraph) {
  int totalBeeLifespan;

  if ((chainActive.Tip()->nHeight) >= nSpeedFork)
    totalBeeLifespan =
        consensusParams.beeLifespanBlocks3 + consensusParams.beeGestationBlocks;
  else
    totalBeeLifespan =
        consensusParams.beeLifespanBlocks + consensusParams.beeGestationBlocks;

  immatureBees = immatureBCTs = matureBees = matureBCTs = 0;

  CBlockIndex *pindexPrev = chainActive.Tip();
  assert(pindexPrev != nullptr);
  int tipHeight = pindexPrev->nHeight;

  if ((chainActive.Tip()->nHeight) >= nSpeedFork)
    potentialLifespanRewards =
        (consensusParams.beeLifespanBlocks3 *
         GetBeeCost(chainActive.Height(), consensusParams)) /
        consensusParams.hiveBlockSpacingTarget;
  else
    potentialLifespanRewards =
        (consensusParams.beeLifespanBlocks *
         GetBeeCost(chainActive.Height(), consensusParams)) /
        consensusParams.hiveBlockSpacingTarget;

  if (recalcGraph) {
    for (int i = 0; i < totalBeeLifespan; i++) {
      beePopGraph[i].immaturePop = 0;
      beePopGraph[i].maturePop = 0;
    }
  }

  if (IsInitialBlockDownload())

    return false;

  CBlock block;
  CScript scriptPubKeyBCF = GetScriptForDestination(
      DecodeDestination(consensusParams.beeCreationAddress));

  CScript scriptPubKeyCF;

  if ((chainActive.Tip()->nHeight) >= nLightFork)
    scriptPubKeyCF = GetScriptForDestination(
        DecodeDestination(consensusParams.hiveCommunityAddress2));
  else
    scriptPubKeyCF = GetScriptForDestination(
        DecodeDestination(consensusParams.hiveCommunityAddress));

  for (int i = 0; i < totalBeeLifespan; i++) {
    if (fHavePruned && !(pindexPrev->nStatus & BLOCK_HAVE_DATA) &&
        pindexPrev->nTx > 0) {
      LogPrintf("! GetNetworkHiveInfo: Warn: Block not available (pruned "
                "data); can't calculate network bee count.");
      return false;
    }

    if (!pindexPrev->GetBlockHeader().IsHiveMined(consensusParams)) {
      if (!ReadBlockFromDisk(block, pindexPrev, consensusParams)) {
        LogPrintf("! GetNetworkHiveInfo: Warn: Block not available (not found "
                  "on disk); can't calculate network bee count.");
        return false;
      }
      int blockHeight = pindexPrev->nHeight;

      CAmount beeCost =
          0.0004 * (GetBlockSubsidy(pindexPrev->nHeight, consensusParams));

      if (block.vtx.size() > 0) {
        for (const auto &tx : block.vtx) {
          CAmount beeFeePaid;
          if (tx->IsBCT(consensusParams, scriptPubKeyBCF, &beeFeePaid)) {
            if (tx->vout.size() > 1 &&
                tx->vout[1].scriptPubKey == scriptPubKeyCF) {
              CAmount donationAmount = tx->vout[1].nValue;
              CAmount expectedDonationAmount;
              if ((chainActive.Tip()->nHeight) >= nContribFork)
                expectedDonationAmount =
                    (beeFeePaid + donationAmount) /
                    consensusParams.communityContribFactor2;

              else
                expectedDonationAmount = (beeFeePaid + donationAmount) /
                                         consensusParams.communityContribFactor;

              if (donationAmount != expectedDonationAmount)
                continue;
              beeFeePaid += donationAmount;
            }
            int beeCount = beeFeePaid / beeCost;
            if (i < consensusParams.beeGestationBlocks) {
              immatureBees += beeCount;
              immatureBCTs++;
            } else {
              matureBees += beeCount;
              matureBCTs++;
            }

            if (recalcGraph) {
              int beeBornBlock = blockHeight;
              int beeMaturesBlock =
                  beeBornBlock + consensusParams.beeGestationBlocks;
              int beeDiesBlock;

              if ((chainActive.Tip()->nHeight) >= nSpeedFork)
                beeDiesBlock =
                    beeMaturesBlock + consensusParams.beeLifespanBlocks3;
              else
                beeDiesBlock =
                    beeMaturesBlock + consensusParams.beeLifespanBlocks;
              for (int j = beeBornBlock; j < beeDiesBlock; j++) {
                int graphPos = j - tipHeight;
                if (graphPos > 0 && graphPos < totalBeeLifespan) {
                  if (j < beeMaturesBlock)
                    beePopGraph[graphPos].immaturePop += beeCount;
                  else
                    beePopGraph[graphPos].maturePop += beeCount;
                }
              }
            }
          }
        }
      }
    }

    if (!pindexPrev->pprev)

      return true;

    pindexPrev = pindexPrev->pprev;
  }

  return true;
}

bool GetNetworkHiveInfo2(int &immatureBees, int &immatureBCTs, int &matureBees,
                         int &matureBCTs, CAmount &potentialLifespanRewards,
                         const Consensus::Params &consensusParams,
                         bool recalcGraph) {
  int totalBeeLifespan =
      consensusParams.beeLifespanBlocks + consensusParams.beeGestationBlocks;
  int beesDying = 0;

  CBlockIndex *pindexTip = chainActive.Tip();
  int tipHeight = pindexTip->nHeight;

  CBlockIndex *pindexPrev;

  if (consensusParams.isTestnet == true)
    pindexPrev = chainActive.TipMinusLifespanT();
  else
    pindexPrev = chainActive.TipMinusLifespan();

  assert(pindexPrev != nullptr);

  potentialLifespanRewards =
      (consensusParams.beeLifespanBlocks *
       GetBlockSubsidy(pindexPrev->nHeight, consensusParams)) /
      consensusParams.hiveBlockSpacingTarget;

  int forkHeight = consensusParams.variableForkBlock;

  if (IsInitialBlockDownload())

    return false;

  CBlock block;
  CScript scriptPubKeyBCF = GetScriptForDestination(
      DecodeDestination(consensusParams.beeCreationAddress));
  CScript scriptPubKeyCF = GetScriptForDestination(
      DecodeDestination(consensusParams.hiveCommunityAddress));

  if (consensusParams.isTestnet == true) {
    priceState = 0;
    immatureBees = 0;
    immatureBCTs = 0;
    matureBees = 0;
    matureBCTs = 0;
  } else {
    priceState = 0;
    immatureBees = 8702499;
    immatureBCTs = 3;
    matureBees = 14359051;
    matureBCTs = 79;
  }

  if (recalcGraph) {
    for (int i = 0; i < totalBeeLifespan; i++) {
      beePopGraph[i].immaturePop = 0;
      beePopGraph[i].maturePop = 0;
    }
  }

  if ((tipHeight - totalBeeLifespan) < forkHeight) {
    for (int i = (tipHeight - totalBeeLifespan); i < forkHeight; i++) {
      if (fHavePruned && !(pindexPrev->nStatus & BLOCK_HAVE_DATA) &&
          pindexPrev->nTx > 0) {
        LogPrintf("! GetNetworkHiveInfo: Warn: Block not available (pruned "
                  "data); can't calculate network bee count.");
        return false;
      }

      if (!pindexPrev->GetBlockHeader().IsHiveMined(consensusParams)) {
        if (!ReadBlockFromDisk(block, pindexPrev, consensusParams)) {
          LogPrintf("! GetNetworkHiveInfo: Warn: Block not available (not "
                    "found on disk); can't calculate network bee count.");
          return false;
        }
        int blockHeight = pindexPrev->nHeight;
        CAmount beeCost =
            0.0004 * (GetBlockSubsidy(pindexPrev->nHeight, consensusParams));

        if (block.vtx.size() > 0) {
          for (const auto &tx : block.vtx) {
            CAmount beeFeePaid;
            if (tx->IsBCT(consensusParams, scriptPubKeyBCF, &beeFeePaid)) {
              if (tx->vout.size() > 1 &&
                  tx->vout[1].scriptPubKey == scriptPubKeyCF) {
                CAmount donationAmount = tx->vout[1].nValue;
                CAmount expectedDonationAmount;
                if ((chainActive.Tip()->nHeight) >= nContribFork)
                  expectedDonationAmount =
                      (beeFeePaid + donationAmount) /
                      consensusParams.communityContribFactor2;

                else
                  expectedDonationAmount =
                      (beeFeePaid + donationAmount) /
                      consensusParams.communityContribFactor;

                if (donationAmount != expectedDonationAmount)
                  continue;
                beeFeePaid += donationAmount;
              }

              int beeCount = beeFeePaid / beeCost;

              if (recalcGraph) {
                int beeBornBlock = blockHeight;
                int beeMaturesBlock =
                    beeBornBlock + consensusParams.beeGestationBlocks;
                int beeDiesBlock =
                    beeMaturesBlock + consensusParams.beeLifespanBlocks;
                for (int j = beeBornBlock; j < beeDiesBlock; j++) {
                  int graphPos = j - tipHeight;
                  if (graphPos > 0 && graphPos < totalBeeLifespan) {
                    if (j < beeMaturesBlock)
                      beePopGraph[graphPos].immaturePop += beeCount;
                    else
                      beePopGraph[graphPos].maturePop += beeCount;
                  }
                }
              }
            }
          }
        }
      }

      if (!chainActive.Next(pindexPrev))

        return true;

      pindexPrev = chainActive.Next(pindexPrev);
    }
  }

  if (consensusParams.isTestnet == true)
    pindexPrev = chainActive.varForkBlocktestnet();
  else
    pindexPrev = chainActive.varForkBlock();

  assert(pindexPrev != nullptr);

  for (int i = forkHeight; i < tipHeight; i++) {
    if (fHavePruned && !(pindexPrev->nStatus & BLOCK_HAVE_DATA) &&
        pindexPrev->nTx > 0) {
      LogPrintf("! GetNetworkHiveInfo: Warn: Block not available (pruned "
                "data); can't calculate network bee count.");
      return false;
    }

    if (!pindexPrev->GetBlockHeader().IsHiveMined(consensusParams)) {
      if (!ReadBlockFromDisk(block, pindexPrev, consensusParams)) {
        LogPrintf("! GetNetworkHiveInfo: Warn: Block not available (not found "
                  "on disk); can't calculate network bee count.");
        return false;
      }
      int blockHeight = pindexPrev->nHeight;
      CAmount beeCost;

      if (priceState == 0)
        beeCost =
            0.0004 * (GetBlockSubsidy(pindexPrev->nHeight, consensusParams));
      else
        beeCost =
            0.0008 * (GetBlockSubsidy(pindexPrev->nHeight, consensusParams));

      if (block.vtx.size() > 0) {
        for (const auto &tx : block.vtx) {
          CAmount beeFeePaid;
          if (tx->IsBCT(consensusParams, scriptPubKeyBCF, &beeFeePaid)) {
            if (tx->vout.size() > 1 &&
                tx->vout[1].scriptPubKey == scriptPubKeyCF) {
              CAmount donationAmount = tx->vout[1].nValue;
              CAmount expectedDonationAmount;
              if ((chainActive.Tip()->nHeight) >= nContribFork)
                expectedDonationAmount =
                    (beeFeePaid + donationAmount) /
                    consensusParams.communityContribFactor2;

              else
                expectedDonationAmount = (beeFeePaid + donationAmount) /
                                         consensusParams.communityContribFactor;

              if (donationAmount != expectedDonationAmount)
                continue;
              beeFeePaid += donationAmount;
            }
            int beeCount = beeFeePaid / beeCost;

            immatureBees += beeCount;
            immatureBCTs++;

            if (recalcGraph) {
              int beeBornBlock = blockHeight;
              int beeMaturesBlock =
                  beeBornBlock + consensusParams.beeGestationBlocks;
              int beeDiesBlock =
                  beeMaturesBlock + consensusParams.beeLifespanBlocks;
              for (int j = beeBornBlock; j < beeDiesBlock; j++) {
                int graphPos = j - tipHeight;
                if (graphPos > 0 && graphPos < totalBeeLifespan) {
                  if (j < beeMaturesBlock) {
                    beePopGraph[graphPos].immaturePop += beeCount;
                  } else {
                    beePopGraph[graphPos].maturePop += beeCount;
                  }
                }
              }
            }
          }
        }
      }
    }

    if (consensusParams.isTestnet == true) {
      if (fHavePruned &&
          !(chainActive.Back24testnet(pindexPrev)->nStatus & BLOCK_HAVE_DATA) &&
          chainActive.Back24testnet(pindexPrev)->nTx > 0) {
        LogPrintf("! GetNetworkHiveInfo: Warn: Block not available (pruned "
                  "data); can't calculate network bee count.");
        return false;
      }

      if (!(chainActive.Back24testnet(pindexPrev)
                ->GetBlockHeader()
                .IsHiveMined(consensusParams))) {
        if (!ReadBlockFromDisk(block, chainActive.Back24testnet(pindexPrev),
                               consensusParams)) {
          LogPrintf("! GetNetworkHiveInfo: Warn: Block not available (not "
                    "found on disk); can't calculate network bee count.");
          return false;
        }

        CAmount beeCost;

        if (block.vtx.size() > 0) {
          for (const auto &tx : block.vtx) {
            CAmount beeFeePaid;
            if (tx->IsBCT(consensusParams, scriptPubKeyBCF, &beeFeePaid)) {
              if (tx->vout.size() > 1 &&
                  tx->vout[1].scriptPubKey == scriptPubKeyCF) {
                CAmount donationAmount = tx->vout[1].nValue;
                CAmount expectedDonationAmount;
                if ((chainActive.Tip()->nHeight) >= nContribFork)
                  expectedDonationAmount =
                      (beeFeePaid + donationAmount) /
                      consensusParams.communityContribFactor2;

                else
                  expectedDonationAmount =
                      (beeFeePaid + donationAmount) /
                      consensusParams.communityContribFactor;

                if (donationAmount != expectedDonationAmount)
                  continue;
                beeFeePaid += donationAmount;
              }

              int maturingbeesCreationTimetestnet =
                  (chainActive.Back24testnet(pindexPrev))->GetBlockTime();

              if (((maturingbeesCreationTimetestnet > switchLmem) &&
                   (switchLmem > switchHmem)) ||
                  ((switchLmem > switchHmem) &&
                   (maturingbeesCreationTimetestnet < switchHmem)) ||
                  ((switchHmem > switchLmem) &&
                   ((maturingbeesCreationTimetestnet > switchLmem) &&
                    (maturingbeesCreationTimetestnet <= switchHmem))) ||
                  (!(switchHmem))) {
                beeCost = 0.0004 *
                          (GetBlockSubsidy(
                              (chainActive.Back24testnet(pindexPrev))->nHeight,
                              consensusParams));

              } else {
                beeCost = 0.0008 *
                          (GetBlockSubsidy(
                              (chainActive.Back24testnet(pindexPrev))->nHeight,
                              consensusParams));
              }

              int beeCount = beeFeePaid / beeCost;

              immatureBees -= beeCount;
              immatureBCTs--;

              matureBees += beeCount;

              matureBCTs++;
            }
          }
        }
      }
    }

    if (consensusParams.isTestnet == false) {
      if (fHavePruned &&
          !(chainActive.Back24(pindexPrev)->nStatus & BLOCK_HAVE_DATA) &&
          chainActive.Back24(pindexPrev)->nTx > 0) {
        LogPrintf("! GetNetworkHiveInfo: Warn: Block not available (pruned "
                  "data); can't calculate network bee count.");
        return false;
      }

      if (!(chainActive.Back24(pindexPrev)
                ->GetBlockHeader()
                .IsHiveMined(consensusParams))) {
        if (!ReadBlockFromDisk(block, chainActive.Back24(pindexPrev),
                               consensusParams)) {
          LogPrintf("! GetNetworkHiveInfo: Warn: Block not available (not "
                    "found on disk); can't calculate network bee count.");
          return false;
        }

        CAmount beeCost;

        if (block.vtx.size() > 0) {
          for (const auto &tx : block.vtx) {
            CAmount beeFeePaid;
            if (tx->IsBCT(consensusParams, scriptPubKeyBCF, &beeFeePaid)) {
              if (tx->vout.size() > 1 &&
                  tx->vout[1].scriptPubKey == scriptPubKeyCF) {
                CAmount donationAmount = tx->vout[1].nValue;
                CAmount expectedDonationAmount;
                if ((chainActive.Tip()->nHeight) >= nContribFork)
                  expectedDonationAmount =
                      (beeFeePaid + donationAmount) /
                      consensusParams.communityContribFactor2;

                else
                  expectedDonationAmount =
                      (beeFeePaid + donationAmount) /
                      consensusParams.communityContribFactor;

                if (donationAmount != expectedDonationAmount)
                  continue;
                beeFeePaid += donationAmount;
              }

              int maturingbeesCreationTime =
                  (chainActive.Back24(pindexPrev))->GetBlockTime();

              if (((maturingbeesCreationTime > switchLmem) &&
                   (switchLmem > switchHmem)) ||
                  ((switchLmem > switchHmem) &&
                   (maturingbeesCreationTime < switchHmem)) ||
                  ((switchHmem > switchLmem) &&
                   ((maturingbeesCreationTime > switchLmem) &&
                    (maturingbeesCreationTime <= switchHmem))) ||
                  (!(switchHmem))) {
                beeCost =
                    0.0004 *
                    (GetBlockSubsidy((chainActive.Back24(pindexPrev))->nHeight,
                                     consensusParams));

              } else {
                beeCost =
                    0.0008 *
                    (GetBlockSubsidy((chainActive.Back24(pindexPrev))->nHeight,
                                     consensusParams));
              }

              int beeCount = beeFeePaid / beeCost;

              immatureBees -= beeCount;
              immatureBCTs--;

              matureBees += beeCount;

              matureBCTs++;
            }
          }
        }
      }
    }

    if (consensusParams.isTestnet == false) {
      if (fHavePruned &&
          !((chainActive.Back(pindexPrev))->nStatus & BLOCK_HAVE_DATA) &&
          (chainActive.Back(pindexPrev))->nTx > 0) {
        LogPrintf("! GetNetworkHiveInfo: Warn: Block not available (pruned "
                  "data); can't calculate network bee count.");
        return false;
      }

      if ((!(chainActive.Back(pindexPrev))
                ->GetBlockHeader()
                .IsHiveMined(consensusParams))) {
        if (!ReadBlockFromDisk(block, (chainActive.Back(pindexPrev)),
                               consensusParams)) {
          LogPrintf("! GetNetworkHiveInfo: Warn: Block not available (not "
                    "found on disk); can't calculate network bee count.");
          return false;
        }

        if (block.vtx.size() > 0) {
          for (const auto &tx : block.vtx) {
            CAmount beeFeePaidX;
            if (tx->IsBCT(consensusParams, scriptPubKeyBCF, &beeFeePaidX)) {
              if (tx->vout.size() > 1 &&
                  tx->vout[1].scriptPubKey == scriptPubKeyCF) {
                CAmount donationAmountX = tx->vout[1].nValue;
                CAmount expectedDonationAmountX;
                if ((chainActive.Tip()->nHeight) >= nContribFork)
                  expectedDonationAmountX =
                      (beeFeePaidX + donationAmountX) /
                      consensusParams.communityContribFactor2;

                else
                  expectedDonationAmountX =
                      (beeFeePaidX + donationAmountX) /
                      consensusParams.communityContribFactor;

                if (donationAmountX != expectedDonationAmountX)
                  continue;
                beeFeePaidX += donationAmountX;
              }

              int dyingbeesCreationTime =
                  (chainActive.Back(pindexPrev))->GetBlockTime();

              CAmount beeCostX;

              if (((dyingbeesCreationTime > switchLmem) &&
                   (switchLmem > switchHmem)) ||
                  ((switchLmem > switchHmem) &&
                   (dyingbeesCreationTime < switchHmem)) ||
                  ((switchHmem > switchLmem) &&
                   ((dyingbeesCreationTime > switchLmem) &&
                    (dyingbeesCreationTime <= switchHmem))) ||
                  (!(switchHmem))) {
                beeCostX =
                    0.0004 *
                    (GetBlockSubsidy((chainActive.Back(pindexPrev))->nHeight,
                                     consensusParams));

              } else {
                beeCostX =
                    0.0008 *
                    (GetBlockSubsidy((chainActive.Back(pindexPrev))->nHeight,
                                     consensusParams));
              }

              int beeCountZ = beeFeePaidX / beeCostX;
              beesDying += beeCountZ;

              matureBees -= beeCountZ;

              matureBCTs--;
            }
          }
        }
      }
    }

    if (consensusParams.isTestnet == true) {
      if (fHavePruned &&
          !((chainActive.Backtestnet(pindexPrev))->nStatus & BLOCK_HAVE_DATA) &&
          (chainActive.Backtestnet(pindexPrev))->nTx > 0) {
        LogPrintf("! GetNetworkHiveInfo: Warn: Block not available (pruned "
                  "data); can't calculate network bee count.");
        return false;
      }

      if ((!(chainActive.Backtestnet(pindexPrev))
                ->GetBlockHeader()
                .IsHiveMined(consensusParams))) {
        if (!ReadBlockFromDisk(block, (chainActive.Backtestnet(pindexPrev)),
                               consensusParams)) {
          LogPrintf("! GetNetworkHiveInfo: Warn: Block not available (not "
                    "found on disk); can't calculate network bee count.");
          return false;
        }

        if (block.vtx.size() > 0) {
          for (const auto &tx : block.vtx) {
            CAmount beeFeePaidX;
            if (tx->IsBCT(consensusParams, scriptPubKeyBCF, &beeFeePaidX)) {
              if (tx->vout.size() > 1 &&
                  tx->vout[1].scriptPubKey == scriptPubKeyCF) {
                CAmount donationAmountX = tx->vout[1].nValue;
                CAmount expectedDonationAmountX;
                if ((chainActive.Tip()->nHeight) >= nContribFork)
                  expectedDonationAmountX =
                      (beeFeePaidX + donationAmountX) /
                      consensusParams.communityContribFactor2;

                else
                  expectedDonationAmountX =
                      (beeFeePaidX + donationAmountX) /
                      consensusParams.communityContribFactor;

                if (donationAmountX != expectedDonationAmountX)
                  continue;
                beeFeePaidX += donationAmountX;
              }

              int dyingbeesCreationTime =
                  (chainActive.Backtestnet(pindexPrev))->GetBlockTime();

              CAmount beeCostX;

              if (((dyingbeesCreationTime > switchLmem) &&
                   (switchLmem > switchHmem)) ||
                  ((switchLmem > switchHmem) &&
                   (dyingbeesCreationTime < switchHmem)) ||
                  ((switchHmem > switchLmem) &&
                   ((dyingbeesCreationTime > switchLmem) &&
                    (dyingbeesCreationTime <= switchHmem))) ||
                  (!(switchHmem))) {
                beeCostX =
                    0.0004 * (GetBlockSubsidy(
                                 (chainActive.Backtestnet(pindexPrev))->nHeight,
                                 consensusParams));

              } else {
                beeCostX =
                    0.0008 * (GetBlockSubsidy(
                                 (chainActive.Backtestnet(pindexPrev))->nHeight,
                                 consensusParams));
              }

              int beeCountZ = beeFeePaidX / beeCostX;

              beesDying += beeCountZ;

              matureBees -= beeCountZ;

              matureBCTs--;

              int testing = pindexPrev->nHeight;
              LogPrintf("For Height %i , %i bees dying \n", testing, beeCountZ);
              LogPrintf(
                  "                                                      %i \n",
                  matureBees);
            }
          }
        }
      }
    }

    int basebeeCost =
        0.0004 * (GetBlockSubsidy(pindexPrev->nHeight, consensusParams));
    threshold = ((potentialLifespanRewards / basebeeCost) * 0.9);

    totalMatureBees = matureBees;

    thematurebees = matureBees;
    wototo = matureBees;

    if ((matureBees > threshold) && (priceState == 0)) {
      priceState = 1;
      int switchHigher = pindexPrev->GetBlockTime();
      switchHmem = switchHigher;
    }

    if ((matureBees <= threshold) && (priceState == 1)) {
      priceState = 0;
      int switchLower = pindexPrev->GetBlockTime();
      switchLmem = switchLower;
    }

    if (!chainActive.Next(pindexPrev))

      return true;

    pindexPrev = chainActive.Next(pindexPrev);
  }

  return true;
}

bool GetNetworkHiveInfo3(int &immatureBees, int &immatureBCTs, int &matureBees,
                         int &matureBCTs, CAmount &potentialLifespanRewards,
                         const Consensus::Params &consensusParams,
                         bool recalcGraph) {
  int totalBeeLifespan2 =
      consensusParams.beeLifespanBlocks2 + consensusParams.beeGestationBlocks;
  int totalBeeLifespan =
      consensusParams.beeLifespanBlocks + consensusParams.beeGestationBlocks;
  int beesDying = 0;

  CBlockIndex *pindexTip = chainActive.Tip();

  int tipHeight = pindexTip->nHeight;

  if (firstRun == 0) {
    remTipHeight = tipHeight - 10;
    pindexRem = chainActive.TipMinusTen();
  }

  CBlockIndex *pindexPrev;

  if (consensusParams.isTestnet == true)
    pindexPrev = chainActive.varForkBlocktestnet();
  else
    pindexPrev = chainActive.varForkBlock();

  assert(pindexPrev != nullptr);

  potentialLifespanRewards =
      (consensusParams.beeLifespanBlocks2 *
       GetBlockSubsidy(pindexPrev->nHeight, consensusParams)) /
      consensusParams.hiveBlockSpacingTarget;

  CAmount potentialLifespanRewards2 =
      (consensusParams.beeLifespanBlocks *
       GetBlockSubsidy(pindexPrev->nHeight, consensusParams)) /
      consensusParams.hiveBlockSpacingTarget;

  if (IsInitialBlockDownload())

    return false;

  CBlock block;
  CScript scriptPubKeyBCF = GetScriptForDestination(
      DecodeDestination(consensusParams.beeCreationAddress));
  CScript scriptPubKeyCF = GetScriptForDestination(
      DecodeDestination(consensusParams.hiveCommunityAddress));

  if (firstRun == 0) {
    if (consensusParams.isTestnet == true) {
      priceState = 0;
      immatureBees = 0;
      immatureBCTs = 0;
      matureBees = 0;
      matureBCTs = 0;
    }

    else {
      priceState = 0;
      immatureBees = 8702499;
      immatureBCTs = 3;
      matureBees = 14359051;
      matureBCTs = 79;
    }
  }

  if ((recalcGraph) && (firstRun == 0)) {
    for (int i = 0; i < totalBeeLifespan2; i++) {
      beePopGraph[i].immaturePop = 0;
      beePopGraph[i].maturePop = 0;
    }
  }

  if (firstRun == 0) {
    for (int i = 67777; i < remTipHeight; i++) {
      if (fHavePruned && !(pindexPrev->nStatus & BLOCK_HAVE_DATA) &&
          pindexPrev->nTx > 0) {
        LogPrintf("! GetNetworkHiveInfo: Warn: Block not available (pruned "
                  "data); can't calculate network bee count.");
        return false;
      }

      if (!pindexPrev->GetBlockHeader().IsHiveMined(consensusParams)) {
        if (!ReadBlockFromDisk(block, pindexPrev, consensusParams)) {
          LogPrintf("! GetNetworkHiveInfo: Warn: Block not available (not "
                    "found on disk); can't calculate network bee count.");
          return false;
        }
        int blockHeight = pindexPrev->nHeight;
        CAmount beeCost;

        if (priceState == 0)
          beeCost =
              0.0004 * (GetBlockSubsidy(pindexPrev->nHeight, consensusParams));
        else
          beeCost =
              0.0008 * (GetBlockSubsidy(pindexPrev->nHeight, consensusParams));

        if (block.vtx.size() > 0) {
          for (const auto &tx : block.vtx) {
            CAmount beeFeePaid;
            if (tx->IsBCT(consensusParams, scriptPubKeyBCF, &beeFeePaid)) {
              if (tx->vout.size() > 1 &&
                  tx->vout[1].scriptPubKey == scriptPubKeyCF) {
                CAmount donationAmount = tx->vout[1].nValue;
                CAmount expectedDonationAmount;
                if ((chainActive.Tip()->nHeight) >= nContribFork)
                  expectedDonationAmount =
                      (beeFeePaid + donationAmount) /
                      consensusParams.communityContribFactor2;

                else
                  expectedDonationAmount =
                      (beeFeePaid + donationAmount) /
                      consensusParams.communityContribFactor;

                if (donationAmount != expectedDonationAmount)
                  continue;
                beeFeePaid += donationAmount;
              }
              int beeCount = beeFeePaid / beeCost;

              immatureBees += beeCount;
              immatureBCTs++;

              int testing = pindexPrev->nHeight;
              LogPrintf("For Height %i , %i bees created \n", testing,
                        beeCount);

              if (recalcGraph) {
                if (i < consensusParams.ratioForkBlock) {
                  int beeBornBlock = blockHeight;
                  int beeMaturesBlock =
                      beeBornBlock + consensusParams.beeGestationBlocks;
                  int beeDiesBlock =
                      beeMaturesBlock + consensusParams.beeLifespanBlocks;
                  for (int j = beeBornBlock; j < beeDiesBlock; j++) {
                    int graphPos = j - tipHeight;
                    if (graphPos > 0 && graphPos < totalBeeLifespan) {
                      if (j < beeMaturesBlock) {
                        beePopGraph[graphPos].immaturePop += beeCount;

                      } else {
                        beePopGraph[graphPos].maturePop += beeCount;
                      }
                    }
                  }

                } else {
                  int beeBornBlock = blockHeight;
                  int beeMaturesBlock =
                      beeBornBlock + consensusParams.beeGestationBlocks;
                  int beeDiesBlock =
                      beeMaturesBlock + consensusParams.beeLifespanBlocks2;
                  for (int j = beeBornBlock; j < beeDiesBlock; j++) {
                    int graphPos = j - tipHeight;
                    if (graphPos > 0 && graphPos < totalBeeLifespan2) {
                      if (j < beeMaturesBlock) {
                        beePopGraph[graphPos].immaturePop += beeCount;

                      } else {
                        beePopGraph[graphPos].maturePop += beeCount;
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }

      if (consensusParams.isTestnet == true) {
        if (fHavePruned &&
            !(chainActive.Back24testnet(pindexPrev)->nStatus &
              BLOCK_HAVE_DATA) &&
            chainActive.Back24testnet(pindexPrev)->nTx > 0) {
          LogPrintf("! GetNetworkHiveInfo: Warn: Block not available (pruned "
                    "data); can't calculate network bee count.");
          return false;
        }

        if (!(chainActive.Back24testnet(pindexPrev)
                  ->GetBlockHeader()
                  .IsHiveMined(consensusParams))) {
          if (!ReadBlockFromDisk(block, chainActive.Back24testnet(pindexPrev),
                                 consensusParams)) {
            LogPrintf("! GetNetworkHiveInfo: Warn: Block not available (not "
                      "found on disk); can't calculate network bee count.");
            return false;
          }

          CAmount beeCost;

          if (block.vtx.size() > 0) {
            for (const auto &tx : block.vtx) {
              CAmount beeFeePaid;
              if (tx->IsBCT(consensusParams, scriptPubKeyBCF, &beeFeePaid)) {
                if (tx->vout.size() > 1 &&
                    tx->vout[1].scriptPubKey == scriptPubKeyCF) {
                  CAmount donationAmount = tx->vout[1].nValue;
                  CAmount expectedDonationAmount;
                  if ((chainActive.Tip()->nHeight) >= nContribFork)
                    expectedDonationAmount =
                        (beeFeePaid + donationAmount) /
                        consensusParams.communityContribFactor2;

                  else
                    expectedDonationAmount =
                        (beeFeePaid + donationAmount) /
                        consensusParams.communityContribFactor;

                  if (donationAmount != expectedDonationAmount)
                    continue;
                  beeFeePaid += donationAmount;
                }

                int maturingbeesCreationTimetestnet =
                    (chainActive.Back24testnet(pindexPrev))->GetBlockTime();

                if (((maturingbeesCreationTimetestnet > switchLmem) &&
                     (switchLmem > switchHmem)) ||
                    ((switchLmem > switchHmem) &&
                     (maturingbeesCreationTimetestnet < switchHmem)) ||
                    ((switchHmem > switchLmem) &&
                     ((maturingbeesCreationTimetestnet > switchLmem) &&
                      (maturingbeesCreationTimetestnet <= switchHmem))) ||
                    (!(switchHmem))) {
                  beeCost =
                      0.0004 *
                      (GetBlockSubsidy(
                          (chainActive.Back24testnet(pindexPrev))->nHeight,
                          consensusParams));

                } else {
                  beeCost =
                      0.0008 *
                      (GetBlockSubsidy(
                          (chainActive.Back24testnet(pindexPrev))->nHeight,
                          consensusParams));
                }

                int beeCount = beeFeePaid / beeCost;

                immatureBees -= beeCount;
                immatureBCTs--;

                matureBees += beeCount;

                matureBCTs++;
              }
            }
          }
        }
      }

      if (consensusParams.isTestnet == false) {
        if (fHavePruned &&
            !(chainActive.Back24(pindexPrev)->nStatus & BLOCK_HAVE_DATA) &&
            chainActive.Back24(pindexPrev)->nTx > 0) {
          LogPrintf("! GetNetworkHiveInfo: Warn: Block not available (pruned "
                    "data); can't calculate network bee count.");
          return false;
        }

        if (!(chainActive.Back24(pindexPrev)
                  ->GetBlockHeader()
                  .IsHiveMined(consensusParams))) {
          if (!ReadBlockFromDisk(block, chainActive.Back24(pindexPrev),
                                 consensusParams)) {
            LogPrintf("! GetNetworkHiveInfo: Warn: Block not available (not "
                      "found on disk); can't calculate network bee count.");
            return false;
          }

          CAmount beeCost;

          if (block.vtx.size() > 0) {
            for (const auto &tx : block.vtx) {
              CAmount beeFeePaid;
              if (tx->IsBCT(consensusParams, scriptPubKeyBCF, &beeFeePaid)) {
                if (tx->vout.size() > 1 &&
                    tx->vout[1].scriptPubKey == scriptPubKeyCF) {
                  CAmount donationAmount = tx->vout[1].nValue;
                  CAmount expectedDonationAmount;
                  if ((chainActive.Tip()->nHeight) >= nContribFork)
                    expectedDonationAmount =
                        (beeFeePaid + donationAmount) /
                        consensusParams.communityContribFactor2;

                  else
                    expectedDonationAmount =
                        (beeFeePaid + donationAmount) /
                        consensusParams.communityContribFactor;

                  if (donationAmount != expectedDonationAmount)
                    continue;
                  beeFeePaid += donationAmount;
                }

                int maturingbeesCreationTime =
                    (chainActive.Back24(pindexPrev))->GetBlockTime();

                if (((maturingbeesCreationTime > switchLmem) &&
                     (switchLmem > switchHmem)) ||
                    ((switchLmem > switchHmem) &&
                     (maturingbeesCreationTime < switchHmem)) ||
                    ((switchHmem > switchLmem) &&
                     ((maturingbeesCreationTime > switchLmem) &&
                      (maturingbeesCreationTime <= switchHmem))) ||
                    (!(switchHmem))) {
                  beeCost =
                      0.0004 * (GetBlockSubsidy(
                                   (chainActive.Back24(pindexPrev))->nHeight,
                                   consensusParams));

                } else {
                  beeCost =
                      0.0008 * (GetBlockSubsidy(
                                   (chainActive.Back24(pindexPrev))->nHeight,
                                   consensusParams));
                }

                int beeCount = beeFeePaid / beeCost;

                immatureBees -= beeCount;
                immatureBCTs--;

                matureBees += beeCount;

                matureBCTs++;
              }
            }
          }
        }
      }

      if (consensusParams.isTestnet == false) {
        if (i < consensusParams.ratioForkBlock + totalBeeLifespan) {
          if (fHavePruned &&
              !((chainActive.Back(pindexPrev))->nStatus & BLOCK_HAVE_DATA) &&
              (chainActive.Back(pindexPrev))->nTx > 0) {
            LogPrintf("! GetNetworkHiveInfo: Warn: Block not available (pruned "
                      "data); can't calculate network bee count.");
            return false;
          }

          if ((!(chainActive.Back(pindexPrev))
                    ->GetBlockHeader()
                    .IsHiveMined(consensusParams))) {
            if (!ReadBlockFromDisk(block, (chainActive.Back(pindexPrev)),
                                   consensusParams)) {
              LogPrintf("! GetNetworkHiveInfo: Warn: Block not available (not "
                        "found on disk); can't calculate network bee count.");
              return false;
            }

            if (block.vtx.size() > 0) {
              for (const auto &tx : block.vtx) {
                CAmount beeFeePaidX;
                if (tx->IsBCT(consensusParams, scriptPubKeyBCF, &beeFeePaidX)) {
                  if (tx->vout.size() > 1 &&
                      tx->vout[1].scriptPubKey == scriptPubKeyCF) {
                    CAmount donationAmountX = tx->vout[1].nValue;
                    CAmount expectedDonationAmountX;
                    if ((chainActive.Tip()->nHeight) >= nContribFork)
                      expectedDonationAmountX =
                          (beeFeePaidX + donationAmountX) /
                          consensusParams.communityContribFactor2;

                    else
                      expectedDonationAmountX =
                          (beeFeePaidX + donationAmountX) /
                          consensusParams.communityContribFactor;

                    if (donationAmountX != expectedDonationAmountX)
                      continue;
                    beeFeePaidX += donationAmountX;
                  }

                  int dyingbeesCreationTime =
                      (chainActive.Back(pindexPrev))->GetBlockTime();

                  CAmount beeCostX;

                  if (((dyingbeesCreationTime > switchLmem) &&
                       (switchLmem > switchHmem)) ||
                      ((switchLmem > switchHmem) &&
                       (dyingbeesCreationTime < switchHmem)) ||
                      ((switchHmem > switchLmem) &&
                       ((dyingbeesCreationTime > switchLmem) &&
                        (dyingbeesCreationTime <= switchHmem))) ||
                      (!(switchHmem))) {
                    beeCostX =
                        0.0004 * (GetBlockSubsidy(
                                     (chainActive.Back(pindexPrev))->nHeight,
                                     consensusParams));

                  } else {
                    beeCostX =
                        0.0008 * (GetBlockSubsidy(
                                     (chainActive.Back(pindexPrev))->nHeight,
                                     consensusParams));
                  }

                  int beeCountZ = beeFeePaidX / beeCostX;
                  beesDying += beeCountZ;

                  matureBees -= beeCountZ;

                  matureBCTs--;
                }
              }
            }
          }
        }

        if (i >= (consensusParams.ratioForkBlock + totalBeeLifespan2)) {
          if (fHavePruned &&
              !((chainActive.ReBack(pindexPrev))->nStatus & BLOCK_HAVE_DATA) &&
              (chainActive.ReBack(pindexPrev))->nTx > 0) {
            LogPrintf("! GetNetworkHiveInfo: Warn: Block not available (pruned "
                      "data); can't calculate network bee count.");
            return false;
          }

          if ((!(chainActive.ReBack(pindexPrev))
                    ->GetBlockHeader()
                    .IsHiveMined(consensusParams))) {
            if (!ReadBlockFromDisk(block, (chainActive.ReBack(pindexPrev)),
                                   consensusParams)) {
              LogPrintf("! GetNetworkHiveInfo: Warn: Block not available (not "
                        "found on disk); can't calculate network bee count.");
              return false;
            }

            if (block.vtx.size() > 0) {
              for (const auto &tx : block.vtx) {
                CAmount beeFeePaidX;
                if (tx->IsBCT(consensusParams, scriptPubKeyBCF, &beeFeePaidX)) {
                  if (tx->vout.size() > 1 &&
                      tx->vout[1].scriptPubKey == scriptPubKeyCF) {
                    CAmount donationAmountX = tx->vout[1].nValue;
                    CAmount expectedDonationAmountX;
                    if ((chainActive.Tip()->nHeight) >= nContribFork)
                      expectedDonationAmountX =
                          (beeFeePaidX + donationAmountX) /
                          consensusParams.communityContribFactor2;

                    else
                      expectedDonationAmountX =
                          (beeFeePaidX + donationAmountX) /
                          consensusParams.communityContribFactor;

                    if (donationAmountX != expectedDonationAmountX)
                      continue;
                    beeFeePaidX += donationAmountX;
                  }

                  int dyingbeesCreationTime =
                      (chainActive.ReBack(pindexPrev))->GetBlockTime();

                  CAmount beeCostX;

                  if (((dyingbeesCreationTime > switchLmem) &&
                       (switchLmem > switchHmem)) ||
                      ((switchLmem > switchHmem) &&
                       (dyingbeesCreationTime < switchHmem)) ||
                      ((switchHmem > switchLmem) &&
                       ((dyingbeesCreationTime > switchLmem) &&
                        (dyingbeesCreationTime <= switchHmem))) ||
                      (!(switchHmem))) {
                    beeCostX =
                        0.0004 * (GetBlockSubsidy(
                                     (chainActive.ReBack(pindexPrev))->nHeight,
                                     consensusParams));

                  } else {
                    beeCostX =
                        0.0008 * (GetBlockSubsidy(
                                     (chainActive.ReBack(pindexPrev))->nHeight,
                                     consensusParams));
                  }

                  int beeCountZ = beeFeePaidX / beeCostX;
                  beesDying += beeCountZ;

                  matureBees -= beeCountZ;

                  matureBCTs--;
                }
              }
            }
          }
        }
      }

      if (consensusParams.isTestnet == true) {
        if (i < consensusParams.ratioForkBlock + totalBeeLifespan) {
          if (fHavePruned &&
              !((chainActive.Backtestnet(pindexPrev))->nStatus &
                BLOCK_HAVE_DATA) &&
              (chainActive.Backtestnet(pindexPrev))->nTx > 0) {
            LogPrintf("! GetNetworkHiveInfo: Warn: Block not available (pruned "
                      "data); can't calculate network bee count.");
            return false;
          }

          if ((!(chainActive.Backtestnet(pindexPrev))
                    ->GetBlockHeader()
                    .IsHiveMined(consensusParams))) {
            if (!ReadBlockFromDisk(block, (chainActive.Backtestnet(pindexPrev)),
                                   consensusParams)) {
              LogPrintf("! GetNetworkHiveInfo: Warn: Block not available (not "
                        "found on disk); can't calculate network bee count.");
              return false;
            }

            if (block.vtx.size() > 0) {
              for (const auto &tx : block.vtx) {
                CAmount beeFeePaidX;
                if (tx->IsBCT(consensusParams, scriptPubKeyBCF, &beeFeePaidX)) {
                  if (tx->vout.size() > 1 &&
                      tx->vout[1].scriptPubKey == scriptPubKeyCF) {
                    CAmount donationAmountX = tx->vout[1].nValue;
                    CAmount expectedDonationAmountX;
                    if ((chainActive.Tip()->nHeight) >= nContribFork)
                      expectedDonationAmountX =
                          (beeFeePaidX + donationAmountX) /
                          consensusParams.communityContribFactor2;

                    else
                      expectedDonationAmountX =
                          (beeFeePaidX + donationAmountX) /
                          consensusParams.communityContribFactor;

                    if (donationAmountX != expectedDonationAmountX)
                      continue;
                    beeFeePaidX += donationAmountX;
                  }

                  int dyingbeesCreationTime =
                      (chainActive.Backtestnet(pindexPrev))->GetBlockTime();

                  CAmount beeCostX;

                  if (((dyingbeesCreationTime > switchLmem) &&
                       (switchLmem > switchHmem)) ||
                      ((switchLmem > switchHmem) &&
                       (dyingbeesCreationTime < switchHmem)) ||
                      ((switchHmem > switchLmem) &&
                       ((dyingbeesCreationTime > switchLmem) &&
                        (dyingbeesCreationTime <= switchHmem))) ||
                      (!(switchHmem))) {
                    beeCostX =
                        0.0004 *
                        (GetBlockSubsidy(
                            (chainActive.Backtestnet(pindexPrev))->nHeight,
                            consensusParams));

                  } else {
                    beeCostX =
                        0.0008 *
                        (GetBlockSubsidy(
                            (chainActive.Backtestnet(pindexPrev))->nHeight,
                            consensusParams));
                  }

                  int beeCountZ = beeFeePaidX / beeCostX;

                  beesDying += beeCountZ;

                  matureBees -= beeCountZ;

                  matureBCTs--;
                }
              }
            }
          }
        }

        if (i >= consensusParams.ratioForkBlock + totalBeeLifespan2) {
          if (fHavePruned &&
              !((chainActive.ReBacktestnet(pindexPrev))->nStatus &
                BLOCK_HAVE_DATA) &&
              (chainActive.ReBacktestnet(pindexPrev))->nTx > 0) {
            LogPrintf("! GetNetworkHiveInfo: Warn: Block not available (pruned "
                      "data); can't calculate network bee count.");
            return false;
          }

          if ((!(chainActive.ReBacktestnet(pindexPrev))
                    ->GetBlockHeader()
                    .IsHiveMined(consensusParams))) {
            if (!ReadBlockFromDisk(block,
                                   (chainActive.ReBacktestnet(pindexPrev)),
                                   consensusParams)) {
              LogPrintf("! GetNetworkHiveInfo: Warn: Block not available (not "
                        "found on disk); can't calculate network bee count.");
              return false;
            }

            if (block.vtx.size() > 0) {
              for (const auto &tx : block.vtx) {
                CAmount beeFeePaidX;
                if (tx->IsBCT(consensusParams, scriptPubKeyBCF, &beeFeePaidX)) {
                  if (tx->vout.size() > 1 &&
                      tx->vout[1].scriptPubKey == scriptPubKeyCF) {
                    CAmount donationAmountX = tx->vout[1].nValue;
                    CAmount expectedDonationAmountX;
                    if ((chainActive.Tip()->nHeight) >= nContribFork)
                      expectedDonationAmountX =
                          (beeFeePaidX + donationAmountX) /
                          consensusParams.communityContribFactor2;

                    else
                      expectedDonationAmountX =
                          (beeFeePaidX + donationAmountX) /
                          consensusParams.communityContribFactor;

                    if (donationAmountX != expectedDonationAmountX)
                      continue;
                    beeFeePaidX += donationAmountX;
                  }

                  int dyingbeesCreationTime =
                      (chainActive.ReBacktestnet(pindexPrev))->GetBlockTime();

                  CAmount beeCostX;

                  if (((dyingbeesCreationTime > switchLmem) &&
                       (switchLmem > switchHmem)) ||
                      ((switchLmem > switchHmem) &&
                       (dyingbeesCreationTime < switchHmem)) ||
                      ((switchHmem > switchLmem) &&
                       ((dyingbeesCreationTime > switchLmem) &&
                        (dyingbeesCreationTime <= switchHmem))) ||
                      (!(switchHmem))) {
                    beeCostX =
                        0.0004 *
                        (GetBlockSubsidy(
                            (chainActive.ReBacktestnet(pindexPrev))->nHeight,
                            consensusParams));

                  } else {
                    beeCostX =
                        0.0008 *
                        (GetBlockSubsidy(
                            (chainActive.ReBacktestnet(pindexPrev))->nHeight,
                            consensusParams));
                  }

                  int beeCountZ = beeFeePaidX / beeCostX;

                  beesDying += beeCountZ;

                  matureBees -= beeCountZ;

                  matureBCTs--;
                }
              }
            }
          }
        }
      }

      int basebeeCost =
          0.0004 * (GetBlockSubsidy(pindexPrev->nHeight, consensusParams));
      threshold = ((potentialLifespanRewards2 / basebeeCost) * 0.9);

      totalMatureBees = matureBees;

      thematurebees = matureBees;
      wototo = matureBees;

      if ((matureBees > threshold) && (priceState == 0)) {
        priceState = 1;
        int switchHigher = pindexPrev->GetBlockTime();
        switchHmem = switchHigher;
      }

      if ((matureBees <= threshold) && (priceState == 1)) {
        priceState = 0;
        int switchLower = pindexPrev->GetBlockTime();
        switchLmem = switchLower;
      }

      if (!chainActive.Next(pindexPrev))

        return true;

      pindexPrev = chainActive.Next(pindexPrev);
    }

    firstRun = 1;

    rpriceState = priceState;
    rimmatureBees = immatureBees;
    rimmatureBCTs = immatureBCTs;
    rmatureBees = matureBees;
    rmatureBCTs = matureBCTs;
  }

  if (firstRun == 1) {
    priceState = rpriceState;
    immatureBees = rimmatureBees;
    immatureBCTs = rimmatureBCTs;
    matureBees = rmatureBees;
    matureBCTs = rmatureBCTs;

    pindexPrev = pindexRem;

    assert(pindexPrev != nullptr);

    int o = (tipHeight - 1);

    for (int i = remTipHeight; i < tipHeight; i++) {
      if (fHavePruned && !(pindexPrev->nStatus & BLOCK_HAVE_DATA) &&
          pindexPrev->nTx > 0) {
        LogPrintf("! GetNetworkHiveInfo: Warn: Block not available (pruned "
                  "data); can't calculate network bee count.");
        return false;
      }

      if (!pindexPrev->GetBlockHeader().IsHiveMined(consensusParams)) {
        if (!ReadBlockFromDisk(block, pindexPrev, consensusParams)) {
          LogPrintf("! GetNetworkHiveInfo: Warn: Block not available (not "
                    "found on disk); can't calculate network bee count.");
          return false;
        }
        int blockHeight = pindexPrev->nHeight;
        CAmount beeCost;

        if (priceState == 0)
          beeCost =
              0.0004 * (GetBlockSubsidy(pindexPrev->nHeight, consensusParams));
        else
          beeCost =
              0.0008 * (GetBlockSubsidy(pindexPrev->nHeight, consensusParams));

        if (block.vtx.size() > 0) {
          for (const auto &tx : block.vtx) {
            CAmount beeFeePaid;
            if (tx->IsBCT(consensusParams, scriptPubKeyBCF, &beeFeePaid)) {
              if (tx->vout.size() > 1 &&
                  tx->vout[1].scriptPubKey == scriptPubKeyCF) {
                CAmount donationAmount = tx->vout[1].nValue;
                CAmount expectedDonationAmount;
                if ((chainActive.Tip()->nHeight) >= nContribFork)
                  expectedDonationAmount =
                      (beeFeePaid + donationAmount) /
                      consensusParams.communityContribFactor2;

                else
                  expectedDonationAmount =
                      (beeFeePaid + donationAmount) /
                      consensusParams.communityContribFactor;

                if (donationAmount != expectedDonationAmount)
                  continue;
                beeFeePaid += donationAmount;
              }
              int beeCount = beeFeePaid / beeCost;

              immatureBees += beeCount;
              immatureBCTs++;

              if (recalcGraph) {
                if (i > bon) {
                  if (i < consensusParams.ratioForkBlock) {
                    int beeBornBlock = blockHeight;
                    int beeMaturesBlock =
                        beeBornBlock + consensusParams.beeGestationBlocks;
                    int beeDiesBlock =
                        beeMaturesBlock + consensusParams.beeLifespanBlocks;
                    for (int j = beeBornBlock; j < beeDiesBlock; j++) {
                      int graphPos = j - tipHeight;
                      if (graphPos > 0 && graphPos < totalBeeLifespan) {
                        if (j < beeMaturesBlock) {
                          beePopGraph[graphPos].immaturePop += beeCount;

                        } else {
                          beePopGraph[graphPos].maturePop += beeCount;
                        }
                      }
                    }

                  } else {
                    int beeBornBlock = blockHeight;
                    int beeMaturesBlock =
                        beeBornBlock + consensusParams.beeGestationBlocks;
                    int beeDiesBlock =
                        beeMaturesBlock + consensusParams.beeLifespanBlocks2;
                    for (int j = beeBornBlock; j < beeDiesBlock; j++) {
                      int graphPos = j - tipHeight;
                      if (graphPos > 0 && graphPos < totalBeeLifespan2) {
                        if (j < beeMaturesBlock) {
                          beePopGraph[graphPos].immaturePop += beeCount;
                        } else {
                          beePopGraph[graphPos].maturePop += beeCount;
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }

      if (consensusParams.isTestnet == true) {
        if (fHavePruned &&
            !(chainActive.Back24testnet(pindexPrev)->nStatus &
              BLOCK_HAVE_DATA) &&
            chainActive.Back24testnet(pindexPrev)->nTx > 0) {
          LogPrintf("! GetNetworkHiveInfo: Warn: Block not available (pruned "
                    "data); can't calculate network bee count.");
          return false;
        }

        if (!(chainActive.Back24testnet(pindexPrev)
                  ->GetBlockHeader()
                  .IsHiveMined(consensusParams))) {
          if (!ReadBlockFromDisk(block, chainActive.Back24testnet(pindexPrev),
                                 consensusParams)) {
            LogPrintf("! GetNetworkHiveInfo: Warn: Block not available (not "
                      "found on disk); can't calculate network bee count.");
            return false;
          }

          CAmount beeCost;

          if (block.vtx.size() > 0) {
            for (const auto &tx : block.vtx) {
              CAmount beeFeePaid;
              if (tx->IsBCT(consensusParams, scriptPubKeyBCF, &beeFeePaid)) {
                if (tx->vout.size() > 1 &&
                    tx->vout[1].scriptPubKey == scriptPubKeyCF) {
                  CAmount donationAmount = tx->vout[1].nValue;
                  CAmount expectedDonationAmount;
                  if ((chainActive.Tip()->nHeight) >= nContribFork)
                    expectedDonationAmount =
                        (beeFeePaid + donationAmount) /
                        consensusParams.communityContribFactor2;

                  else
                    expectedDonationAmount =
                        (beeFeePaid + donationAmount) /
                        consensusParams.communityContribFactor;

                  if (donationAmount != expectedDonationAmount)
                    continue;
                  beeFeePaid += donationAmount;
                }

                int maturingbeesCreationTimetestnet =
                    (chainActive.Back24testnet(pindexPrev))->GetBlockTime();

                if (((maturingbeesCreationTimetestnet > switchLmem) &&
                     (switchLmem > switchHmem)) ||
                    ((switchLmem > switchHmem) &&
                     (maturingbeesCreationTimetestnet < switchHmem)) ||
                    ((switchHmem > switchLmem) &&
                     ((maturingbeesCreationTimetestnet > switchLmem) &&
                      (maturingbeesCreationTimetestnet <= switchHmem))) ||
                    (!(switchHmem))) {
                  beeCost =
                      0.0004 *
                      (GetBlockSubsidy(
                          (chainActive.Back24testnet(pindexPrev))->nHeight,
                          consensusParams));

                } else {
                  beeCost =
                      0.0008 *
                      (GetBlockSubsidy(
                          (chainActive.Back24testnet(pindexPrev))->nHeight,
                          consensusParams));
                }

                int beeCount = beeFeePaid / beeCost;

                immatureBees -= beeCount;
                immatureBCTs--;

                matureBees += beeCount;

                matureBCTs++;
              }
            }
          }
        }
      }

      if (consensusParams.isTestnet == false) {
        if (fHavePruned &&
            !(chainActive.Back24(pindexPrev)->nStatus & BLOCK_HAVE_DATA) &&
            chainActive.Back24(pindexPrev)->nTx > 0) {
          LogPrintf("! GetNetworkHiveInfo: Warn: Block not available (pruned "
                    "data); can't calculate network bee count.");
          return false;
        }

        if (!(chainActive.Back24(pindexPrev)
                  ->GetBlockHeader()
                  .IsHiveMined(consensusParams))) {
          if (!ReadBlockFromDisk(block, chainActive.Back24(pindexPrev),
                                 consensusParams)) {
            LogPrintf("! GetNetworkHiveInfo: Warn: Block not available (not "
                      "found on disk); can't calculate network bee count.");
            return false;
          }

          CAmount beeCost;

          if (block.vtx.size() > 0) {
            for (const auto &tx : block.vtx) {
              CAmount beeFeePaid;
              if (tx->IsBCT(consensusParams, scriptPubKeyBCF, &beeFeePaid)) {
                if (tx->vout.size() > 1 &&
                    tx->vout[1].scriptPubKey == scriptPubKeyCF) {
                  CAmount donationAmount = tx->vout[1].nValue;
                  CAmount expectedDonationAmount;
                  if ((chainActive.Tip()->nHeight) >= nContribFork)
                    expectedDonationAmount =
                        (beeFeePaid + donationAmount) /
                        consensusParams.communityContribFactor2;

                  else
                    expectedDonationAmount =
                        (beeFeePaid + donationAmount) /
                        consensusParams.communityContribFactor;

                  if (donationAmount != expectedDonationAmount)
                    continue;
                  beeFeePaid += donationAmount;
                }

                int maturingbeesCreationTime =
                    (chainActive.Back24(pindexPrev))->GetBlockTime();

                if (((maturingbeesCreationTime > switchLmem) &&
                     (switchLmem > switchHmem)) ||
                    ((switchLmem > switchHmem) &&
                     (maturingbeesCreationTime < switchHmem)) ||
                    ((switchHmem > switchLmem) &&
                     ((maturingbeesCreationTime > switchLmem) &&
                      (maturingbeesCreationTime <= switchHmem))) ||
                    (!(switchHmem))) {
                  beeCost =
                      0.0004 * (GetBlockSubsidy(
                                   (chainActive.Back24(pindexPrev))->nHeight,
                                   consensusParams));

                } else {
                  beeCost =
                      0.0008 * (GetBlockSubsidy(
                                   (chainActive.Back24(pindexPrev))->nHeight,
                                   consensusParams));
                }

                int beeCount = beeFeePaid / beeCost;

                immatureBees -= beeCount;
                immatureBCTs--;

                matureBees += beeCount;

                matureBCTs++;
              }
            }
          }
        }
      }

      if (consensusParams.isTestnet == false) {
        if (i < consensusParams.ratioForkBlock + totalBeeLifespan) {
          if (fHavePruned &&
              !((chainActive.Back(pindexPrev))->nStatus & BLOCK_HAVE_DATA) &&
              (chainActive.Back(pindexPrev))->nTx > 0) {
            LogPrintf("! GetNetworkHiveInfo: Warn: Block not available (pruned "
                      "data); can't calculate network bee count.");
            return false;
          }

          if ((!(chainActive.Back(pindexPrev))
                    ->GetBlockHeader()
                    .IsHiveMined(consensusParams))) {
            if (!ReadBlockFromDisk(block, (chainActive.Back(pindexPrev)),
                                   consensusParams)) {
              LogPrintf("! GetNetworkHiveInfo: Warn: Block not available (not "
                        "found on disk); can't calculate network bee count.");
              return false;
            }

            if (block.vtx.size() > 0) {
              for (const auto &tx : block.vtx) {
                CAmount beeFeePaidX;
                if (tx->IsBCT(consensusParams, scriptPubKeyBCF, &beeFeePaidX)) {
                  if (tx->vout.size() > 1 &&
                      tx->vout[1].scriptPubKey == scriptPubKeyCF) {
                    CAmount donationAmountX = tx->vout[1].nValue;
                    CAmount expectedDonationAmountX;
                    if ((chainActive.Tip()->nHeight) >= nContribFork)
                      expectedDonationAmountX =
                          (beeFeePaidX + donationAmountX) /
                          consensusParams.communityContribFactor2;

                    else
                      expectedDonationAmountX =
                          (beeFeePaidX + donationAmountX) /
                          consensusParams.communityContribFactor;

                    if (donationAmountX != expectedDonationAmountX)
                      continue;
                    beeFeePaidX += donationAmountX;
                  }

                  int dyingbeesCreationTime =
                      (chainActive.Back(pindexPrev))->GetBlockTime();

                  CAmount beeCostX;

                  if (((dyingbeesCreationTime > switchLmem) &&
                       (switchLmem > switchHmem)) ||
                      ((switchLmem > switchHmem) &&
                       (dyingbeesCreationTime < switchHmem)) ||
                      ((switchHmem > switchLmem) &&
                       ((dyingbeesCreationTime > switchLmem) &&
                        (dyingbeesCreationTime <= switchHmem))) ||
                      (!(switchHmem))) {
                    beeCostX =
                        0.0004 * (GetBlockSubsidy(
                                     (chainActive.Back(pindexPrev))->nHeight,
                                     consensusParams));

                  } else {
                    beeCostX =
                        0.0008 * (GetBlockSubsidy(
                                     (chainActive.Back(pindexPrev))->nHeight,
                                     consensusParams));
                  }

                  int beeCountZ = beeFeePaidX / beeCostX;
                  beesDying += beeCountZ;

                  matureBees -= beeCountZ;

                  matureBCTs--;
                }
              }
            }
          }
        }

        if (i >= (consensusParams.ratioForkBlock + totalBeeLifespan2)) {
          if (fHavePruned &&
              !((chainActive.ReBack(pindexPrev))->nStatus & BLOCK_HAVE_DATA) &&
              (chainActive.ReBack(pindexPrev))->nTx > 0) {
            LogPrintf("! GetNetworkHiveInfo: Warn: Block not available (pruned "
                      "data); can't calculate network bee count.");
            return false;
          }

          if ((!(chainActive.ReBack(pindexPrev))
                    ->GetBlockHeader()
                    .IsHiveMined(consensusParams))) {
            if (!ReadBlockFromDisk(block, (chainActive.ReBack(pindexPrev)),
                                   consensusParams)) {
              LogPrintf("! GetNetworkHiveInfo: Warn: Block not available (not "
                        "found on disk); can't calculate network bee count.");
              return false;
            }

            if (block.vtx.size() > 0) {
              for (const auto &tx : block.vtx) {
                CAmount beeFeePaidX;
                if (tx->IsBCT(consensusParams, scriptPubKeyBCF, &beeFeePaidX)) {
                  if (tx->vout.size() > 1 &&
                      tx->vout[1].scriptPubKey == scriptPubKeyCF) {
                    CAmount donationAmountX = tx->vout[1].nValue;
                    CAmount expectedDonationAmountX;
                    if ((chainActive.Tip()->nHeight) >= nContribFork)
                      expectedDonationAmountX =
                          (beeFeePaidX + donationAmountX) /
                          consensusParams.communityContribFactor2;

                    else
                      expectedDonationAmountX =
                          (beeFeePaidX + donationAmountX) /
                          consensusParams.communityContribFactor;

                    if (donationAmountX != expectedDonationAmountX)
                      continue;
                    beeFeePaidX += donationAmountX;
                  }

                  int dyingbeesCreationTime =
                      (chainActive.ReBack(pindexPrev))->GetBlockTime();

                  CAmount beeCostX;

                  if (((dyingbeesCreationTime > switchLmem) &&
                       (switchLmem > switchHmem)) ||
                      ((switchLmem > switchHmem) &&
                       (dyingbeesCreationTime < switchHmem)) ||
                      ((switchHmem > switchLmem) &&
                       ((dyingbeesCreationTime > switchLmem) &&
                        (dyingbeesCreationTime <= switchHmem))) ||
                      (!(switchHmem))) {
                    beeCostX =
                        0.0004 * (GetBlockSubsidy(
                                     (chainActive.ReBack(pindexPrev))->nHeight,
                                     consensusParams));

                  } else {
                    beeCostX =
                        0.0008 * (GetBlockSubsidy(
                                     (chainActive.ReBack(pindexPrev))->nHeight,
                                     consensusParams));
                  }

                  int beeCountZ = beeFeePaidX / beeCostX;
                  beesDying += beeCountZ;

                  matureBees -= beeCountZ;

                  matureBCTs--;
                }
              }
            }
          }
        }
      }

      if (consensusParams.isTestnet == true) {
        if (i < consensusParams.ratioForkBlock + totalBeeLifespan) {
          if (fHavePruned &&
              !((chainActive.Backtestnet(pindexPrev))->nStatus &
                BLOCK_HAVE_DATA) &&
              (chainActive.Backtestnet(pindexPrev))->nTx > 0) {
            LogPrintf("! GetNetworkHiveInfo: Warn: Block not available (pruned "
                      "data); can't calculate network bee count.");
            return false;
          }

          if ((!(chainActive.Backtestnet(pindexPrev))
                    ->GetBlockHeader()
                    .IsHiveMined(consensusParams))) {
            if (!ReadBlockFromDisk(block, (chainActive.Backtestnet(pindexPrev)),
                                   consensusParams)) {
              LogPrintf("! GetNetworkHiveInfo: Warn: Block not available (not "
                        "found on disk); can't calculate network bee count.");
              return false;
            }

            if (block.vtx.size() > 0) {
              for (const auto &tx : block.vtx) {
                CAmount beeFeePaidX;
                if (tx->IsBCT(consensusParams, scriptPubKeyBCF, &beeFeePaidX)) {
                  if (tx->vout.size() > 1 &&
                      tx->vout[1].scriptPubKey == scriptPubKeyCF) {
                    CAmount donationAmountX = tx->vout[1].nValue;
                    CAmount expectedDonationAmountX;
                    if ((chainActive.Tip()->nHeight) >= nContribFork)
                      expectedDonationAmountX =
                          (beeFeePaidX + donationAmountX) /
                          consensusParams.communityContribFactor2;

                    else
                      expectedDonationAmountX =
                          (beeFeePaidX + donationAmountX) /
                          consensusParams.communityContribFactor;

                    if (donationAmountX != expectedDonationAmountX)
                      continue;
                    beeFeePaidX += donationAmountX;
                  }

                  int dyingbeesCreationTime =
                      (chainActive.Backtestnet(pindexPrev))->GetBlockTime();

                  CAmount beeCostX;

                  if (((dyingbeesCreationTime > switchLmem) &&
                       (switchLmem > switchHmem)) ||
                      ((switchLmem > switchHmem) &&
                       (dyingbeesCreationTime < switchHmem)) ||
                      ((switchHmem > switchLmem) &&
                       ((dyingbeesCreationTime > switchLmem) &&
                        (dyingbeesCreationTime <= switchHmem))) ||
                      (!(switchHmem))) {
                    beeCostX =
                        0.0004 *
                        (GetBlockSubsidy(
                            (chainActive.Backtestnet(pindexPrev))->nHeight,
                            consensusParams));

                  } else {
                    beeCostX =
                        0.0008 *
                        (GetBlockSubsidy(
                            (chainActive.Backtestnet(pindexPrev))->nHeight,
                            consensusParams));
                  }

                  int beeCountZ = beeFeePaidX / beeCostX;

                  beesDying += beeCountZ;

                  matureBees -= beeCountZ;

                  matureBCTs--;
                }
              }
            }
          }
        }

        if (i >= consensusParams.ratioForkBlock + totalBeeLifespan2) {
          if (fHavePruned &&
              !((chainActive.ReBacktestnet(pindexPrev))->nStatus &
                BLOCK_HAVE_DATA) &&
              (chainActive.ReBacktestnet(pindexPrev))->nTx > 0) {
            LogPrintf("! GetNetworkHiveInfo: Warn: Block not available (pruned "
                      "data); can't calculate network bee count.");
            return false;
          }

          if ((!(chainActive.ReBacktestnet(pindexPrev))
                    ->GetBlockHeader()
                    .IsHiveMined(consensusParams))) {
            if (!ReadBlockFromDisk(block,
                                   (chainActive.ReBacktestnet(pindexPrev)),
                                   consensusParams)) {
              LogPrintf("! GetNetworkHiveInfo: Warn: Block not available (not "
                        "found on disk); can't calculate network bee count.");
              return false;
            }

            if (block.vtx.size() > 0) {
              for (const auto &tx : block.vtx) {
                CAmount beeFeePaidX;
                if (tx->IsBCT(consensusParams, scriptPubKeyBCF, &beeFeePaidX)) {
                  if (tx->vout.size() > 1 &&
                      tx->vout[1].scriptPubKey == scriptPubKeyCF) {
                    CAmount donationAmountX = tx->vout[1].nValue;
                    CAmount expectedDonationAmountX;
                    if ((chainActive.Tip()->nHeight) >= nContribFork)
                      expectedDonationAmountX =
                          (beeFeePaidX + donationAmountX) /
                          consensusParams.communityContribFactor2;

                    else
                      expectedDonationAmountX =
                          (beeFeePaidX + donationAmountX) /
                          consensusParams.communityContribFactor;

                    if (donationAmountX != expectedDonationAmountX)
                      continue;
                    beeFeePaidX += donationAmountX;
                  }

                  int dyingbeesCreationTime =
                      (chainActive.ReBacktestnet(pindexPrev))->GetBlockTime();

                  CAmount beeCostX;

                  if (((dyingbeesCreationTime > switchLmem) &&
                       (switchLmem > switchHmem)) ||
                      ((switchLmem > switchHmem) &&
                       (dyingbeesCreationTime < switchHmem)) ||
                      ((switchHmem > switchLmem) &&
                       ((dyingbeesCreationTime > switchLmem) &&
                        (dyingbeesCreationTime <= switchHmem))) ||
                      (!(switchHmem))) {
                    beeCostX =
                        0.0004 *
                        (GetBlockSubsidy(
                            (chainActive.ReBacktestnet(pindexPrev))->nHeight,
                            consensusParams));

                  } else {
                    beeCostX =
                        0.0008 *
                        (GetBlockSubsidy(
                            (chainActive.ReBacktestnet(pindexPrev))->nHeight,
                            consensusParams));
                  }

                  int beeCountZ = beeFeePaidX / beeCostX;

                  beesDying += beeCountZ;

                  matureBees -= beeCountZ;

                  matureBCTs--;
                }
              }
            }
          }
        }
      }

      int basebeeCost =
          0.0004 * (GetBlockSubsidy(pindexPrev->nHeight, consensusParams));
      threshold = ((potentialLifespanRewards2 / basebeeCost) * 0.9);

      totalMatureBees = matureBees;

      thematurebees = matureBees;
      wototo = matureBees;

      if ((matureBees > threshold) && (priceState == 0)) {
        priceState = 1;
        int switchHigher = pindexPrev->GetBlockTime();
        switchHmem = switchHigher;
      }

      if ((matureBees <= threshold) && (priceState == 1)) {
        priceState = 0;
        int switchLower = pindexPrev->GetBlockTime();
        switchLmem = switchLower;
      }

      if (!chainActive.Next(pindexPrev))

        return true;

      pindexPrev = chainActive.Next(pindexPrev);
    }

    bon = o;
  }

  return true;
}

bool GetNetworkHiveInfo4(int &immatureBees, int &immatureBCTs, int &matureBees,
                         int &matureBCTs, CAmount &potentialLifespanRewards,
                         const Consensus::Params &consensusParams,
                         bool recalcGraph) {
  int totalBeeLifespan;

  if ((chainActive.Tip()->nHeight) >= nAdjustFork)
    totalBeeLifespan =
        consensusParams.beeLifespanBlocks + consensusParams.beeGestationBlocks;

  else if (((chainActive.Tip()->nHeight) >= nSpeedFork) &&
           ((chainActive.Tip()->nHeight) < nAdjustFork))
    totalBeeLifespan =
        consensusParams.beeLifespanBlocks3 + consensusParams.beeGestationBlocks;

  else
    totalBeeLifespan =
        consensusParams.beeLifespanBlocks + consensusParams.beeGestationBlocks;

  immatureBees = immatureBCTs = matureBees = matureBCTs = 0;

  CBlockIndex *pindexPrev = chainActive.Tip();
  assert(pindexPrev != nullptr);
  int tipHeight = pindexPrev->nHeight;

  if ((chainActive.Tip()->nHeight) >= nAdjustFork) {
    potentialLifespanRewards =
        (consensusParams.beeLifespanBlocks *
         (GetBlockSubsidy(pindexPrev->nHeight, consensusParams))) /
        consensusParams.hiveBlockSpacingTarget;

    LogPrintf("potentialLifespanRewards = %i\n", potentialLifespanRewards);
  }

  else if (((chainActive.Tip()->nHeight) >= nSpeedFork) &&
           ((chainActive.Tip()->nHeight) < nAdjustFork))
    potentialLifespanRewards =
        (consensusParams.beeLifespanBlocks3 *
         GetBeeCost(chainActive.Height(), consensusParams)) /
        consensusParams.hiveBlockSpacingTarget;
  else
    potentialLifespanRewards =
        (consensusParams.beeLifespanBlocks *
         GetBeeCost(chainActive.Height(), consensusParams)) /
        consensusParams.hiveBlockSpacingTarget;

  if (recalcGraph) {
    for (int i = 0; i < totalBeeLifespan; i++) {
      beePopGraph[i].immaturePop = 0;
      beePopGraph[i].maturePop = 0;
    }
  }

  if (IsInitialBlockDownload())

    return false;

  CBlock block;
  CScript scriptPubKeyBCF = GetScriptForDestination(
      DecodeDestination(consensusParams.beeCreationAddress));
  CScript scriptPubKeyCF = GetScriptForDestination(
      DecodeDestination(consensusParams.hiveCommunityAddress));

  for (int i = 0; i < totalBeeLifespan; i++) {
    if (fHavePruned && !(pindexPrev->nStatus & BLOCK_HAVE_DATA) &&
        pindexPrev->nTx > 0) {
      LogPrintf("! GetNetworkHiveInfo: Warn: Block not available (pruned "
                "data); can't calculate network bee count.");
      return false;
    }

    if (!pindexPrev->GetBlockHeader().IsHiveMined(consensusParams)) {
      if (!ReadBlockFromDisk(block, pindexPrev, consensusParams)) {
        LogPrintf("! GetNetworkHiveInfo: Warn: Block not available (not found "
                  "on disk); can't calculate network bee count.");
        return false;
      }
      int blockHeight = pindexPrev->nHeight;

      CAmount beeCost =
          0.0004 * (GetBlockSubsidy(pindexPrev->nHeight, consensusParams));

      if (block.vtx.size() > 0) {
        for (const auto &tx : block.vtx) {
          CAmount beeFeePaid;
          if (tx->IsBCT(consensusParams, scriptPubKeyBCF, &beeFeePaid)) {
            if (tx->vout.size() > 1 &&
                tx->vout[1].scriptPubKey == scriptPubKeyCF) {
              CAmount donationAmount = tx->vout[1].nValue;
              CAmount expectedDonationAmount;
              if ((chainActive.Tip()->nHeight) >= nContribFork)
                expectedDonationAmount =
                    (beeFeePaid + donationAmount) /
                    consensusParams.communityContribFactor2;

              else
                expectedDonationAmount = (beeFeePaid + donationAmount) /
                                         consensusParams.communityContribFactor;

              if (donationAmount != expectedDonationAmount)
                continue;
              beeFeePaid += donationAmount;
            }
            int beeCount = beeFeePaid / beeCost;
            if (i < consensusParams.beeGestationBlocks) {
              immatureBees += beeCount;
              immatureBCTs++;
            } else {
              matureBees += beeCount;
              matureBCTs++;
            }

            if (recalcGraph) {
              int beeBornBlock = blockHeight;
              int beeMaturesBlock =
                  beeBornBlock + consensusParams.beeGestationBlocks;
              int beeDiesBlock;
              if ((chainActive.Tip()->nHeight) >= nAdjustFork)
                beeDiesBlock =
                    beeMaturesBlock + consensusParams.beeLifespanBlocks;
              else if (((chainActive.Tip()->nHeight) >= nSpeedFork) &&
                       ((chainActive.Tip()->nHeight) < nAdjustFork))
                beeDiesBlock =
                    beeMaturesBlock + consensusParams.beeLifespanBlocks3;
              else
                beeDiesBlock =
                    beeMaturesBlock + consensusParams.beeLifespanBlocks;
              for (int j = beeBornBlock; j < beeDiesBlock; j++) {
                int graphPos = j - tipHeight;
                if (graphPos > 0 && graphPos < totalBeeLifespan) {
                  if (j < beeMaturesBlock)
                    beePopGraph[graphPos].immaturePop += beeCount;
                  else
                    beePopGraph[graphPos].maturePop += beeCount;
                }
              }
            }
          }
        }
      }
    }

    if (!pindexPrev->pprev)

      return true;

    pindexPrev = pindexPrev->pprev;
  }

  return true;
}

bool CheckHiveProof(const CBlock *pblock,
                    const Consensus::Params &consensusParams) {
  bool verbose = LogAcceptCategory(BCLog::HIVE);

  if (verbose)
    LogPrintf(
        "********************* Hive: CheckHiveProof *********************\n");

  int blockHeight;
  CBlockIndex *pindexPrev;
  {
    LOCK(cs_main);
    pindexPrev = mapBlockIndex[pblock->hashPrevBlock];
    blockHeight = pindexPrev->nHeight + 1;
  }
  if (!pindexPrev) {
    LogPrintf("CheckHiveProof: Couldn't get previous block's CBlockIndex!\n");
    return false;
  }
  if (verbose)
    LogPrintf("CheckHiveProof: nHeight             = %i\n", blockHeight);

  if (!IsHiveEnabled(pindexPrev, consensusParams)) {
    LogPrintf("CheckHiveProof: Can't accept a Hive block; Hive is not yet "
              "enabled on the network.\n");
    return false;
  }

  if (pindexPrev->GetBlockHeader().IsHiveMined(consensusParams)) {
    LogPrintf("CheckHiveProof: Hive block must follow a POW block.\n");
    return false;
  }

  CScript scriptPubKeyBCF = GetScriptForDestination(
      DecodeDestination(consensusParams.beeCreationAddress));
  if (pblock->vtx.size() > 1)
    for (unsigned int i = 1; i < pblock->vtx.size(); i++)
      if (pblock->vtx[i]->IsBCT(consensusParams, scriptPubKeyBCF)) {
        LogPrintf("CheckHiveProof: Hivemined block contains BCTs!\n");
        return false;
      }

  CTransactionRef txCoinbase = pblock->vtx[0];

  if (!txCoinbase->IsCoinBase()) {
    LogPrintf("CheckHiveProof: Coinbase tx isn't valid!\n");
    return false;
  }

  if (txCoinbase->vout.size() < 2 || txCoinbase->vout.size() > 3) {
    LogPrintf("CheckHiveProof: Didn't expect %i vouts!\n",
              txCoinbase->vout.size());
    return false;
  }

  if (txCoinbase->vout[0].scriptPubKey.size() < 144) {
    LogPrintf("CheckHiveProof: vout[0].scriptPubKey isn't long enough to "
              "contain hive proof encodings\n");
    return false;
  }

  if (txCoinbase->vout[0].scriptPubKey[0] != OP_RETURN ||
      txCoinbase->vout[0].scriptPubKey[1] != OP_BEE) {
    LogPrintf("CheckHiveProof: vout[0].scriptPubKey doesn't start OP_RETURN "
              "OP_BEE\n");
    return false;
  }

  uint32_t beeNonce = ReadLE32(&txCoinbase->vout[0].scriptPubKey[3]);
  if (verbose)
    LogPrintf("CheckHiveProof: beeNonce            = %i\n", beeNonce);

  uint32_t bctClaimedHeight = ReadLE32(&txCoinbase->vout[0].scriptPubKey[8]);
  if (verbose)
    LogPrintf("CheckHiveProof: bctHeight           = %i\n", bctClaimedHeight);

  bool communityContrib = txCoinbase->vout[0].scriptPubKey[12] == OP_TRUE;
  if (verbose)
    LogPrintf("CheckHiveProof: communityContrib    = %s\n",
              communityContrib ? "true" : "false");

  std::vector<unsigned char> txid(&txCoinbase->vout[0].scriptPubKey[14],
                                  &txCoinbase->vout[0].scriptPubKey[14 + 64]);
  std::string txidStr = std::string(txid.begin(), txid.end());
  if (verbose)
    LogPrintf("CheckHiveProof: bctTxId             = %s\n", txidStr);

  std::string deterministicRandString = GetDeterministicRandString(pindexPrev);
  if (verbose)
    LogPrintf("CheckHiveProof: detRandString       = %s\n",
              deterministicRandString);
  arith_uint256 beeHashTarget;
  beeHashTarget.SetCompact(
      GetNextHiveWorkRequired(pindexPrev, consensusParams));
  if (verbose)
    LogPrintf("CheckHiveProof: beeHashTarget       = %s\n",
              beeHashTarget.ToString());
  std::string hashHex = (CHashWriter(SER_GETHASH, 0)
                         << deterministicRandString << txidStr << beeNonce)
                            .GetHash()
                            .GetHex();
  arith_uint256 beeHash = arith_uint256(hashHex);
  if (verbose)
    LogPrintf("CheckHiveProof: beeHash             = %s\n", hashHex);
  if (beeHash >= beeHashTarget) {
    LogPrintf("CheckHiveProof: Bee does not meet hash target!\n");
    return false;
  }

  std::vector<unsigned char> messageSig(
      &txCoinbase->vout[0].scriptPubKey[79],
      &txCoinbase->vout[0].scriptPubKey[79 + 65]);
  if (verbose)
    LogPrintf("CheckHiveProof: messageSig          = %s\n",
              HexStr(&messageSig[0], &messageSig[messageSig.size()]));

  CTxDestination honeyDestination;
  if (!ExtractDestination(txCoinbase->vout[1].scriptPubKey, honeyDestination)) {
    LogPrintf("CheckHiveProof: Couldn't extract honey address\n");
    return false;
  }
  if (!IsValidDestination(honeyDestination)) {
    LogPrintf("CheckHiveProof: Honey address is invalid\n");
    return false;
  }
  if (verbose)
    LogPrintf("CheckHiveProof: honeyAddress        = %s\n",
              EncodeDestination(honeyDestination));

  const CKeyID *keyID = boost::get<CKeyID>(&honeyDestination);
  if (!keyID) {
    LogPrintf("CheckHiveProof: Can't get pubkey for honey address\n");
    return false;
  }
  CHashWriter ss(SER_GETHASH, 0);
  ss << deterministicRandString;
  uint256 mhash = ss.GetHash();
  CPubKey pubkey;
  if (!pubkey.RecoverCompact(mhash, messageSig)) {
    LogPrintf("CheckHiveProof: Couldn't recover pubkey from hash\n");
    return false;
  }
  if (pubkey.GetID() != *keyID) {
    LogPrintf("CheckHiveProof: Signature mismatch! GetID() = %s, *keyID = %s\n",
              pubkey.GetID().ToString(), (*keyID).ToString());
    return false;
  }

  bool deepDrill = false;
  uint32_t bctFoundHeight;
  CAmount bctValue;
  CScript bctScriptPubKey;
  {
    LOCK(cs_main);

    COutPoint outBeeCreation(uint256S(txidStr), 0);
    COutPoint outCommFund(uint256S(txidStr), 1);
    Coin coin;
    CTransactionRef bct = nullptr;
    CBlockIndex foundAt;

    if (pcoinsTip && pcoinsTip->GetCoin(outBeeCreation, coin)) {
      if (verbose)
        LogPrintf("CheckHiveProof: Using UTXO set for outBeeCreation\n");
      bctValue = coin.out.nValue;
      bctScriptPubKey = coin.out.scriptPubKey;
      bctFoundHeight = coin.nHeight;
    } else {
      if (verbose)
        LogPrintf(
            "! CheckHiveProof: Warn: Using deep drill for outBeeCreation\n");
      if (!GetTxByHashAndHeight(uint256S(txidStr), bctClaimedHeight, bct,
                                foundAt, pindexPrev, consensusParams)) {
        LogPrintf("CheckHiveProof: Couldn't locate indicated BCT\n");
        return false;
      }
      deepDrill = true;
      bctFoundHeight = foundAt.nHeight;
      bctValue = bct->vout[0].nValue;
      bctScriptPubKey = bct->vout[0].scriptPubKey;
    }

    if (communityContrib) {
      CScript scriptPubKeyCF;

      if (bctFoundHeight >= nLightFork)
        scriptPubKeyCF = GetScriptForDestination(
            DecodeDestination(consensusParams.hiveCommunityAddress2));
      else
        scriptPubKeyCF = GetScriptForDestination(
            DecodeDestination(consensusParams.hiveCommunityAddress));

      CAmount donationAmount;

      if (bct == nullptr) {
        if (pcoinsTip && pcoinsTip->GetCoin(outCommFund, coin)) {
          if (verbose)
            LogPrintf("CheckHiveProof: Using UTXO set for outCommFund\n");
          if (coin.out.scriptPubKey != scriptPubKeyCF) {
            LogPrintf("CheckHiveProof: Community contrib was indicated but not "
                      "found\n");
            return false;
          }
          donationAmount = coin.out.nValue;
        } else {
          if (verbose)
            LogPrintf(
                "! CheckHiveProof: Warn: Using deep drill for outCommFund\n");
          if (!GetTxByHashAndHeight(uint256S(txidStr), bctClaimedHeight, bct,
                                    foundAt, pindexPrev, consensusParams)) {
            LogPrintf("CheckHiveProof: Couldn't locate indicated BCT\n");

            return false;
          }
          deepDrill = true;
        }
      }
      if (bct != nullptr) {
        if (bct->vout.size() < 2 ||
            bct->vout[1].scriptPubKey != scriptPubKeyCF) {
          LogPrintf("CheckHiveProof: Community contrib was indicated but not "
                    "found\n");
          return false;
        }
        donationAmount = bct->vout[1].nValue;
      }

      CAmount expectedDonationAmount;
      if ((chainActive.Tip()->nHeight) >= nContribFork)
        expectedDonationAmount = (bctValue + donationAmount) /
                                 consensusParams.communityContribFactor2;

      else
        expectedDonationAmount = (bctValue + donationAmount) /
                                 consensusParams.communityContribFactor;

      if (donationAmount != expectedDonationAmount) {
        LogPrintf("CheckHiveProof: BCT pays community fund incorrect amount %i "
                  "(expected %i)\n",
                  donationAmount, expectedDonationAmount);
        return false;
      }

      bctValue += donationAmount;
    }
  }

  if (bctFoundHeight != bctClaimedHeight) {
    LogPrintf("CheckHiveProof: Claimed BCT height of %i conflicts with found "
              "height of %i\n",
              bctClaimedHeight, bctFoundHeight);
    return false;
  }

  int bctDepth = blockHeight - bctFoundHeight;
  if (bctDepth < consensusParams.beeGestationBlocks) {
    LogPrintf("CheckHiveProof: Indicated BCT is immature.\n");
    return false;
  }

  if (bctFoundHeight >= nSpeedFork) {
    if (bctDepth > consensusParams.beeGestationBlocks +
                       consensusParams.beeLifespanBlocks3) {
      LogPrintf("CheckHiveProof: Indicated BCT is too old.\n");
      return false;
    }
  }

  if ((bctFoundHeight >= consensusParams.ratioForkBlock) &&
      (bctFoundHeight < nSpeedFork)) {
    if (bctDepth > consensusParams.beeGestationBlocks +
                       consensusParams.beeLifespanBlocks2) {
      LogPrintf("CheckHiveProof: Indicated BCT is too old.\n");
      return false;
    }
  }
  if (bctFoundHeight < consensusParams.ratioForkBlock) {
    if (bctDepth > consensusParams.beeGestationBlocks +
                       consensusParams.beeLifespanBlocks) {
      LogPrintf("CheckHiveProof: Indicated BCT is too old.\n");
      return false;
    }
  }

  CScript scriptPubKeyHoney;
  if (!CScript::IsBCTScript(bctScriptPubKey, scriptPubKeyBCF,
                            &scriptPubKeyHoney)) {
    LogPrintf("CheckHiveProof: Indicated utxo is not a valid BCT script\n");
    return false;
  }

  CTxDestination honeyDestinationBCT;
  if (!ExtractDestination(scriptPubKeyHoney, honeyDestinationBCT)) {
    LogPrintf("CheckHiveProof: Couldn't extract honey address from BCT UTXO\n");
    return false;
  }

  if (honeyDestination != honeyDestinationBCT) {
    LogPrintf("CheckHiveProof: BCT's honey address does not match claimed "
              "honey address!\n");
    return false;
  }

  CAmount beeCost = GetBeeCost(bctFoundHeight, consensusParams);
  if (bctValue < consensusParams.minBeeCost) {
    LogPrintf(
        "CheckHiveProof: BCT fee is less than the minimum possible bee cost\n");
    return false;
  }
  if (bctValue < beeCost) {
    LogPrintf(
        "CheckHiveProof: BCT fee is less than the cost for a single bee\n");
    return false;
  }
  unsigned int beeCount = bctValue / beeCost;
  if (verbose) {
    LogPrintf("CheckHiveProof: bctValue            = %i\n", bctValue);
    LogPrintf("CheckHiveProof: beeCost             = %i\n", beeCost);
    LogPrintf("CheckHiveProof: beeCount            = %i\n", beeCount);
  }

  if (beeNonce >= beeCount) {
    LogPrintf(
        "CheckHiveProof: BCT did not create enough bees for claimed nonce!\n");
    return false;
  }

  LogPrintf("CheckHiveProof: Pass at %i%s\n", blockHeight,
            deepDrill ? " (used deepdrill)" : "");
  return true;
}

bool CheckHiveProof2(const CBlock *pblock,
                     const Consensus::Params &consensusParams) {
  bool verbose = LogAcceptCategory(BCLog::HIVE);

  if (verbose)
    LogPrintf(
        "********************* Hive: CheckHiveProof 2 *********************\n");

  int blockHeight;
  CBlockIndex *pindexPrev;
  {
    LOCK(cs_main);
    pindexPrev = mapBlockIndex[pblock->hashPrevBlock];
    blockHeight = pindexPrev->nHeight + 1;
  }
  if (!pindexPrev) {
    LogPrintf("CheckHiveProof: Couldn't get previous block's CBlockIndex!\n");
    return false;
  }
  if (verbose)
    LogPrintf("CheckHiveProof: nHeight             = %i\n", blockHeight);

  if (!IsHiveEnabled(pindexPrev, consensusParams)) {
    LogPrintf("CheckHiveProof: Can't accept a Hive block; Hive is not yet "
              "enabled on the network.\n");
    return false;
  }

  if (pindexPrev->GetBlockHeader().IsHiveMined(consensusParams)) {
    LogPrintf("CheckHiveProof: Hive block must follow a POW block.\n");
    return false;
  }

  CScript scriptPubKeyBCF = GetScriptForDestination(
      DecodeDestination(consensusParams.beeCreationAddress));
  if (pblock->vtx.size() > 1)
    for (unsigned int i = 1; i < pblock->vtx.size(); i++)
      if (pblock->vtx[i]->IsBCT(consensusParams, scriptPubKeyBCF)) {
        LogPrintf("CheckHiveProof: Hivemined block contains BCTs!\n");
        return false;
      }

  CTransactionRef txCoinbase = pblock->vtx[0];

  if (!txCoinbase->IsCoinBase()) {
    LogPrintf("CheckHiveProof: Coinbase tx isn't valid!\n");
    return false;
  }

  if (txCoinbase->vout.size() < 2 || txCoinbase->vout.size() > 3) {
    LogPrintf("CheckHiveProof: Didn't expect %i vouts!\n",
              txCoinbase->vout.size());
    return false;
  }

  if (txCoinbase->vout[0].scriptPubKey.size() < 144) {
    LogPrintf("CheckHiveProof: vout[0].scriptPubKey isn't long enough to "
              "contain hive proof encodings\n");
    return false;
  }

  if (txCoinbase->vout[0].scriptPubKey[0] != OP_RETURN ||
      txCoinbase->vout[0].scriptPubKey[1] != OP_BEE) {
    LogPrintf("CheckHiveProof: vout[0].scriptPubKey doesn't start OP_RETURN "
              "OP_BEE\n");
    return false;
  }

  uint32_t beeNonce = ReadLE32(&txCoinbase->vout[0].scriptPubKey[3]);
  if (verbose)
    LogPrintf("CheckHiveProof: beeNonce            = %i\n", beeNonce);

  uint32_t bctClaimedHeight = ReadLE32(&txCoinbase->vout[0].scriptPubKey[8]);
  if (verbose)
    LogPrintf("CheckHiveProof: bctHeight           = %i\n", bctClaimedHeight);

  bool communityContrib = txCoinbase->vout[0].scriptPubKey[12] == OP_TRUE;
  if (verbose)
    LogPrintf("CheckHiveProof: communityContrib    = %s\n",
              communityContrib ? "true" : "false");

  std::vector<unsigned char> txid(&txCoinbase->vout[0].scriptPubKey[14],
                                  &txCoinbase->vout[0].scriptPubKey[14 + 64]);
  std::string txidStr = std::string(txid.begin(), txid.end());
  if (verbose)
    LogPrintf("CheckHiveProof: bctTxId             = %s\n", txidStr);

  std::string deterministicRandString = GetDeterministicRandString(pindexPrev);
  if (verbose)
    LogPrintf("CheckHiveProof: detRandString       = %s\n",
              deterministicRandString);
  arith_uint256 beeHashTarget;
  beeHashTarget.SetCompact(
      GetNextHiveWorkRequired(pindexPrev, consensusParams));
  if (verbose)
    LogPrintf("CheckHiveProof: beeHashTarget       = %s\n",
              beeHashTarget.ToString());
  std::string hashHex = (CHashWriter(SER_GETHASH, 0)
                         << deterministicRandString << txidStr << beeNonce)
                            .GetHash()
                            .GetHex();
  arith_uint256 beeHash = arith_uint256(hashHex);
  if (verbose)
    LogPrintf("CheckHiveProof: beeHash             = %s\n", hashHex);
  if (beeHash >= beeHashTarget) {
    LogPrintf("CheckHiveProof: Bee does not meet hash target!\n");
    return false;
  }

  std::vector<unsigned char> messageSig(
      &txCoinbase->vout[0].scriptPubKey[79],
      &txCoinbase->vout[0].scriptPubKey[79 + 65]);
  if (verbose)
    LogPrintf("CheckHiveProof: messageSig          = %s\n",
              HexStr(&messageSig[0], &messageSig[messageSig.size()]));

  CTxDestination honeyDestination;
  if (!ExtractDestination(txCoinbase->vout[1].scriptPubKey, honeyDestination)) {
    LogPrintf("CheckHiveProof: Couldn't extract honey address\n");
    return false;
  }
  if (!IsValidDestination(honeyDestination)) {
    LogPrintf("CheckHiveProof: Honey address is invalid\n");
    return false;
  }
  if (verbose)
    LogPrintf("CheckHiveProof: honeyAddress        = %s\n",
              EncodeDestination(honeyDestination));

  const CKeyID *keyID = boost::get<CKeyID>(&honeyDestination);
  if (!keyID) {
    LogPrintf("CheckHiveProof: Can't get pubkey for honey address\n");
    return false;
  }
  CHashWriter ss(SER_GETHASH, 0);
  ss << deterministicRandString;
  uint256 mhash = ss.GetHash();
  CPubKey pubkey;
  if (!pubkey.RecoverCompact(mhash, messageSig)) {
    LogPrintf("CheckHiveProof: Couldn't recover pubkey from hash\n");
    return false;
  }
  if (pubkey.GetID() != *keyID) {
    LogPrintf("CheckHiveProof: Signature mismatch! GetID() = %s, *keyID = %s\n",
              pubkey.GetID().ToString(), (*keyID).ToString());
    return false;
  }

  bool deepDrill = false;
  uint32_t bctFoundHeight;
  CAmount bctValue;
  CScript bctScriptPubKey;
  {
    LOCK(cs_main);

    COutPoint outBeeCreation(uint256S(txidStr), 0);
    COutPoint outCommFund(uint256S(txidStr), 1);
    Coin coin;
    CTransactionRef bct = nullptr;
    CBlockIndex foundAt;

    if (pcoinsTip && pcoinsTip->GetCoin(outBeeCreation, coin)) {
      if (verbose)
        LogPrintf("CheckHiveProof: Using UTXO set for outBeeCreation\n");
      bctValue = coin.out.nValue;
      bctScriptPubKey = coin.out.scriptPubKey;
      bctFoundHeight = coin.nHeight;
    } else {
      if (verbose)
        LogPrintf(
            "! CheckHiveProof: Warn: Using deep drill for outBeeCreation\n");
      if (!GetTxByHashAndHeight(uint256S(txidStr), bctClaimedHeight, bct,
                                foundAt, pindexPrev, consensusParams)) {
        LogPrintf("CheckHiveProof: Couldn't locate indicated BCT\n");
        return false;
      }
      deepDrill = true;
      bctFoundHeight = foundAt.nHeight;
      bctValue = bct->vout[0].nValue;
      bctScriptPubKey = bct->vout[0].scriptPubKey;
    }

    if (communityContrib) {
      CScript scriptPubKeyCF;

      if (bctFoundHeight >= nLightFork)
        scriptPubKeyCF = GetScriptForDestination(
            DecodeDestination(consensusParams.hiveCommunityAddress2));
      else
        scriptPubKeyCF = GetScriptForDestination(
            DecodeDestination(consensusParams.hiveCommunityAddress));

      CAmount donationAmount;

      if (bct == nullptr) {
        if (pcoinsTip && pcoinsTip->GetCoin(outCommFund, coin)) {
          if (verbose)
            LogPrintf("CheckHiveProof: Using UTXO set for outCommFund\n");
          if (coin.out.scriptPubKey != scriptPubKeyCF) {
            LogPrintf("CheckHiveProof: Community contrib was indicated but not "
                      "found\n");
            return false;
          }
          donationAmount = coin.out.nValue;
        } else {
          if (verbose)
            LogPrintf(
                "! CheckHiveProof: Warn: Using deep drill for outCommFund\n");
          if (!GetTxByHashAndHeight(uint256S(txidStr), bctClaimedHeight, bct,
                                    foundAt, pindexPrev, consensusParams)) {
            LogPrintf("CheckHiveProof: Couldn't locate indicated BCT\n");

            return false;
          }
          deepDrill = true;
        }
      }
      if (bct != nullptr) {
        if (bct->vout.size() < 2 ||
            bct->vout[1].scriptPubKey != scriptPubKeyCF) {
          LogPrintf("CheckHiveProof: Community contrib was indicated but not "
                    "found\n");
          return false;
        }
        donationAmount = bct->vout[1].nValue;
      }

      CAmount expectedDonationAmount;
      if ((chainActive.Tip()->nHeight) >= nContribFork)
        expectedDonationAmount = (bctValue + donationAmount) /
                                 consensusParams.communityContribFactor2;

      else
        expectedDonationAmount = (bctValue + donationAmount) /
                                 consensusParams.communityContribFactor;

      if (donationAmount != expectedDonationAmount) {
        LogPrintf("CheckHiveProof: BCT pays community fund incorrect amount %i "
                  "(expected %i)\n",
                  donationAmount, expectedDonationAmount);
        return false;
      }

      bctValue += donationAmount;
    }
  }

  if (bctFoundHeight != bctClaimedHeight) {
    LogPrintf("CheckHiveProof: Claimed BCT height of %i conflicts with found "
              "height of %i\n",
              bctClaimedHeight, bctFoundHeight);
    return false;
  }

  int bctDepth = blockHeight - bctFoundHeight;
  if (bctDepth < consensusParams.beeGestationBlocks) {
    LogPrintf("CheckHiveProof: Indicated BCT is immature.\n");
    return false;
  }

  if (bctFoundHeight >= consensusParams.ratioForkBlock) {
    if (bctDepth > consensusParams.beeGestationBlocks +
                       consensusParams.beeLifespanBlocks2) {
      LogPrintf("CheckHiveProof: Indicated BCT is too old.\n");
      return false;
    }

  }

  else {
    if (bctDepth > consensusParams.beeGestationBlocks +
                       consensusParams.beeLifespanBlocks) {
      LogPrintf("CheckHiveProof: Indicated BCT is too old.\n");
      return false;
    }
  }

  CScript scriptPubKeyHoney;
  if (!CScript::IsBCTScript(bctScriptPubKey, scriptPubKeyBCF,
                            &scriptPubKeyHoney)) {
    LogPrintf("CheckHiveProof: Indicated utxo is not a valid BCT script\n");
    return false;
  }

  CTxDestination honeyDestinationBCT;
  if (!ExtractDestination(scriptPubKeyHoney, honeyDestinationBCT)) {
    LogPrintf("CheckHiveProof: Couldn't extract honey address from BCT UTXO\n");
    return false;
  }

  if (honeyDestination != honeyDestinationBCT) {
    LogPrintf("CheckHiveProof: BCT's honey address does not match claimed "
              "honey address!\n");
    return false;
  }

  CAmount beeCost;
  CBlockIndex foundAt;
  int foundTime = foundAt.GetBlockTime();

  if ((priceState == 0) || ((priceState == 1) && (foundTime <= switchHmem))) {
    beeCost = 0.0004 * (GetBlockSubsidy(pindexPrev->nHeight, consensusParams));

  } else {
    beeCost = 0.0008 * (GetBlockSubsidy(pindexPrev->nHeight, consensusParams));
  }

  if (bctValue < consensusParams.minBeeCost) {
    LogPrintf(
        "CheckHiveProof: BCT fee is less than the minimum possible bee cost\n");
    return false;
  }
  if (bctValue < beeCost) {
    LogPrintf(
        "CheckHiveProof: BCT fee is less than the cost for a single bee\n");
    return false;
  }
  unsigned int beeCount = bctValue / beeCost;
  if (verbose) {
    LogPrintf("CheckHiveProof: bctValue            = %i\n", bctValue);
    LogPrintf("CheckHiveProof: beeCost             = %i\n", beeCost);
    LogPrintf("CheckHiveProof: beeCount            = %i\n", beeCount);
  }

  if (beeNonce >= beeCount) {
    LogPrintf(
        "CheckHiveProof: BCT did not create enough bees for claimed nonce!\n");
    return false;
  }

  LogPrintf("CheckHiveProof 2 : Pass at %i%s\n", blockHeight,
            deepDrill ? " (used deepdrill)" : "");
  return true;
}

bool CheckHiveProof3(const CBlock *pblock,
                     const Consensus::Params &consensusParams) {
  bool verbose = LogAcceptCategory(BCLog::HIVE);

  if (verbose)
    LogPrintf(
        "********************* Hive: CheckHiveProof 3 *********************\n");

  int blockHeight;
  CBlockIndex *pindexPrev;
  {
    LOCK(cs_main);
    pindexPrev = mapBlockIndex[pblock->hashPrevBlock];
    blockHeight = pindexPrev->nHeight + 1;
  }
  if (!pindexPrev) {
    LogPrintf("CheckHiveProof: Couldn't get previous block's CBlockIndex!\n");
    return false;
  }
  if (verbose)
    LogPrintf("CheckHiveProof: nHeight             = %i\n", blockHeight);

  if (!IsHiveEnabled(pindexPrev, consensusParams)) {
    LogPrintf("CheckHiveProof: Can't accept a Hive block; Hive is not yet "
              "enabled on the network.\n");
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
      LogPrintf("CheckHiveProof: Too many Hive blocks without a POW block.\n");
      return false;
    }
  } else {
    if (pindexPrev->GetBlockHeader().IsHiveMined(consensusParams)) {
      LogPrint(BCLog::HIVE,
               "CheckHiveProof: Hive block must follow a POW block.\n");
      return false;
    }
  }

  CScript scriptPubKeyBCF = GetScriptForDestination(
      DecodeDestination(consensusParams.beeCreationAddress));
  if (pblock->vtx.size() > 1)
    for (unsigned int i = 1; i < pblock->vtx.size(); i++)
      if (pblock->vtx[i]->IsBCT(consensusParams, scriptPubKeyBCF)) {
        LogPrintf("CheckHiveProof: Hivemined block contains BCTs!\n");
        return false;
      }

  CTransactionRef txCoinbase = pblock->vtx[0];

  if (!txCoinbase->IsCoinBase()) {
    LogPrintf("CheckHiveProof: Coinbase tx isn't valid!\n");
    return false;
  }

  if (txCoinbase->vout.size() < 2 || txCoinbase->vout.size() > 3) {
    LogPrintf("CheckHiveProof: Didn't expect %i vouts!\n",
              txCoinbase->vout.size());
    return false;
  }

  if (txCoinbase->vout[0].scriptPubKey.size() < 144) {
    LogPrintf("CheckHiveProof: vout[0].scriptPubKey isn't long enough to "
              "contain hive proof encodings\n");
    return false;
  }

  if (txCoinbase->vout[0].scriptPubKey[0] != OP_RETURN ||
      txCoinbase->vout[0].scriptPubKey[1] != OP_BEE) {
    LogPrintf("CheckHiveProof: vout[0].scriptPubKey doesn't start OP_RETURN "
              "OP_BEE\n");
    return false;
  }

  uint32_t beeNonce = ReadLE32(&txCoinbase->vout[0].scriptPubKey[3]);
  if (verbose)
    LogPrintf("CheckHiveProof: beeNonce            = %i\n", beeNonce);

  uint32_t bctClaimedHeight = ReadLE32(&txCoinbase->vout[0].scriptPubKey[8]);
  if (verbose)
    LogPrintf("CheckHiveProof: bctHeight           = %i\n", bctClaimedHeight);

  bool communityContrib = txCoinbase->vout[0].scriptPubKey[12] == OP_TRUE;
  if (verbose)
    LogPrintf("CheckHiveProof: communityContrib    = %s\n",
              communityContrib ? "true" : "false");

  std::vector<unsigned char> txid(&txCoinbase->vout[0].scriptPubKey[14],
                                  &txCoinbase->vout[0].scriptPubKey[14 + 64]);
  std::string txidStr = std::string(txid.begin(), txid.end());
  if (verbose)
    LogPrintf("CheckHiveProof: bctTxId             = %s\n", txidStr);

  std::string deterministicRandString = GetDeterministicRandString(pindexPrev);
  if (verbose)
    LogPrintf("CheckHiveProof: detRandString       = %s\n",
              deterministicRandString);
  arith_uint256 beeHashTarget;
  beeHashTarget.SetCompact(
      GetNextHiveWorkRequired(pindexPrev, consensusParams));
  if (verbose)
    LogPrintf("CheckHiveProof: beeHashTarget       = %s\n",
              beeHashTarget.ToString());
  std::string hashHex = (CHashWriter(SER_GETHASH, 0)
                         << deterministicRandString << txidStr << beeNonce)
                            .GetHash()
                            .GetHex();
  arith_uint256 beeHash = arith_uint256(hashHex);
  if (verbose)
    LogPrintf("CheckHiveProof: beeHash             = %s\n", hashHex);
  if (beeHash >= beeHashTarget) {
    LogPrintf("CheckHiveProof: Bee does not meet hash target!\n");
    return false;
  }

  std::vector<unsigned char> messageSig(
      &txCoinbase->vout[0].scriptPubKey[79],
      &txCoinbase->vout[0].scriptPubKey[79 + 65]);
  if (verbose)
    LogPrintf("CheckHiveProof: messageSig          = %s\n",
              HexStr(&messageSig[0], &messageSig[messageSig.size()]));

  CTxDestination honeyDestination;
  if (!ExtractDestination(txCoinbase->vout[1].scriptPubKey, honeyDestination)) {
    LogPrintf("CheckHiveProof: Couldn't extract honey address\n");
    return false;
  }
  if (!IsValidDestination(honeyDestination)) {
    LogPrintf("CheckHiveProof: Honey address is invalid\n");
    return false;
  }
  if (verbose)
    LogPrintf("CheckHiveProof: honeyAddress        = %s\n",
              EncodeDestination(honeyDestination));

  const CKeyID *keyID = boost::get<CKeyID>(&honeyDestination);
  if (!keyID) {
    LogPrintf("CheckHiveProof: Can't get pubkey for honey address\n");
    return false;
  }
  CHashWriter ss(SER_GETHASH, 0);
  ss << deterministicRandString;
  uint256 mhash = ss.GetHash();
  CPubKey pubkey;
  if (!pubkey.RecoverCompact(mhash, messageSig)) {
    LogPrintf("CheckHiveProof: Couldn't recover pubkey from hash\n");
    return false;
  }
  if (pubkey.GetID() != *keyID) {
    LogPrintf("CheckHiveProof: Signature mismatch! GetID() = %s, *keyID = %s\n",
              pubkey.GetID().ToString(), (*keyID).ToString());
    return false;
  }

  bool deepDrill = false;
  uint32_t bctFoundHeight;
  CAmount bctValue;
  CScript bctScriptPubKey;
  {
    LOCK(cs_main);

    COutPoint outBeeCreation(uint256S(txidStr), 0);
    COutPoint outCommFund(uint256S(txidStr), 1);
    Coin coin;
    CTransactionRef bct = nullptr;
    CBlockIndex foundAt;

    if (pcoinsTip && pcoinsTip->GetCoin(outBeeCreation, coin)) {
      if (verbose)
        LogPrintf("CheckHiveProof: Using UTXO set for outBeeCreation\n");
      bctValue = coin.out.nValue;
      bctScriptPubKey = coin.out.scriptPubKey;
      bctFoundHeight = coin.nHeight;
    } else {
      if (verbose)
        LogPrintf(
            "! CheckHiveProof: Warn: Using deep drill for outBeeCreation\n");
      if (!GetTxByHashAndHeight(uint256S(txidStr), bctClaimedHeight, bct,
                                foundAt, pindexPrev, consensusParams)) {
        LogPrintf("CheckHiveProof: Couldn't locate indicated BCT\n");
        return false;
      }
      deepDrill = true;
      bctFoundHeight = foundAt.nHeight;
      bctValue = bct->vout[0].nValue;
      bctScriptPubKey = bct->vout[0].scriptPubKey;
    }

    if (communityContrib) {
      CScript scriptPubKeyCF;

      if (bctFoundHeight >= nLightFork)
        scriptPubKeyCF = GetScriptForDestination(
            DecodeDestination(consensusParams.hiveCommunityAddress2));
      else
        scriptPubKeyCF = GetScriptForDestination(
            DecodeDestination(consensusParams.hiveCommunityAddress));

      CAmount donationAmount;

      if (bct == nullptr) {
        if (pcoinsTip && pcoinsTip->GetCoin(outCommFund, coin)) {
          if (verbose)
            LogPrintf("CheckHiveProof: Using UTXO set for outCommFund\n");
          if (coin.out.scriptPubKey != scriptPubKeyCF) {
            LogPrintf("CheckHiveProof: Community contrib was indicated but not "
                      "found\n");
            return false;
          }
          donationAmount = coin.out.nValue;
        } else {
          if (verbose)
            LogPrintf(
                "! CheckHiveProof: Warn: Using deep drill for outCommFund\n");
          if (!GetTxByHashAndHeight(uint256S(txidStr), bctClaimedHeight, bct,
                                    foundAt, pindexPrev, consensusParams)) {
            LogPrintf("CheckHiveProof: Couldn't locate indicated BCT\n");

            return false;
          }
          deepDrill = true;
        }
      }
      if (bct != nullptr) {
        if (bct->vout.size() < 2 ||
            bct->vout[1].scriptPubKey != scriptPubKeyCF) {
          LogPrintf("CheckHiveProof: Community contrib was indicated but not "
                    "found\n");
          return false;
        }
        donationAmount = bct->vout[1].nValue;
      }

      CAmount expectedDonationAmount;
      if ((chainActive.Tip()->nHeight) >= nContribFork)
        expectedDonationAmount = (bctValue + donationAmount) /
                                 consensusParams.communityContribFactor2;

      else
        expectedDonationAmount = (bctValue + donationAmount) /
                                 consensusParams.communityContribFactor;

      if (donationAmount != expectedDonationAmount) {
        LogPrintf("CheckHiveProof: BCT pays community fund incorrect amount %i "
                  "(expected %i)\n",
                  donationAmount, expectedDonationAmount);
        return false;
      }

      bctValue += donationAmount;
    }
  }

  if (bctFoundHeight != bctClaimedHeight) {
    LogPrintf("CheckHiveProof: Claimed BCT height of %i conflicts with found "
              "height of %i\n",
              bctClaimedHeight, bctFoundHeight);
    return false;
  }

  int bctDepth = blockHeight - bctFoundHeight;
  if (bctDepth < consensusParams.beeGestationBlocks) {
    LogPrintf("CheckHiveProof: Indicated BCT is immature.\n");
    return false;
  }

  if (bctFoundHeight >= nAdjustFork) {
    if (bctDepth > consensusParams.beeGestationBlocks +
                       consensusParams.beeLifespanBlocks) {
      LogPrintf("CheckHiveProof: Indicated BCT is too old.\n");
      return false;
    }
  }

  if ((bctFoundHeight >= consensusParams.ratioForkBlock) &&
      (bctFoundHeight < nAdjustFork)) {
    if (bctDepth > consensusParams.beeGestationBlocks +
                       consensusParams.beeLifespanBlocks2) {
      LogPrintf("CheckHiveProof: Indicated BCT is too old.\n");
      return false;
    }

  }

  else {
    if (bctDepth > consensusParams.beeGestationBlocks +
                       consensusParams.beeLifespanBlocks) {
      LogPrintf("CheckHiveProof: Indicated BCT is too old.\n");
      return false;
    }
  }

  CScript scriptPubKeyHoney;
  if (!CScript::IsBCTScript(bctScriptPubKey, scriptPubKeyBCF,
                            &scriptPubKeyHoney)) {
    LogPrintf("CheckHiveProof: Indicated utxo is not a valid BCT script\n");
    return false;
  }

  CTxDestination honeyDestinationBCT;
  if (!ExtractDestination(scriptPubKeyHoney, honeyDestinationBCT)) {
    LogPrintf("CheckHiveProof: Couldn't extract honey address from BCT UTXO\n");
    return false;
  }

  if (honeyDestination != honeyDestinationBCT) {
    LogPrintf("CheckHiveProof: BCT's honey address does not match claimed "
              "honey address!\n");
    return false;
  }

  CAmount beeCost;
  CBlockIndex foundAt;
  int foundTime = foundAt.GetBlockTime();

  beeCost = 0.0004 * (GetBlockSubsidy(pindexPrev->nHeight, consensusParams));

  if (bctValue < consensusParams.minBeeCost) {
    LogPrintf(
        "CheckHiveProof: BCT fee is less than the minimum possible bee cost\n");
    return false;
  }
  if (bctValue < beeCost) {
    LogPrintf(
        "CheckHiveProof: BCT fee is less than the cost for a single bee\n");
    return false;
  }
  unsigned int beeCount = bctValue / beeCost;
  if (verbose) {
    LogPrintf("CheckHiveProof: bctValue            = %i\n", bctValue);
    LogPrintf("CheckHiveProof: beeCost             = %i\n", beeCost);
    LogPrintf("CheckHiveProof: beeCount            = %i\n", beeCount);
  }

  if (beeNonce >= beeCount) {
    LogPrintf(
        "CheckHiveProof: BCT did not create enough bees for claimed nonce!\n");
    return false;
  }

  LogPrintf("CheckHiveProof 3 : Pass at %i%s\n", blockHeight,
            deepDrill ? " (used deepdrill)" : "");
  return true;
}
