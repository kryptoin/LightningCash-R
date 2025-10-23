// Copyright (c) 2012-2025 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <limitedmap.h>

#include <test/test_bitcoin.h>

#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE(limitedmap_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(limitedmap_test) {
  limitedmap<int, int> map(10);

  BOOST_CHECK(map.max_size() == 10);

  BOOST_CHECK(map.size() == 0);

  map.insert(std::pair<int, int>(-1, -1));

  BOOST_CHECK(map.size() == 1);

  BOOST_CHECK(map.count(-1) == 1);

  for (int i = 0; i < 10; i++) {
    map.insert(std::pair<int, int>(i, i + 1));
  }

  BOOST_CHECK(map.size() == 10);

  BOOST_CHECK(map.count(-1) == 0);

  limitedmap<int, int>::const_iterator it = map.begin();
  for (int i = 0; i < 10; i++) {
    BOOST_CHECK(map.count(i) == 1);

    BOOST_CHECK(it->first == i);
    BOOST_CHECK(it->second == i + 1);

    BOOST_CHECK(map.find(i)->second == i + 1);

    map.update(it, i + 2);
    BOOST_CHECK(map.find(i)->second == i + 2);

    it++;
  }

  BOOST_CHECK(it == map.end());

  map.max_size(5);

  BOOST_CHECK(map.max_size() == 5);
  BOOST_CHECK(map.size() == 5);

  for (int i = 0; i < 10; i++) {
    if (i < 5) {
      BOOST_CHECK(map.count(i) == 0);
    } else {
      BOOST_CHECK(map.count(i) == 1);
    }
  }

  for (int i = 100; i < 1000; i += 100) {
    map.erase(i);
  }

  BOOST_CHECK(map.size() == 5);

  for (int i = 5; i < 10; i++) {
    map.erase(i);
  }

  BOOST_CHECK(map.empty());
}

BOOST_AUTO_TEST_SUITE_END()
