// Copyright (c) 2025 The LightningCash Reborn Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_MERKLE
#define BITCOIN_MERKLE

#include <stdint.h>
#include <vector>

#include <primitives/block.h>
#include <primitives/transaction.h>
#include <uint256.h>

uint256 ComputeMerkleRoot(const std::vector<uint256> &leaves,
                          bool *mutated = nullptr);
std::vector<uint256> ComputeMerkleBranch(const std::vector<uint256> &leaves,
                                         uint32_t position);
uint256 ComputeMerkleRootFromBranch(const uint256 &leaf,
                                    const std::vector<uint256> &branch,
                                    uint32_t position);
uint256 BlockMerkleRoot(const CBlock &block, bool *mutated = nullptr);
uint256 BlockWitnessMerkleRoot(const CBlock &block, bool *mutated = nullptr);
std::vector<uint256> BlockMerkleBranch(const CBlock &block, uint32_t position);

uint256 ComputeMerkleRootOptimized(const std::vector<uint256> &leaves,
                                   bool *mutated = nullptr);
uint256
ComputeMerkleRootFromTransactions(const std::vector<CTransactionRef> &vtx,
                                  bool *mutated = nullptr);
bool ValidateMerkleProof(const uint256 &leaf,
                         const std::vector<uint256> &branch, uint32_t position,
                         const uint256 &root);
size_t GetMerkleTreeDepth(size_t leaf_count);

#endif