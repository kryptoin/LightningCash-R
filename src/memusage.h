// Copyright (c) 2015-2025 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_MEMUSAGE_H
#define BITCOIN_MEMUSAGE_H

#include <indirectmap.h>

#include <stdlib.h>

#include <map>
#include <set>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace memusage
{

static size_t MallocUsage(size_t alloc);

static inline size_t DynamicUsage(const int8_t& v) { return 0; }
static inline size_t DynamicUsage(const uint8_t& v) { return 0; }
static inline size_t DynamicUsage(const int16_t& v) { return 0; }
static inline size_t DynamicUsage(const uint16_t& v) { return 0; }
static inline size_t DynamicUsage(const int32_t& v) { return 0; }
static inline size_t DynamicUsage(const uint32_t& v) { return 0; }
static inline size_t DynamicUsage(const int64_t& v) { return 0; }
static inline size_t DynamicUsage(const uint64_t& v) { return 0; }
static inline size_t DynamicUsage(const float& v) { return 0; }
static inline size_t DynamicUsage(const double& v) { return 0; }
template<typename X> static inline size_t DynamicUsage(X * const &v) { return 0; }
template<typename X> static inline size_t DynamicUsage(const X * const &v) { return 0; }

static inline size_t MallocUsage(size_t alloc)
{
    // Measured on libc6 2.19 on Linux.
    if (alloc == 0) {
        return 0;
    } else if (sizeof(void*) == 8) {
        return ((alloc + 31) >> 4) << 4;
    } else if (sizeof(void*) == 4) {
        return ((alloc + 15) >> 3) << 3;
    } else {
        assert(0);
    }
}

// STL data structures

template<typename X>
struct stl_tree_node
{
private:
    int color;
    void* parent;
    void* left;
    void* right;
    X x;
};

struct stl_shared_counter
{

    void* class_type;
    size_t use_count;
    size_t weak_count;
};

template<typename X>
static inline size_t DynamicUsage(const std::vector<X>& v)
{
    return MallocUsage(v.capacity() * sizeof(X));
}

template<unsigned int N, typename X, typename S, typename D>
static inline size_t DynamicUsage(const prevector<N, X, S, D>& v)
{
    return MallocUsage(v.allocated_memory());
}

template<typename X, typename Y>
static inline size_t DynamicUsage(const std::set<X, Y>& s)
{
    return MallocUsage(sizeof(stl_tree_node<X>)) * s.size();
}

template<typename X, typename Y>
static inline size_t IncrementalDynamicUsage(const std::set<X, Y>& s)
{
    return MallocUsage(sizeof(stl_tree_node<X>));
}

template<typename X, typename Y, typename Z>
static inline size_t DynamicUsage(const std::map<X, Y, Z>& m)
{
    return MallocUsage(sizeof(stl_tree_node<std::pair<const X, Y> >)) * m.size();
}

template<typename X, typename Y, typename Z>
static inline size_t IncrementalDynamicUsage(const std::map<X, Y, Z>& m)
{
    return MallocUsage(sizeof(stl_tree_node<std::pair<const X, Y> >));
}

// indirectmap has underlying map with pointer as key

template<typename X, typename Y>
static inline size_t DynamicUsage(const indirectmap<X, Y>& m)
{
    return MallocUsage(sizeof(stl_tree_node<std::pair<const X*, Y> >)) * m.size();
}

template<typename X, typename Y>
static inline size_t IncrementalDynamicUsage(const indirectmap<X, Y>& m)
{
    return MallocUsage(sizeof(stl_tree_node<std::pair<const X*, Y> >));
}

template<typename X>
static inline size_t DynamicUsage(const std::unique_ptr<X>& p)
{
    return p ? MallocUsage(sizeof(X)) : 0;
}

template<typename X>
static inline size_t DynamicUsage(const std::shared_ptr<X>& p)
{
    // A shared_ptr can either use a single continuous memory block for both
    // the counter and the storage (when using std::make_shared), or separate.
    // We can't observe the difference, however, so assume the worst.
    return p ? MallocUsage(sizeof(X)) + MallocUsage(sizeof(stl_shared_counter)) : 0;
}

template<typename X>
struct unordered_node : private X
{
private:
    void* ptr;
};

template<typename X, typename Y>
static inline size_t DynamicUsage(const std::unordered_set<X, Y>& s)
{
    return MallocUsage(sizeof(unordered_node<X>)) * s.size() + MallocUsage(sizeof(void*) * s.bucket_count());
}

template<typename X, typename Y, typename Z>
static inline size_t DynamicUsage(const std::unordered_map<X, Y, Z>& m)
{
    return MallocUsage(sizeof(unordered_node<std::pair<const X, Y> >)) * m.size() + MallocUsage(sizeof(void*) * m.bucket_count());
}

}

#endif // BITCOIN_MEMUSAGE_H
