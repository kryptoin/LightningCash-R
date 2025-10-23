// Copyright (c) 2012-2025 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <coins.h>

#include <consensus/consensus.h>
#include <random.h>

bool CCoinsView::GetCoin(const COutPoint &outpoint, Coin &coin) const {
  return false;
}
uint256 CCoinsView::GetBestBlock() const { return uint256(); }
std::vector<uint256> CCoinsView::GetHeadBlocks() const {
  return std::vector<uint256>();
}
bool CCoinsView::BatchWrite(CCoinsMap &mapCoins, const uint256 &hashBlock) {
  return false;
}
CCoinsViewCursor *CCoinsView::Cursor() const { return nullptr; }

bool CCoinsView::HaveCoin(const COutPoint &outpoint) const {
  Coin coin;
  return GetCoin(outpoint, coin);
}

CCoinsViewBacked::CCoinsViewBacked(CCoinsView *viewIn) : base(viewIn) {}
bool CCoinsViewBacked::GetCoin(const COutPoint &outpoint, Coin &coin) const {
  return base->GetCoin(outpoint, coin);
}
bool CCoinsViewBacked::HaveCoin(const COutPoint &outpoint) const {
  return base->HaveCoin(outpoint);
}
uint256 CCoinsViewBacked::GetBestBlock() const { return base->GetBestBlock(); }
std::vector<uint256> CCoinsViewBacked::GetHeadBlocks() const {
  return base->GetHeadBlocks();
}
void CCoinsViewBacked::SetBackend(CCoinsView &viewIn) { base = &viewIn; }
bool CCoinsViewBacked::BatchWrite(CCoinsMap &mapCoins,
                                  const uint256 &hashBlock) {
  return base->BatchWrite(mapCoins, hashBlock);
}
CCoinsViewCursor *CCoinsViewBacked::Cursor() const { return base->Cursor(); }
size_t CCoinsViewBacked::EstimateSize() const { return base->EstimateSize(); }

SaltedOutpointHasher::SaltedOutpointHasher()
    : k0(GetRand(std::numeric_limits<uint64_t>::max())),
      k1(GetRand(std::numeric_limits<uint64_t>::max())) {}

CCoinsViewCache::CCoinsViewCache(CCoinsView *baseIn)
    : CCoinsViewBacked(baseIn), cachedCoinsUsage(0) {}

size_t CCoinsViewCache::DynamicMemoryUsage() const {
  return memusage::DynamicUsage(cacheCoins) + cachedCoinsUsage;
}

CCoinsMap::iterator
CCoinsViewCache::FetchCoin(const COutPoint &outpoint) const {
  CCoinsMap::iterator it = cacheCoins.find(outpoint);
  if (it != cacheCoins.end())
    return it;
  Coin tmp;
  if (!base->GetCoin(outpoint, tmp))
    return cacheCoins.end();
  CCoinsMap::iterator ret =
      cacheCoins
          .emplace(std::piecewise_construct, std::forward_as_tuple(outpoint),
                   std::forward_as_tuple(std::move(tmp)))
          .first;
  if (ret->second.coin.IsSpent()) {
    ret->second.flags = CCoinsCacheEntry::FRESH;
  }
  cachedCoinsUsage += ret->second.coin.DynamicMemoryUsage();
  return ret;
}

bool CCoinsViewCache::GetCoin(const COutPoint &outpoint, Coin &coin) const {
  CCoinsMap::const_iterator it = FetchCoin(outpoint);
  if (it != cacheCoins.end()) {
    coin = it->second.coin;
    return !coin.IsSpent();
  }
  return false;
}

void CCoinsViewCache::AddCoin(const COutPoint &outpoint, Coin &&coin,
                              bool possible_overwrite) {
  assert(!coin.IsSpent());
  if (coin.out.scriptPubKey.IsUnspendable())
    return;
  CCoinsMap::iterator it;
  bool inserted;
  std::tie(it, inserted) =
      cacheCoins.emplace(std::piecewise_construct,
                         std::forward_as_tuple(outpoint), std::tuple<>());
  bool fresh = false;
  if (!inserted) {
    cachedCoinsUsage -= it->second.coin.DynamicMemoryUsage();
  }
  if (!possible_overwrite) {
    if (!it->second.coin.IsSpent()) {
      throw std::logic_error("Adding new coin that replaces non-pruned entry");
    }
    fresh = !(it->second.flags & CCoinsCacheEntry::DIRTY);
  }
  it->second.coin = std::move(coin);
  it->second.flags |=
      CCoinsCacheEntry::DIRTY | (fresh ? CCoinsCacheEntry::FRESH : 0);
  cachedCoinsUsage += it->second.coin.DynamicMemoryUsage();
}

void AddCoins(CCoinsViewCache &cache, const CTransaction &tx, int nHeight,
              bool check) {
  bool fCoinbase = tx.IsCoinBase();
  const uint256 &txid = tx.GetHash();
  for (size_t i = 0; i < tx.vout.size(); ++i) {
    bool overwrite = check ? cache.HaveCoin(COutPoint(txid, i)) : fCoinbase;

    cache.AddCoin(COutPoint(txid, i), Coin(tx.vout[i], nHeight, fCoinbase),
                  overwrite);
  }
}

bool CCoinsViewCache::SpendCoin(const COutPoint &outpoint, Coin *moveout) {
  CCoinsMap::iterator it = FetchCoin(outpoint);
  if (it == cacheCoins.end())
    return false;
  cachedCoinsUsage -= it->second.coin.DynamicMemoryUsage();
  if (moveout) {
    *moveout = std::move(it->second.coin);
  }
  if (it->second.flags & CCoinsCacheEntry::FRESH) {
    cacheCoins.erase(it);
  } else {
    it->second.flags |= CCoinsCacheEntry::DIRTY;
    it->second.coin.Clear();
  }
  return true;
}

static const Coin coinEmpty;

const Coin &CCoinsViewCache::AccessCoin(const COutPoint &outpoint) const {
  CCoinsMap::const_iterator it = FetchCoin(outpoint);
  if (it == cacheCoins.end()) {
    return coinEmpty;
  } else {
    return it->second.coin;
  }
}

bool CCoinsViewCache::HaveCoin(const COutPoint &outpoint) const {
  CCoinsMap::const_iterator it = FetchCoin(outpoint);
  return (it != cacheCoins.end() && !it->second.coin.IsSpent());
}

bool CCoinsViewCache::HaveCoinInCache(const COutPoint &outpoint) const {
  CCoinsMap::const_iterator it = cacheCoins.find(outpoint);
  return (it != cacheCoins.end() && !it->second.coin.IsSpent());
}

uint256 CCoinsViewCache::GetBestBlock() const {
  if (hashBlock.IsNull())
    hashBlock = base->GetBestBlock();
  return hashBlock;
}

void CCoinsViewCache::SetBestBlock(const uint256 &hashBlockIn) {
  hashBlock = hashBlockIn;
}

bool CCoinsViewCache::BatchWrite(CCoinsMap &mapCoins,
                                 const uint256 &hashBlockIn) {
  for (CCoinsMap::iterator it = mapCoins.begin(); it != mapCoins.end();
       it = mapCoins.erase(it)) {
    if (!(it->second.flags & CCoinsCacheEntry::DIRTY)) {
      continue;
    }
    CCoinsMap::iterator itUs = cacheCoins.find(it->first);
    if (itUs == cacheCoins.end()) {
      if (!(it->second.flags & CCoinsCacheEntry::FRESH &&
            it->second.coin.IsSpent())) {
        CCoinsCacheEntry &entry = cacheCoins[it->first];
        entry.coin = std::move(it->second.coin);
        cachedCoinsUsage += entry.coin.DynamicMemoryUsage();
        entry.flags = CCoinsCacheEntry::DIRTY;

        if (it->second.flags & CCoinsCacheEntry::FRESH) {
          entry.flags |= CCoinsCacheEntry::FRESH;
        }
      }
    } else {
      if ((it->second.flags & CCoinsCacheEntry::FRESH) &&
          !itUs->second.coin.IsSpent()) {
        throw std::logic_error("FRESH flag misapplied to cache entry for base "
                               "transaction with spendable outputs");
      }

      if ((itUs->second.flags & CCoinsCacheEntry::FRESH) &&
          it->second.coin.IsSpent()) {
        cachedCoinsUsage -= itUs->second.coin.DynamicMemoryUsage();
        cacheCoins.erase(itUs);
      } else {
        cachedCoinsUsage -= itUs->second.coin.DynamicMemoryUsage();
        itUs->second.coin = std::move(it->second.coin);
        cachedCoinsUsage += itUs->second.coin.DynamicMemoryUsage();
        itUs->second.flags |= CCoinsCacheEntry::DIRTY;
      }
    }
  }
  hashBlock = hashBlockIn;
  return true;
}

bool CCoinsViewCache::Flush() {
  bool fOk = base->BatchWrite(cacheCoins, hashBlock);
  cacheCoins.clear();
  cachedCoinsUsage = 0;
  return fOk;
}

void CCoinsViewCache::Uncache(const COutPoint &hash) {
  CCoinsMap::iterator it = cacheCoins.find(hash);
  if (it != cacheCoins.end() && it->second.flags == 0) {
    cachedCoinsUsage -= it->second.coin.DynamicMemoryUsage();
    cacheCoins.erase(it);
  }
}

unsigned int CCoinsViewCache::GetCacheSize() const { return cacheCoins.size(); }

CAmount CCoinsViewCache::GetValueIn(const CTransaction &tx) const {
  if (tx.IsCoinBase())
    return 0;

  CAmount nResult = 0;
  for (unsigned int i = 0; i < tx.vin.size(); i++)
    nResult += AccessCoin(tx.vin[i].prevout).out.nValue;

  return nResult;
}

bool CCoinsViewCache::HaveInputs(const CTransaction &tx) const {
  if (!tx.IsCoinBase()) {
    for (unsigned int i = 0; i < tx.vin.size(); i++) {
      if (!HaveCoin(tx.vin[i].prevout)) {
        return false;
      }
    }
  }
  return true;
}

static const size_t MIN_TRANSACTION_OUTPUT_WEIGHT =
    WITNESS_SCALE_FACTOR *
    ::GetSerializeSize(CTxOut(), SER_NETWORK, PROTOCOL_VERSION);
static const size_t MAX_OUTPUTS_PER_BLOCK =
    MAX_BLOCK_WEIGHT / MIN_TRANSACTION_OUTPUT_WEIGHT;

const Coin &AccessByTxid(const CCoinsViewCache &view, const uint256 &txid) {
  COutPoint iter(txid, 0);
  while (iter.n < MAX_OUTPUTS_PER_BLOCK) {
    const Coin &alternate = view.AccessCoin(iter);
    if (!alternate.IsSpent())
      return alternate;
    ++iter.n;
  }
  return coinEmpty;
}
