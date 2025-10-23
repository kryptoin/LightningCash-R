// Copyright (c) 2012-2025 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <dbwrapper.h>
#include <random.h>
#include <test/test_bitcoin.h>
#include <uint256.h>

#include <boost/test/unit_test.hpp>

bool is_null_key(const std::vector<unsigned char> &key) {
  bool isnull = true;

  for (unsigned int i = 0; i < key.size(); i++)
    isnull &= (key[i] == '\x00');

  return isnull;
}

BOOST_FIXTURE_TEST_SUITE(dbwrapper_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(dbwrapper) {
  for (bool obfuscate : {false, true}) {
    fs::path ph = fs::temp_directory_path() / fs::unique_path();
    CDBWrapper dbw(ph, (1 << 20), true, false, obfuscate);
    char key = 'k';
    uint256 in = InsecureRand256();
    uint256 res;

    BOOST_CHECK(obfuscate !=
                is_null_key(dbwrapper_private::GetObfuscateKey(dbw)));

    BOOST_CHECK(dbw.Write(key, in));
    BOOST_CHECK(dbw.Read(key, res));
    BOOST_CHECK_EQUAL(res.ToString(), in.ToString());
  }
}

BOOST_AUTO_TEST_CASE(dbwrapper_batch) {
  for (bool obfuscate : {false, true}) {
    fs::path ph = fs::temp_directory_path() / fs::unique_path();
    CDBWrapper dbw(ph, (1 << 20), true, false, obfuscate);

    char key = 'i';
    uint256 in = InsecureRand256();
    char key2 = 'j';
    uint256 in2 = InsecureRand256();
    char key3 = 'k';
    uint256 in3 = InsecureRand256();

    uint256 res;
    CDBBatch batch(dbw);

    batch.Write(key, in);
    batch.Write(key2, in2);
    batch.Write(key3, in3);

    batch.Erase(key3);

    dbw.WriteBatch(batch);

    BOOST_CHECK(dbw.Read(key, res));
    BOOST_CHECK_EQUAL(res.ToString(), in.ToString());
    BOOST_CHECK(dbw.Read(key2, res));
    BOOST_CHECK_EQUAL(res.ToString(), in2.ToString());

    BOOST_CHECK(dbw.Read(key3, res) == false);
  }
}

BOOST_AUTO_TEST_CASE(dbwrapper_iterator) {
  for (bool obfuscate : {false, true}) {
    fs::path ph = fs::temp_directory_path() / fs::unique_path();
    CDBWrapper dbw(ph, (1 << 20), true, false, obfuscate);

    char key = 'j';
    uint256 in = InsecureRand256();
    BOOST_CHECK(dbw.Write(key, in));
    char key2 = 'k';
    uint256 in2 = InsecureRand256();
    BOOST_CHECK(dbw.Write(key2, in2));

    std::unique_ptr<CDBIterator> it(
        const_cast<CDBWrapper &>(dbw).NewIterator());

    it->Seek(key);

    char key_res;
    uint256 val_res;

    it->GetKey(key_res);
    it->GetValue(val_res);
    BOOST_CHECK_EQUAL(key_res, key);
    BOOST_CHECK_EQUAL(val_res.ToString(), in.ToString());

    it->Next();

    it->GetKey(key_res);
    it->GetValue(val_res);
    BOOST_CHECK_EQUAL(key_res, key2);
    BOOST_CHECK_EQUAL(val_res.ToString(), in2.ToString());

    it->Next();
    BOOST_CHECK_EQUAL(it->Valid(), false);
  }
}

BOOST_AUTO_TEST_CASE(existing_data_no_obfuscate) {
  fs::path ph = fs::temp_directory_path() / fs::unique_path();
  create_directories(ph);

  std::unique_ptr<CDBWrapper> dbw =
      MakeUnique<CDBWrapper>(ph, (1 << 10), false, false, false);
  char key = 'k';
  uint256 in = InsecureRand256();
  uint256 res;

  BOOST_CHECK(dbw->Write(key, in));
  BOOST_CHECK(dbw->Read(key, res));
  BOOST_CHECK_EQUAL(res.ToString(), in.ToString());

  dbw.reset();

  CDBWrapper odbw(ph, (1 << 10), false, false, true);

  uint256 res2;
  BOOST_CHECK(odbw.Read(key, res2));
  BOOST_CHECK_EQUAL(res2.ToString(), in.ToString());

  BOOST_CHECK(!odbw.IsEmpty());

  BOOST_CHECK(is_null_key(dbwrapper_private::GetObfuscateKey(odbw)));

  uint256 in2 = InsecureRand256();
  uint256 res3;

  BOOST_CHECK(odbw.Write(key, in2));
  BOOST_CHECK(odbw.Read(key, res3));
  BOOST_CHECK_EQUAL(res3.ToString(), in2.ToString());
}

BOOST_AUTO_TEST_CASE(existing_data_reindex) {
  fs::path ph = fs::temp_directory_path() / fs::unique_path();
  create_directories(ph);

  std::unique_ptr<CDBWrapper> dbw =
      MakeUnique<CDBWrapper>(ph, (1 << 10), false, false, false);
  char key = 'k';
  uint256 in = InsecureRand256();
  uint256 res;

  BOOST_CHECK(dbw->Write(key, in));
  BOOST_CHECK(dbw->Read(key, res));
  BOOST_CHECK_EQUAL(res.ToString(), in.ToString());

  dbw.reset();

  CDBWrapper odbw(ph, (1 << 10), false, true, true);

  uint256 res2;
  BOOST_CHECK(!odbw.Read(key, res2));
  BOOST_CHECK(!is_null_key(dbwrapper_private::GetObfuscateKey(odbw)));

  uint256 in2 = InsecureRand256();
  uint256 res3;

  BOOST_CHECK(odbw.Write(key, in2));
  BOOST_CHECK(odbw.Read(key, res3));
  BOOST_CHECK_EQUAL(res3.ToString(), in2.ToString());
}

BOOST_AUTO_TEST_CASE(iterator_ordering) {
  fs::path ph = fs::temp_directory_path() / fs::unique_path();
  CDBWrapper dbw(ph, (1 << 20), true, false, false);
  for (int x = 0x00; x < 256; ++x) {
    uint8_t key = x;
    uint32_t value = x * x;
    if (!(x & 1))
      BOOST_CHECK(dbw.Write(key, value));
  }

  std::unique_ptr<CDBIterator> it(const_cast<CDBWrapper &>(dbw).NewIterator());

  for (int x = 0x00; x < 256; ++x) {
    uint8_t key = x;
    uint32_t value = x * x;
    if (x & 1)
      BOOST_CHECK(dbw.Write(key, value));
  }

  for (int seek_start : {0x00, 0x80}) {
    it->Seek((uint8_t)seek_start);
    for (int x = seek_start; x < 255; ++x) {
      uint8_t key;
      uint32_t value;
      BOOST_CHECK(it->Valid());
      if (!it->Valid())

        break;
      BOOST_CHECK(it->GetKey(key));
      if (x & 1) {
        BOOST_CHECK_EQUAL(key, x + 1);
        continue;
      }
      BOOST_CHECK(it->GetValue(value));
      BOOST_CHECK_EQUAL(key, x);
      BOOST_CHECK_EQUAL(value, x * x);
      it->Next();
    }
    BOOST_CHECK(!it->Valid());
  }
}

struct StringContentsSerializer {
  std::string str;
  StringContentsSerializer() {}
  explicit StringContentsSerializer(const std::string &inp) : str(inp) {}

  StringContentsSerializer &operator+=(const std::string &s) {
    str += s;
    return *this;
  }
  StringContentsSerializer &operator+=(const StringContentsSerializer &s) {
    return *this += s.str;
  }

  ADD_SERIALIZE_METHODS;

  template <typename Stream, typename Operation>
  inline void SerializationOp(Stream &s, Operation ser_action) {
    if (ser_action.ForRead()) {
      str.clear();
      char c = 0;
      while (true) {
        try {
          READWRITE(c);
          str.push_back(c);
        } catch (const std::ios_base::failure &e) {
          break;
        }
      }
    } else {
      for (size_t i = 0; i < str.size(); i++)
        READWRITE(str[i]);
    }
  }
};

BOOST_AUTO_TEST_CASE(iterator_string_ordering) {
  char buf[10];

  fs::path ph = fs::temp_directory_path() / fs::unique_path();
  CDBWrapper dbw(ph, (1 << 20), true, false, false);
  for (int x = 0x00; x < 10; ++x) {
    for (int y = 0; y < 10; y++) {
      snprintf(buf, sizeof(buf), "%d", x);
      StringContentsSerializer key(buf);
      for (int z = 0; z < y; z++)
        key += key;
      uint32_t value = x * x;
      BOOST_CHECK(dbw.Write(key, value));
    }
  }

  std::unique_ptr<CDBIterator> it(const_cast<CDBWrapper &>(dbw).NewIterator());
  for (int seek_start : {0, 5}) {
    snprintf(buf, sizeof(buf), "%d", seek_start);
    StringContentsSerializer seek_key(buf);
    it->Seek(seek_key);
    for (int x = seek_start; x < 10; ++x) {
      for (int y = 0; y < 10; y++) {
        snprintf(buf, sizeof(buf), "%d", x);
        std::string exp_key(buf);
        for (int z = 0; z < y; z++)
          exp_key += exp_key;
        StringContentsSerializer key;
        uint32_t value;
        BOOST_CHECK(it->Valid());
        if (!it->Valid())

          break;
        BOOST_CHECK(it->GetKey(key));
        BOOST_CHECK(it->GetValue(value));
        BOOST_CHECK_EQUAL(key.str, exp_key);
        BOOST_CHECK_EQUAL(value, x * x);
        it->Next();
      }
    }
    BOOST_CHECK(!it->Valid());
  }
}

BOOST_AUTO_TEST_SUITE_END()
