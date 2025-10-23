// Copyright (c) 2025 The LightningCash Reborn Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <algorithm>
#include <consensus/merkle.h>
#include <hash.h>
#include <utilstrencodings.h>

static void MerkleComputationOptimized(const std::vector<uint256> &leaves,
                                       uint256 *proot, bool *pmutated,
                                       uint32_t branchpos,
                                       std::vector<uint256> *pbranch) {
  if (pbranch) {
    pbranch->clear();

    if (leaves.size() > 0) {
      pbranch->reserve(32);
    }
  }

  if (leaves.empty()) {
    if (pmutated)
      *pmutated = false;
    if (proot)
      *proot = uint256();
    return;
  }

  bool mutated = false;
  uint32_t count = 0;
  uint256 inner[32];
  int matchlevel = -1;

  while (count < leaves.size()) {
    uint256 h = leaves[count];
    bool matchh = count == branchpos;
    count++;
    int level;

    for (level = 0; !(count & (1u << level)); level++) {
      if (pbranch) {
        if (matchh) {
          pbranch->push_back(inner[level]);
        } else if (matchlevel == level) {
          pbranch->push_back(h);
          matchh = true;
        }
      }

      mutated |= (inner[level] == h);

      CHash256()
          .Write(inner[level].begin(), 32)
          .Write(h.begin(), 32)
          .Finalize(h.begin());
    }

    inner[level] = h;
    if (matchh) {
      matchlevel = level;
    }
  }

  int level = 0;
  while (!(count & (1u << level))) {
    level++;
  }

  uint256 h = inner[level];
  bool matchh = matchlevel == level;

  while (count != (1u << level)) {
    if (pbranch && matchh) {
      pbranch->push_back(h);
    }

    CHash256().Write(h.begin(), 32).Write(h.begin(), 32).Finalize(h.begin());

    count += (1u << level);
    level++;

    while (!(count & (1u << level))) {
      if (pbranch) {
        if (matchh) {
          pbranch->push_back(inner[level]);
        } else if (matchlevel == level) {
          pbranch->push_back(h);
          matchh = true;
        }
      }
      CHash256()
          .Write(inner[level].begin(), 32)
          .Write(h.begin(), 32)
          .Finalize(h.begin());
      level++;
    }
  }

  if (pmutated)
    *pmutated = mutated;
  if (proot)
    *proot = h;
}

static void MerkleComputation(const std::vector<uint256> &leaves,
                              uint256 *proot, bool *pmutated,
                              uint32_t branchpos,
                              std::vector<uint256> *pbranch) {
  if (pbranch)
    pbranch->clear();
  if (leaves.size() == 0) {
    if (pmutated)
      *pmutated = false;
    if (proot)
      *proot = uint256();
    return;
  }
  bool mutated = false;
  uint32_t count = 0;
  uint256 inner[32];
  int matchlevel = -1;

  while (count < leaves.size()) {
    uint256 h = leaves[count];
    bool matchh = count == branchpos;
    count++;
    int level;

    for (level = 0; !(count & (((uint32_t)1) << level)); level++) {
      if (pbranch) {
        if (matchh) {
          pbranch->push_back(inner[level]);
        } else if (matchlevel == level) {
          pbranch->push_back(h);
          matchh = true;
        }
      }
      mutated |= (inner[level] == h);
      CHash256()
          .Write(inner[level].begin(), 32)
          .Write(h.begin(), 32)
          .Finalize(h.begin());
    }

    inner[level] = h;
    if (matchh) {
      matchlevel = level;
    }
  }

  int level = 0;
  while (!(count & (((uint32_t)1) << level))) {
    level++;
  }
  uint256 h = inner[level];
  bool matchh = matchlevel == level;
  while (count != (((uint32_t)1) << level)) {
    if (pbranch && matchh) {
      pbranch->push_back(h);
    }
    CHash256().Write(h.begin(), 32).Write(h.begin(), 32).Finalize(h.begin());
    count += (((uint32_t)1) << level);
    level++;

    while (!(count & (((uint32_t)1) << level))) {
      if (pbranch) {
        if (matchh) {
          pbranch->push_back(inner[level]);
        } else if (matchlevel == level) {
          pbranch->push_back(h);
          matchh = true;
        }
      }
      CHash256()
          .Write(inner[level].begin(), 32)
          .Write(h.begin(), 32)
          .Finalize(h.begin());
      level++;
    }
  }

  if (pmutated)
    *pmutated = mutated;
  if (proot)
    *proot = h;
}

uint256 ComputeMerkleRoot(const std::vector<uint256> &leaves, bool *mutated) {
  uint256 hash;
  MerkleComputation(leaves, &hash, mutated, -1, nullptr);
  return hash;
}

std::vector<uint256> ComputeMerkleBranch(const std::vector<uint256> &leaves,
                                         uint32_t position) {
  std::vector<uint256> ret;
  MerkleComputation(leaves, nullptr, nullptr, position, &ret);
  return ret;
}

uint256 ComputeMerkleRootFromBranch(const uint256 &leaf,
                                    const std::vector<uint256> &vMerkleBranch,
                                    uint32_t nIndex) {
  uint256 hash = leaf;
  for (const auto &merkleHash : vMerkleBranch) {
    if (nIndex & 1) {
      hash = Hash(BEGIN(merkleHash), END(merkleHash), BEGIN(hash), END(hash));
    } else {
      hash = Hash(BEGIN(hash), END(hash), BEGIN(merkleHash), END(merkleHash));
    }
    nIndex >>= 1;
  }
  return hash;
}

uint256 BlockMerkleRoot(const CBlock &block, bool *mutated) {
  std::vector<uint256> leaves;
  leaves.reserve(block.vtx.size());

  for (const auto &tx : block.vtx) {
    leaves.push_back(tx->GetHash());
  }
  return ComputeMerkleRoot(leaves, mutated);
}

uint256 BlockWitnessMerkleRoot(const CBlock &block, bool *mutated) {
  std::vector<uint256> leaves;
  leaves.reserve(block.vtx.size());

  leaves.resize(block.vtx.size());
  leaves[0].SetNull();

  for (size_t s = 1; s < block.vtx.size(); s++) {
    leaves[s] = block.vtx[s]->GetWitnessHash();
  }
  return ComputeMerkleRoot(leaves, mutated);
}

std::vector<uint256> BlockMerkleBranch(const CBlock &block, uint32_t position) {
  std::vector<uint256> leaves;
  leaves.reserve(block.vtx.size());

  for (const auto &tx : block.vtx) {
    leaves.push_back(tx->GetHash());
  }
  return ComputeMerkleBranch(leaves, position);
}

uint256 ComputeMerkleRootOptimized(const std::vector<uint256> &leaves,
                                   bool *mutated) {
  uint256 hash;
  MerkleComputationOptimized(leaves, &hash, mutated, -1, nullptr);
  return hash;
}

uint256
ComputeMerkleRootFromTransactions(const std::vector<CTransactionRef> &vtx,
                                  bool *mutated) {
  if (vtx.empty()) {
    if (mutated)
      *mutated = false;
    return uint256();
  }

  std::vector<uint256> leaves;
  leaves.reserve(vtx.size());
  for (const auto &tx : vtx) {
    leaves.push_back(tx->GetHash());
  }
  return ComputeMerkleRootOptimized(leaves, mutated);
}

bool ValidateMerkleProof(const uint256 &leaf,
                         const std::vector<uint256> &branch, uint32_t position,
                         const uint256 &root) {
  uint256 computed_root = ComputeMerkleRootFromBranch(leaf, branch, position);
  return computed_root == root;
}

size_t GetMerkleTreeDepth(size_t leaf_count) {
  if (leaf_count <= 1)
    return 0;
  size_t depth = 0;
  size_t n = leaf_count - 1;
  while (n > 0) {
    n >>= 1;
    depth++;
  }
  return depth;
}