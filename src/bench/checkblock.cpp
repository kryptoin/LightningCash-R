// Copyright (c) 2016-2025 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <bench/bench.h>

#include <chainparams.h>
#include <consensus/validation.h>
#include <streams.h>
#include <validation.h>

namespace block_bench {
#include <bench/data/block413567.raw.h>
}

static void DeserializeBlockTest(benchmark::State &state) {
  CDataStream stream(
      (const char *)block_bench::block413567,
      (const char *)&block_bench::block413567[sizeof(block_bench::block413567)],
      SER_NETWORK, PROTOCOL_VERSION);
  char a = '\0';
  stream.write(&a, 1);

  while (state.KeepRunning()) {
    CBlock block;
    stream >> block;
    assert(stream.Rewind(sizeof(block_bench::block413567)));
  }
}

static void DeserializeAndCheckBlockTest(benchmark::State &state) {
  CDataStream stream(
      (const char *)block_bench::block413567,
      (const char *)&block_bench::block413567[sizeof(block_bench::block413567)],
      SER_NETWORK, PROTOCOL_VERSION);
  char a = '\0';
  stream.write(&a, 1);

  const auto chainParams = CreateChainParams(CBaseChainParams::MAIN);

  while (state.KeepRunning()) {
    CBlock block;

    stream >> block;
    assert(stream.Rewind(sizeof(block_bench::block413567)));

    CValidationState validationState;
    assert(CheckBlock(block, validationState, chainParams->GetConsensus()));
  }
}

BENCHMARK(DeserializeBlockTest, 130);
BENCHMARK(DeserializeAndCheckBlockTest, 160);
