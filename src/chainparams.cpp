// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2025 The Bitcoin Core developers
// Copyright (c) 2019-2025 Antoine Brûlé
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <arith_uint256.h>
#include <chainparams.h>
#include <consensus/merkle.h>

#include <base58.h>
#include <tinyformat.h>
#include <util.h>
#include <utilstrencodings.h>

#include <assert.h>

#include <chainparamsseeds.h>

static CBlock CreateGenesisBlock(const char *pszTimestamp,
                                 const CScript &genesisOutputScript,
                                 uint32_t nTime, uint32_t nNonce,
                                 uint32_t nBits, int32_t nVersion,
                                 const CAmount &genesisReward) {
  CMutableTransaction txNew;
  txNew.nVersion = 1;
  txNew.vin.resize(1);
  txNew.vout.resize(1);
  txNew.vin[0].scriptSig = CScript()
                           << 486604799 << CScriptNum(4)
                           << std::vector<unsigned char>(
                                  (const unsigned char *)pszTimestamp,
                                  (const unsigned char *)pszTimestamp +
                                      strlen(pszTimestamp));
  txNew.vout[0].nValue = genesisReward;
  txNew.vout[0].scriptPubKey = genesisOutputScript;

  CBlock genesis;
  genesis.nTime = nTime;
  genesis.nBits = nBits;
  genesis.nNonce = nNonce;
  genesis.nVersion = nVersion;
  genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
  genesis.hashPrevBlock.SetNull();
  genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
  return genesis;
}

static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce,
                                 uint32_t nBits, int32_t nVersion,
                                 const CAmount &genesisReward) {
  const char *pszTimestamp = "LNC Reborn";
  const CScript genesisOutputScript =
      CScript() << ParseHex("04d603f5d711578999d92907b6d8c020d5900a025be2dd9b79"
                            "c0a026def0e2f62101f14319b0f4f783204fd79dc4976a2596"
                            "885d256c6a7a8cc8c38d67a5e49249")
                << OP_CHECKSIG;
  return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce,
                            nBits, nVersion, genesisReward);
}

void CChainParams::UpdateVersionBitsParameters(Consensus::DeploymentPos d,
                                               int64_t nStartTime,
                                               int64_t nTimeout) {
  consensus.vDeployments[d].nStartTime = nStartTime;
  consensus.vDeployments[d].nTimeout = nTimeout;
}

class CMainParams : public CChainParams {
public:
  CMainParams() {
    strNetworkID = "main";
    consensus.nSubsidyHalvingInterval = 8400000;
    consensus.BIP16Height = 0;

    consensus.BIP34Height = 71000000;

    consensus.BIP34Hash = uint256S(
        "fa09d204a83a768ed5a7c8d441fa62f2043abf420cff1226c7b4329aeb9d51cf");
    consensus.BIP65Height = 91868400;

    consensus.BIP66Height = 81187900;

    consensus.powLimit = uint256S(
        "00000ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
    consensus.powLimit2 = uint256S(
        "0007fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
    consensus.nPowTargetTimespan = 4440;
    consensus.nPowTargetSpacing = 60;

    consensus.nPowTargetSpacing2 = 20;

    consensus.fPowAllowMinDifficultyBlocks = false;
    consensus.fPowNoRetargeting = false;
    consensus.nRuleChangeActivationThreshold = 222;

    consensus.nMinerConfirmationWindow = 296;

    consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
    consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime =
        1199145601;

    consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout =
        1230767999;

    consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
    consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime =
        1681749846 + 5259486;

    consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout =
        1681749846 + 31536000 + 5259486;

    consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
    consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime =
        Consensus::BIP9Deployment::ALWAYS_ACTIVE;

    consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout =
        Consensus::BIP9Deployment::NO_TIMEOUT;

    consensus.vDeployments[Consensus::DEPLOYMENT_HIVE].bit = 7;
    consensus.vDeployments[Consensus::DEPLOYMENT_HIVE].nStartTime =
        Consensus::BIP9Deployment::ALWAYS_ACTIVE;

    consensus.vDeployments[Consensus::DEPLOYMENT_HIVE].nTimeout =
        Consensus::BIP9Deployment::NO_TIMEOUT;

    consensus.vDeployments[Consensus::DEPLOYMENT_HIVE_1_1].bit = 9;
    consensus.vDeployments[Consensus::DEPLOYMENT_HIVE_1_1].nStartTime =
        1695443728;

    consensus.vDeployments[Consensus::DEPLOYMENT_HIVE_1_1].nTimeout =
        1695443728 + 31536000 + 5259486;

    consensus.powForkTime = 1694548330;

    consensus.lastScryptBlock = 0;

    consensus.powLimitSHA = uint256S(
        "00000ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");

    consensus.slowStartBlocks = 0;

    consensus.totalMoneySupplyHeight = 6215968;

    consensus.minBeeCost = 10000;

    consensus.beeCostFactor = 2500;

    consensus.beeCreationAddress = "CReateLitecoinCashWorkerBeeXYs19YQ";

    consensus.hiveCommunityAddress = "CeckYLfkWnViDxKE1R5vLZaarygLrNgUwa";

    consensus.hiveCommunityAddress2 = "CH5B3qVbJVxVCL5qwmXayEo97Z1Ux3pqSB";
    consensus.communityContribFactor = 10;

    consensus.communityContribFactor2 = 2;

    consensus.beeGestationBlocks = 48 * 24;

    consensus.beeLifespanBlocks = 48 * 24 * 14;

    consensus.powLimitHive = uint256S(
        "0ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");

    consensus.powLimitHive2 = uint256S(
        "7ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");

    consensus.minHiveCheckBlock = 1;

    consensus.hiveTargetAdjustAggression = 30;

    consensus.hiveBlockSpacingTarget = 2;

    consensus.hiveNonceMarker = 192;

    consensus.minK = 1;

    consensus.maxK = 8;

    consensus.maxK2 = 7;

    consensus.maxHiveDiff = 0.001;

    consensus.maxHiveDiff2 = 0.006;

    consensus.maxKPow = 5;

    consensus.powSplit1 = 0.000833;

    consensus.powSplit2 = 0.000416;

    consensus.powSplit12 = 0.005;

    consensus.powSplit22 = 0.0025;

    consensus.maxConsecutiveHiveBlocks = 2;

    consensus.hiveDifficultyWindow = 24;

    consensus.variableBeecost = true;

    consensus.variableForkBlock = 5000000;

    consensus.isTestnet = false;
    consensus.ratioForkBlock = 5000000;
    consensus.beeLifespanBlocks2 = 48 * 24 * 21;
    consensus.beeLifespanBlocks3 = 48 * 24 * 21;
    consensus.remvariableForkBlock = 5000000;

    consensus.nMinimumChainWork = uint256S(
        "0x00000000000000000000000000000000000000000000000000008db26219b009");

    consensus.defaultAssumeValid = uint256S(
        "0x090200eb68d00cd1ad990c2a7ddca7d567a2402fb909e99d304d5cf333ab5bdf");

    pchMessageStart[0] = 0xea;
    pchMessageStart[1] = 0xaf;
    pchMessageStart[2] = 0x47;
    pchMessageStart[3] = 0x48;
    nDefaultPort = 9113;
    nPruneAfterHeight = 100000;

    genesis = CreateGenesisBlock(1694548330, 1500284949, 0x1e0ffff0, 1,
                                 1.25 * COIN * COIN_SCALE);

    consensus.hashGenesisBlock = genesis.GetHash();
    assert(consensus.hashGenesisBlock ==
           uint256S("0x35f373b6fed4342ef20327917f756dc5fe76b4f2c111ac0a1c52ba40"
                    "8764709b"));
    assert(genesis.hashMerkleRoot ==
           uint256S("0x4caf91337a748b3d946f7f145bc180da5c9281d6323113c0535d77b8"
                    "08526e5a"));

    base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 28);
    base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 5);
    base58Prefixes[SCRIPT_ADDRESS2] = std::vector<unsigned char>(1, 50);
    base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 176);
    base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x88, 0xB2, 0x1E};
    base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x88, 0xAD, 0xE4};

    bech32_hrp = "lncr";

    vFixedSeeds.clear();
    vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main,
                                         pnSeed6_main + ARRAYLEN(pnSeed6_main));

    fDefaultConsistencyChecks = false;
    fRequireStandard = true;
    fMineBlocksOnDemand = false;

    checkpointData = {{
        {0, uint256S("0x35f373b6fed4342ef20327917f756dc5fe76b4f2c111ac0a1c52ba4"
                     "08764709b")},
        {1305391, uint256S("0x99bd061f8c1b4a5b18ab76c93b353007d9026822df35c7201"
                           "b08dde8cc42e5d3")},
        {3657000, uint256S("0x966f34fec23d771d6a0793396650e64fb1f991c06fbfa046c"
                           "5928d50884f409b")},
        {5362791, uint256S("0x9e881790e1bb01f8f43fc8f3c49f1ca2f257ddf9f7042800a"
                           "aa512e475727528")},
        {5472830, uint256S("0x090200eb68d00cd1ad990c2a7ddca7d567a2402fb909e99d3"
                           "04d5cf333ab5bdf")},
    }};

    chainTxData = ChainTxData{1751354596,

                              5626136,

                              0.09991706136773344

    };
  }
};

class CTestNetParams : public CChainParams {
public:
  CTestNetParams() {
    strNetworkID = "test";
    consensus.nSubsidyHalvingInterval = 1050000;
    consensus.BIP16Height = 0;

    consensus.BIP34Height = 14600000;
    consensus.BIP34Hash = uint256S(
        "000000042bcd56d6ea0509230b76fe850f0a40a9110f7dba979fd5d707e47c8a");

    consensus.BIP65Height = 14600000;
    consensus.BIP66Height = 14600000;
    consensus.powLimit = uint256S(
        "00000ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
    consensus.powLimit2 = uint256S(
        "0007fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
    consensus.nPowTargetTimespan = 3840;

    consensus.nPowTargetSpacing = 60;
    consensus.nPowTargetSpacing2 = 25;
    consensus.fPowAllowMinDifficultyBlocks = false;
    consensus.fPowNoRetargeting = false;
    consensus.nRuleChangeActivationThreshold = 192;

    consensus.nMinerConfirmationWindow = 256;
    consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
    consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime =
        1683303184;

    consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout =
        1683303184 + 31536000;

    consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
    consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1683303184;

    consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout =
        1683303184 + 31536000;

    consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
    consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime =
        Consensus::BIP9Deployment::ALWAYS_ACTIVE;

    consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout =
        Consensus::BIP9Deployment::NO_TIMEOUT;

    consensus.vDeployments[Consensus::DEPLOYMENT_HIVE].bit = 7;
    consensus.vDeployments[Consensus::DEPLOYMENT_HIVE].nStartTime =
        Consensus::BIP9Deployment::ALWAYS_ACTIVE;

    consensus.vDeployments[Consensus::DEPLOYMENT_HIVE].nTimeout =
        Consensus::BIP9Deployment::NO_TIMEOUT;

    consensus.vDeployments[Consensus::DEPLOYMENT_HIVE_1_1].bit = 9;
    consensus.vDeployments[Consensus::DEPLOYMENT_HIVE_1_1].nStartTime =
        1683303184;

    consensus.vDeployments[Consensus::DEPLOYMENT_HIVE_1_1].nTimeout =
        1683303184 + 31536000;

    consensus.powForkTime = 1694548330;

    consensus.lastScryptBlock = 0;

    consensus.powLimitSHA = uint256S(
        "00000ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");

    consensus.slowStartBlocks = 125;

    consensus.totalMoneySupplyHeight = 6215968;

    consensus.minBeeCost = 10000;

    consensus.beeCostFactor = 2500;

    consensus.beeCreationAddress = "tEstNetCreateLCCWorkerBeeXXXYq6T3r";

    consensus.hiveCommunityAddress = "t9ctP2rDfvnqUr9kmo2nb1LEDpu1Lc5sQn";

    consensus.communityContribFactor = 10;

    consensus.beeGestationBlocks = 48 * 24;

    consensus.beeLifespanBlocks = 48 * 24 * 14;

    consensus.powLimitHive = uint256S(
        "0ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");

    consensus.powLimitHive2 = uint256S(
        "7ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");

    consensus.minHiveCheckBlock = 1;

    consensus.hiveTargetAdjustAggression = 30;

    consensus.hiveBlockSpacingTarget = 2;

    consensus.hiveNonceMarker = 192;

    consensus.minK = 1;

    consensus.maxK = 8;

    consensus.maxHiveDiff = 0.002;

    consensus.maxKPow = 5;

    consensus.powSplit1 = 0.001;

    consensus.powSplit2 = 0.0005;

    consensus.maxConsecutiveHiveBlocks = 2;

    consensus.hiveDifficultyWindow = 24;

    consensus.variableBeecost = true;
    consensus.variableForkBlock = 5000000;

    consensus.isTestnet = true;
    consensus.ratioForkBlock = 5000000;

    consensus.beeLifespanBlocks2 = 48 * 24 * 21;
    consensus.beeLifespanBlocks3 = 48 * 24 * 21;
    consensus.remvariableForkBlock = 5000000;

    consensus.nMinimumChainWork = uint256S(
        "0x0000000000000000000000000000000000000000000000000000000000000000");

    consensus.defaultAssumeValid = uint256S(
        "0x35f373b6fed4342ef20327917f756dc5fe76b4f2c111ac0a1c52ba408764709b");

    pchMessageStart[0] = 0x3a;
    pchMessageStart[1] = 0x8b;
    pchMessageStart[2] = 0x5c;
    pchMessageStart[3] = 0x2d;
    nDefaultPort = 59113;
    nPruneAfterHeight = 1000;

    genesis = CreateGenesisBlock(1694548330, 1500284949, 0x1e0ffff0, 1,
                                 1.25 * COIN * COIN_SCALE);

    consensus.hashGenesisBlock = genesis.GetHash();
    assert(consensus.hashGenesisBlock ==
           uint256S("0x35f373b6fed4342ef20327917f756dc5fe76b4f2c111ac0a1c52ba40"
                    "8764709b"));
    assert(genesis.hashMerkleRoot ==
           uint256S("0x4caf91337a748b3d946f7f145bc180da5c9281d6323113c0535d77b8"
                    "08526e5a"));

    vFixedSeeds.clear();

    base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 127);
    base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 196);
    base58Prefixes[SCRIPT_ADDRESS2] = std::vector<unsigned char>(1, 58);
    base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 239);
    base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
    base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

    bech32_hrp = "tlncr";

    vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test,
                                         pnSeed6_test + ARRAYLEN(pnSeed6_test));

    fDefaultConsistencyChecks = false;
    fRequireStandard = true;
    fMineBlocksOnDemand = false;

    checkpointData = {{
        {0, uint256S("0x35f373b6fed4342ef20327917f756dc5fe76b4f2c111ac0a1c52ba4"
                     "08764709b")},
    }};

    chainTxData = ChainTxData{1694548330, 0, 0.0};
  }
};

class CRegTestParams : public CChainParams {
public:
  CRegTestParams() {
    strNetworkID = "regtest";
    consensus.nSubsidyHalvingInterval = 150;
    consensus.BIP16Height = 0;

    consensus.BIP34Height = 100000000;

    consensus.BIP34Hash = uint256();
    consensus.BIP65Height = 1351;

    consensus.BIP66Height = 1251;

    consensus.powLimit = uint256S(
        "7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
    consensus.nPowTargetTimespan = 3.5 * 24 * 60 * 60;

    consensus.nPowTargetSpacing = 2.5 * 60;
    consensus.fPowAllowMinDifficultyBlocks = true;
    consensus.fPowNoRetargeting = true;
    consensus.nRuleChangeActivationThreshold = 108;

    consensus.nMinerConfirmationWindow = 144;

    consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
    consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
    consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout =
        Consensus::BIP9Deployment::NO_TIMEOUT;
    consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
    consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 0;
    consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout =
        Consensus::BIP9Deployment::NO_TIMEOUT;
    consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
    consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime =
        Consensus::BIP9Deployment::ALWAYS_ACTIVE;
    consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout =
        Consensus::BIP9Deployment::NO_TIMEOUT;

    consensus.powForkTime = 1551819029;

    consensus.lastScryptBlock = 0;

    consensus.powLimitSHA = uint256S(
        "000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");

    consensus.slowStartBlocks = 2000;

    consensus.totalMoneySupplyHeight = 6215968;

    consensus.hiveNonceMarker = 192;

    consensus.remvariableForkBlock = 118956;

    consensus.nMinimumChainWork = uint256S("0x00");

    consensus.defaultAssumeValid = uint256S("0x00");

    pchMessageStart[0] = 0xd9;
    pchMessageStart[1] = 0xf6;
    pchMessageStart[2] = 0xcc;
    pchMessageStart[3] = 0xea;
    nDefaultPort = 59444;
    nPruneAfterHeight = 1000;

    genesis = CreateGenesisBlock(1551819029, 1, 0x207fffff, 1,
                                 50 * COIN * COIN_SCALE);
    consensus.hashGenesisBlock = genesis.GetHash();
    assert(consensus.hashGenesisBlock ==
           uint256S("0x238a0bf5a26d0bb55bf257a6b180a49f1422c2270857bf53cbc49f83"
                    "16eb88fd"));
    assert(genesis.hashMerkleRoot ==
           uint256S("0xfe90e5f71db801cab3064947169305a13c8107e645f9387e211fd73f"
                    "266a581a"));

    vFixedSeeds.clear();

    vSeeds.clear();

    fDefaultConsistencyChecks = true;
    fRequireStandard = false;
    fMineBlocksOnDemand = true;

    checkpointData = {{{0, uint256S("238a0bf5a26d0bb55bf257a6b180a49f1422c22708"
                                    "57bf53cbc49f8316eb88fd")}}};

    chainTxData = ChainTxData{0, 0, 0};

    base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 111);
    base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 196);
    base58Prefixes[SCRIPT_ADDRESS2] = std::vector<unsigned char>(1, 58);
    base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 239);
    base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
    base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

    bech32_hrp = "rlncr";
  }
};

static std::unique_ptr<CChainParams> globalChainParams;

const CChainParams &Params() {
  assert(globalChainParams);
  return *globalChainParams;
}

std::unique_ptr<CChainParams> CreateChainParams(const std::string &chain) {
  if (chain == CBaseChainParams::MAIN)
    return std::unique_ptr<CChainParams>(new CMainParams());
  else if (chain == CBaseChainParams::TESTNET)
    return std::unique_ptr<CChainParams>(new CTestNetParams());
  else if (chain == CBaseChainParams::REGTEST)
    return std::unique_ptr<CChainParams>(new CRegTestParams());
  throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string &network) {
  SelectBaseParams(network);
  globalChainParams = CreateChainParams(network);
}

void UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime,
                                 int64_t nTimeout) {
  globalChainParams->UpdateVersionBitsParameters(d, nStartTime, nTimeout);
}
