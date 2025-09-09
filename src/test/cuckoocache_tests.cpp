// Copyright (c) 2012-2025 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include <boost/test/unit_test.hpp>
#include <cuckoocache.h>
#include <script/sigcache.h>
#include <test/test_bitcoin.h>
#include <random.h>
#include <thread>

FastRandomContext local_rand_ctx(true);

BOOST_AUTO_TEST_SUITE(cuckoocache_tests);

void insecure_GetRandHash(uint256& t)
{
    uint32_t* ptr = (uint32_t*)t.begin();
    for (uint8_t j = 0; j < 8; ++j)
        *(ptr++) = local_rand_ctx.rand32();
}

BOOST_AUTO_TEST_CASE(test_cuckoocache_no_fakes)
{
    local_rand_ctx = FastRandomContext(true);
    CuckooCache::cache<uint256, SignatureCacheHasher> cc{};
    size_t megabytes = 4;
    cc.setup_bytes(megabytes << 20);
    uint256 v;
    for (int x = 0; x < 100000; ++x) {
        insecure_GetRandHash(v);
        cc.insert(v);
    }
    for (int x = 0; x < 100000; ++x) {
        insecure_GetRandHash(v);
        BOOST_CHECK(!cc.contains(v, false));
    }
};

template <typename Cache>
double test_cache(size_t megabytes, double load)
{
    local_rand_ctx = FastRandomContext(true);
    std::vector<uint256> hashes;
    Cache set{};
    size_t bytes = megabytes * (1 << 20);
    set.setup_bytes(bytes);
    uint32_t n_insert = static_cast<uint32_t>(load * (bytes / sizeof(uint256)));
    hashes.resize(n_insert);
    for (uint32_t i = 0; i < n_insert; ++i) {
        uint32_t* ptr = (uint32_t*)hashes[i].begin();
        for (uint8_t j = 0; j < 8; ++j)
            *(ptr++) = local_rand_ctx.rand32();
    }

    std::vector<uint256> hashes_insert_copy = hashes;

    for (uint256& h : hashes_insert_copy)
        set.insert(h);

    uint32_t count = 0;
    for (uint256& h : hashes)
        count += set.contains(h, false);
    double hit_rate = ((double)count) / ((double)n_insert);
    return hit_rate;
}

double normalize_hit_rate(double hits, double load)
{
    return hits * std::max(load, 1.0);
}

BOOST_AUTO_TEST_CASE(cuckoocache_hit_rate_ok)
{

    double HitRateThresh = 0.98;
    size_t megabytes = 4;
    for (double load = 0.1; load < 2; load *= 2) {
        double hits = test_cache<CuckooCache::cache<uint256, SignatureCacheHasher>>(megabytes, load);
        BOOST_CHECK(normalize_hit_rate(hits, load) > HitRateThresh);
    }
}

template <typename Cache>
void test_cache_erase(size_t megabytes)
{
    double load = 1;
    local_rand_ctx = FastRandomContext(true);
    std::vector<uint256> hashes;
    Cache set{};
    size_t bytes = megabytes * (1 << 20);
    set.setup_bytes(bytes);
    uint32_t n_insert = static_cast<uint32_t>(load * (bytes / sizeof(uint256)));
    hashes.resize(n_insert);
    for (uint32_t i = 0; i < n_insert; ++i) {
        uint32_t* ptr = (uint32_t*)hashes[i].begin();
        for (uint8_t j = 0; j < 8; ++j)
            *(ptr++) = local_rand_ctx.rand32();
    }

    std::vector<uint256> hashes_insert_copy = hashes;

    for (uint32_t i = 0; i < (n_insert / 2); ++i)
        set.insert(hashes_insert_copy[i]);

    for (uint32_t i = 0; i < (n_insert / 4); ++i)
        set.contains(hashes[i], true);

    for (uint32_t i = (n_insert / 2); i < n_insert; ++i)
        set.insert(hashes_insert_copy[i]);

    size_t count_erased_but_contained = 0;

    size_t count_stale = 0;

    size_t count_fresh = 0;

    for (uint32_t i = 0; i < (n_insert / 4); ++i)
        count_erased_but_contained += set.contains(hashes[i], false);
    for (uint32_t i = (n_insert / 4); i < (n_insert / 2); ++i)
        count_stale += set.contains(hashes[i], false);
    for (uint32_t i = (n_insert / 2); i < n_insert; ++i)
        count_fresh += set.contains(hashes[i], false);

    double hit_rate_erased_but_contained = double(count_erased_but_contained) / (double(n_insert) / 4.0);
    double hit_rate_stale = double(count_stale) / (double(n_insert) / 4.0);
    double hit_rate_fresh = double(count_fresh) / (double(n_insert) / 2.0);

    // Check that our hit_rate_fresh is perfect
    BOOST_CHECK_EQUAL(hit_rate_fresh, 1.0);
    // Check that we have a more than 2x better hit rate on stale elements than
    // erased elements.
    BOOST_CHECK(hit_rate_stale > 2 * hit_rate_erased_but_contained);
}

BOOST_AUTO_TEST_CASE(cuckoocache_erase_ok)
{
    size_t megabytes = 4;
    test_cache_erase<CuckooCache::cache<uint256, SignatureCacheHasher>>(megabytes);
}

template <typename Cache>
void test_cache_erase_parallel(size_t megabytes)
{
    double load = 1;
    local_rand_ctx = FastRandomContext(true);
    std::vector<uint256> hashes;
    Cache set{};
    size_t bytes = megabytes * (1 << 20);
    set.setup_bytes(bytes);
    uint32_t n_insert = static_cast<uint32_t>(load * (bytes / sizeof(uint256)));
    hashes.resize(n_insert);
    for (uint32_t i = 0; i < n_insert; ++i) {
        uint32_t* ptr = (uint32_t*)hashes[i].begin();
        for (uint8_t j = 0; j < 8; ++j)
            *(ptr++) = local_rand_ctx.rand32();
    }

    std::vector<uint256> hashes_insert_copy = hashes;
    boost::shared_mutex mtx;

    {

        boost::unique_lock<boost::shared_mutex> l(mtx);

        for (uint32_t i = 0; i < (n_insert / 2); ++i)
            set.insert(hashes_insert_copy[i]);
    }

    std::vector<std::thread> threads;

    for (uint32_t x = 0; x < 3; ++x)

        threads.emplace_back([&, x] {
            boost::shared_lock<boost::shared_mutex> l(mtx);
            size_t ntodo = (n_insert/4)/3;
            size_t start = ntodo*x;
            size_t end = ntodo*(x+1);
            for (uint32_t i = start; i < end; ++i)
                set.contains(hashes[i], true);
        });

    for (std::thread& t : threads)
        t.join();

    boost::unique_lock<boost::shared_mutex> l(mtx);

    for (uint32_t i = (n_insert / 2); i < n_insert; ++i)
        set.insert(hashes_insert_copy[i]);

    size_t count_erased_but_contained = 0;

    size_t count_stale = 0;

    size_t count_fresh = 0;

    for (uint32_t i = 0; i < (n_insert / 4); ++i)
        count_erased_but_contained += set.contains(hashes[i], false);
    for (uint32_t i = (n_insert / 4); i < (n_insert / 2); ++i)
        count_stale += set.contains(hashes[i], false);
    for (uint32_t i = (n_insert / 2); i < n_insert; ++i)
        count_fresh += set.contains(hashes[i], false);

    double hit_rate_erased_but_contained = double(count_erased_but_contained) / (double(n_insert) / 4.0);
    double hit_rate_stale = double(count_stale) / (double(n_insert) / 4.0);
    double hit_rate_fresh = double(count_fresh) / (double(n_insert) / 2.0);

    // Check that our hit_rate_fresh is perfect
    BOOST_CHECK_EQUAL(hit_rate_fresh, 1.0);
    // Check that we have a more than 2x better hit rate on stale elements than
    // erased elements.
    BOOST_CHECK(hit_rate_stale > 2 * hit_rate_erased_but_contained);
}
BOOST_AUTO_TEST_CASE(cuckoocache_erase_parallel_ok)
{
    size_t megabytes = 4;
    test_cache_erase_parallel<CuckooCache::cache<uint256, SignatureCacheHasher>>(megabytes);
}

template <typename Cache>
void test_cache_generations()
{
    // This test checks that for a simulation of network activity, the fresh hit
    // rate is never below 99%, and the number of times that it is worse than
    // 99.9% are less than 1% of the time.
    double min_hit_rate = 0.99;
    double tight_hit_rate = 0.999;
    double max_rate_less_than_tight_hit_rate = 0.01;
    // A cache that meets this specification is therefore shown to have a hit
    // rate of at least tight_hit_rate * (1 - max_rate_less_than_tight_hit_rate) +
    // min_hit_rate*max_rate_less_than_tight_hit_rate = 0.999*99%+0.99*1% == 99.89%
    // hit rate with low variance.

    // We use deterministic values, but this test has also passed on many
    // iterations with non-deterministic values, so it isn't "overfit" to the
    // specific entropy in FastRandomContext(true) and implementation of the
    // cache.
    local_rand_ctx = FastRandomContext(true);

    // block_activity models a chunk of network activity. n_insert elements are
    // adde to the cache. The first and last n/4 are stored for removal later
    // and the middle n/2 are not stored. This models a network which uses half
    // the signatures of recently (since the last block) added transactions
    // immediately and never uses the other half.
    struct block_activity {
        std::vector<uint256> reads;
        block_activity(uint32_t n_insert, Cache& c) : reads()
        {
            std::vector<uint256> inserts;
            inserts.resize(n_insert);
            reads.reserve(n_insert / 2);
            for (uint32_t i = 0; i < n_insert; ++i) {
                uint32_t* ptr = (uint32_t*)inserts[i].begin();
                for (uint8_t j = 0; j < 8; ++j)
                    *(ptr++) = local_rand_ctx.rand32();
            }
            for (uint32_t i = 0; i < n_insert / 4; ++i)
                reads.push_back(inserts[i]);
            for (uint32_t i = n_insert - (n_insert / 4); i < n_insert; ++i)
                reads.push_back(inserts[i]);
            for (auto h : inserts)
                c.insert(h);
        }
    };

    const uint32_t BLOCK_SIZE = 1000;
    // We expect window size 60 to perform reasonably given that each epoch
    // stores 45% of the cache size (~472k).
    const uint32_t WINDOW_SIZE = 60;
    const uint32_t POP_AMOUNT = (BLOCK_SIZE / WINDOW_SIZE) / 2;
    const double load = 10;
    const size_t megabytes = 4;
    const size_t bytes = megabytes * (1 << 20);
    const uint32_t n_insert = static_cast<uint32_t>(load * (bytes / sizeof(uint256)));

    std::vector<block_activity> hashes;
    Cache set{};
    set.setup_bytes(bytes);
    hashes.reserve(n_insert / BLOCK_SIZE);
    std::deque<block_activity> last_few;
    uint32_t out_of_tight_tolerance = 0;
    uint32_t total = n_insert / BLOCK_SIZE;
    // we use the deque last_few to model a sliding window of blocks. at each
    // step, each of the last WINDOW_SIZE block_activities checks the cache for
    // POP_AMOUNT of the hashes that they inserted, and marks these erased.
    for (uint32_t i = 0; i < total; ++i) {
        if (last_few.size() == WINDOW_SIZE)
            last_few.pop_front();
        last_few.emplace_back(BLOCK_SIZE, set);
        uint32_t count = 0;
        for (auto& act : last_few)
            for (uint32_t k = 0; k < POP_AMOUNT; ++k) {
                count += set.contains(act.reads.back(), true);
                act.reads.pop_back();
            }
        // We use last_few.size() rather than WINDOW_SIZE for the correct
        // behavior on the first WINDOW_SIZE iterations where the deque is not
        // full yet.
        double hit = (double(count)) / (last_few.size() * POP_AMOUNT);
        // Loose Check that hit rate is above min_hit_rate
        BOOST_CHECK(hit > min_hit_rate);
        // Tighter check, count number of times we are less than tight_hit_rate
        // (and implicitly, greater than min_hit_rate)
        out_of_tight_tolerance += hit < tight_hit_rate;
    }
    // Check that being out of tolerance happens less than
    // max_rate_less_than_tight_hit_rate of the time
    BOOST_CHECK(double(out_of_tight_tolerance) / double(total) < max_rate_less_than_tight_hit_rate);
}
BOOST_AUTO_TEST_CASE(cuckoocache_generations)
{
    test_cache_generations<CuckooCache::cache<uint256, SignatureCacheHasher>>();
}

BOOST_AUTO_TEST_SUITE_END();
