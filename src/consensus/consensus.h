// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2025 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_CONSENSUS_CONSENSUS_H
#define BITCOIN_CONSENSUS_CONSENSUS_H

#include <stdint.h>
#include <stdlib.h>

static const unsigned int MAX_BLOCK_SERIALIZED_SIZE = 4000000;

static const unsigned int MAX_BLOCK_WEIGHT = 4000000;

static const int64_t MAX_BLOCK_SIGOPS_COST = 80000;

static const int COINBASE_MATURITY = 125;

static const int WITNESS_SCALE_FACTOR = 4;

static const size_t MIN_TRANSACTION_WEIGHT = WITNESS_SCALE_FACTOR * 60;

static const size_t MIN_SERIALIZABLE_TRANSACTION_WEIGHT =
    WITNESS_SCALE_FACTOR * 10;

static constexpr unsigned int LOCKTIME_VERIFY_SEQUENCE = (1 << 0);

static constexpr unsigned int LOCKTIME_MEDIAN_TIME_PAST = (1 << 1);

#endif
