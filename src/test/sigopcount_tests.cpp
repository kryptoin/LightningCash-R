// Copyright (c) 2012-2025 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <consensus/tx_verify.h>
#include <consensus/validation.h>
#include <key.h>
#include <pubkey.h>
#include <script/script.h>
#include <script/standard.h>
#include <test/test_bitcoin.h>
#include <uint256.h>

#include <vector>

#include <boost/test/unit_test.hpp>

static std::vector<unsigned char> Serialize(const CScript &s) {
  std::vector<unsigned char> sSerialized(s.begin(), s.end());
  return sSerialized;
}

BOOST_FIXTURE_TEST_SUITE(sigopcount_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(GetSigOpCount) {
  CScript s1;
  BOOST_CHECK_EQUAL(s1.GetSigOpCount(false), 0U);
  BOOST_CHECK_EQUAL(s1.GetSigOpCount(true), 0U);

  uint160 dummy;
  s1 << OP_1 << ToByteVector(dummy) << ToByteVector(dummy) << OP_2
     << OP_CHECKMULTISIG;
  BOOST_CHECK_EQUAL(s1.GetSigOpCount(true), 2U);
  s1 << OP_IF << OP_CHECKSIG << OP_ENDIF;
  BOOST_CHECK_EQUAL(s1.GetSigOpCount(true), 3U);
  BOOST_CHECK_EQUAL(s1.GetSigOpCount(false), 21U);

  CScript p2sh = GetScriptForDestination(CScriptID(s1));
  CScript scriptSig;
  scriptSig << OP_0 << Serialize(s1);
  BOOST_CHECK_EQUAL(p2sh.GetSigOpCount(scriptSig), 3U);

  std::vector<CPubKey> keys;
  for (int i = 0; i < 3; i++) {
    CKey k;
    k.MakeNewKey(true);
    keys.push_back(k.GetPubKey());
  }
  CScript s2 = GetScriptForMultisig(1, keys);
  BOOST_CHECK_EQUAL(s2.GetSigOpCount(true), 3U);
  BOOST_CHECK_EQUAL(s2.GetSigOpCount(false), 20U);

  p2sh = GetScriptForDestination(CScriptID(s2));
  BOOST_CHECK_EQUAL(p2sh.GetSigOpCount(true), 0U);
  BOOST_CHECK_EQUAL(p2sh.GetSigOpCount(false), 0U);
  CScript scriptSig2;
  scriptSig2 << OP_1 << ToByteVector(dummy) << ToByteVector(dummy)
             << Serialize(s2);
  BOOST_CHECK_EQUAL(p2sh.GetSigOpCount(scriptSig2), 3U);
}

ScriptError VerifyWithFlag(const CTransaction &output,
                           const CMutableTransaction &input, int flags) {
  ScriptError error;
  CTransaction inputi(input);
  bool ret = VerifyScript(
      inputi.vin[0].scriptSig, output.vout[0].scriptPubKey,
      &inputi.vin[0].scriptWitness, flags,
      TransactionSignatureChecker(&inputi, 0, output.vout[0].nValue), &error);
  BOOST_CHECK((ret == true) == (error == SCRIPT_ERR_OK));

  return error;
}

void BuildTxs(CMutableTransaction &spendingTx, CCoinsViewCache &coins,
              CMutableTransaction &creationTx, const CScript &scriptPubKey,
              const CScript &scriptSig, const CScriptWitness &witness) {
  creationTx.nVersion = 1;
  creationTx.vin.resize(1);
  creationTx.vin[0].prevout.SetNull();
  creationTx.vin[0].scriptSig = CScript();
  creationTx.vout.resize(1);
  creationTx.vout[0].nValue = 1;
  creationTx.vout[0].scriptPubKey = scriptPubKey;

  spendingTx.nVersion = 1;
  spendingTx.vin.resize(1);
  spendingTx.vin[0].prevout.hash = creationTx.GetHash();
  spendingTx.vin[0].prevout.n = 0;
  spendingTx.vin[0].scriptSig = scriptSig;
  spendingTx.vin[0].scriptWitness = witness;
  spendingTx.vout.resize(1);
  spendingTx.vout[0].nValue = 1;
  spendingTx.vout[0].scriptPubKey = CScript();

  AddCoins(coins, creationTx, 0);
}

BOOST_AUTO_TEST_CASE(GetTxSigOpCost) {
  CMutableTransaction creationTx;

  CMutableTransaction spendingTx;

  CCoinsView coinsDummy;
  CCoinsViewCache coins(&coinsDummy);

  CKey key;
  key.MakeNewKey(true);
  CPubKey pubkey = key.GetPubKey();

  int flags = SCRIPT_VERIFY_WITNESS | SCRIPT_VERIFY_P2SH;

  {
    CScript scriptPubKey = CScript()
                           << 1 << ToByteVector(pubkey) << ToByteVector(pubkey)
                           << 2 << OP_CHECKMULTISIGVERIFY;

    CScript scriptSig = CScript() << OP_0 << OP_0;

    BuildTxs(spendingTx, coins, creationTx, scriptPubKey, scriptSig,
             CScriptWitness());

    assert(GetTransactionSigOpCost(CTransaction(spendingTx), coins, flags) ==
           0);

    assert(GetTransactionSigOpCost(CTransaction(creationTx), coins, flags) ==
           MAX_PUBKEYS_PER_MULTISIG * WITNESS_SCALE_FACTOR);

    assert(VerifyWithFlag(creationTx, spendingTx, flags) ==
           SCRIPT_ERR_CHECKMULTISIGVERIFY);
  }

  {
    CScript redeemScript = CScript()
                           << 1 << ToByteVector(pubkey) << ToByteVector(pubkey)
                           << 2 << OP_CHECKMULTISIGVERIFY;
    CScript scriptPubKey = GetScriptForDestination(CScriptID(redeemScript));
    CScript scriptSig = CScript() << OP_0 << OP_0 << ToByteVector(redeemScript);

    BuildTxs(spendingTx, coins, creationTx, scriptPubKey, scriptSig,
             CScriptWitness());
    assert(GetTransactionSigOpCost(CTransaction(spendingTx), coins, flags) ==
           2 * WITNESS_SCALE_FACTOR);
    assert(VerifyWithFlag(creationTx, spendingTx, flags) ==
           SCRIPT_ERR_CHECKMULTISIGVERIFY);
  }

  {
    CScript p2pk = CScript() << ToByteVector(pubkey) << OP_CHECKSIG;
    CScript scriptPubKey = GetScriptForWitness(p2pk);
    CScript scriptSig = CScript();
    CScriptWitness scriptWitness;
    scriptWitness.stack.push_back(std::vector<unsigned char>(0));
    scriptWitness.stack.push_back(std::vector<unsigned char>(0));

    BuildTxs(spendingTx, coins, creationTx, scriptPubKey, scriptSig,
             scriptWitness);
    assert(GetTransactionSigOpCost(CTransaction(spendingTx), coins, flags) ==
           1);

    assert(GetTransactionSigOpCost(CTransaction(spendingTx), coins,
                                   flags & ~SCRIPT_VERIFY_WITNESS) == 0);
    assert(VerifyWithFlag(creationTx, spendingTx, flags) ==
           SCRIPT_ERR_EQUALVERIFY);

    assert(scriptPubKey[0] == 0x00);
    scriptPubKey[0] = 0x51;
    BuildTxs(spendingTx, coins, creationTx, scriptPubKey, scriptSig,
             scriptWitness);
    assert(GetTransactionSigOpCost(CTransaction(spendingTx), coins, flags) ==
           0);
    scriptPubKey[0] = 0x00;
    BuildTxs(spendingTx, coins, creationTx, scriptPubKey, scriptSig,
             scriptWitness);

    spendingTx.vin[0].prevout.SetNull();
    assert(GetTransactionSigOpCost(CTransaction(spendingTx), coins, flags) ==
           0);
  }

  {
    CScript p2pk = CScript() << ToByteVector(pubkey) << OP_CHECKSIG;
    CScript scriptSig = GetScriptForWitness(p2pk);
    CScript scriptPubKey = GetScriptForDestination(CScriptID(scriptSig));
    scriptSig = CScript() << ToByteVector(scriptSig);
    CScriptWitness scriptWitness;
    scriptWitness.stack.push_back(std::vector<unsigned char>(0));
    scriptWitness.stack.push_back(std::vector<unsigned char>(0));

    BuildTxs(spendingTx, coins, creationTx, scriptPubKey, scriptSig,
             scriptWitness);
    assert(GetTransactionSigOpCost(CTransaction(spendingTx), coins, flags) ==
           1);
    assert(VerifyWithFlag(creationTx, spendingTx, flags) ==
           SCRIPT_ERR_EQUALVERIFY);
  }

  {
    CScript witnessScript = CScript()
                            << 1 << ToByteVector(pubkey) << ToByteVector(pubkey)
                            << 2 << OP_CHECKMULTISIGVERIFY;
    CScript scriptPubKey = GetScriptForWitness(witnessScript);
    CScript scriptSig = CScript();
    CScriptWitness scriptWitness;
    scriptWitness.stack.push_back(std::vector<unsigned char>(0));
    scriptWitness.stack.push_back(std::vector<unsigned char>(0));
    scriptWitness.stack.push_back(
        std::vector<unsigned char>(witnessScript.begin(), witnessScript.end()));

    BuildTxs(spendingTx, coins, creationTx, scriptPubKey, scriptSig,
             scriptWitness);
    assert(GetTransactionSigOpCost(CTransaction(spendingTx), coins, flags) ==
           2);
    assert(GetTransactionSigOpCost(CTransaction(spendingTx), coins,
                                   flags & ~SCRIPT_VERIFY_WITNESS) == 0);
    assert(VerifyWithFlag(creationTx, spendingTx, flags) ==
           SCRIPT_ERR_CHECKMULTISIGVERIFY);
  }

  {
    CScript witnessScript = CScript()
                            << 1 << ToByteVector(pubkey) << ToByteVector(pubkey)
                            << 2 << OP_CHECKMULTISIGVERIFY;
    CScript redeemScript = GetScriptForWitness(witnessScript);
    CScript scriptPubKey = GetScriptForDestination(CScriptID(redeemScript));
    CScript scriptSig = CScript() << ToByteVector(redeemScript);
    CScriptWitness scriptWitness;
    scriptWitness.stack.push_back(std::vector<unsigned char>(0));
    scriptWitness.stack.push_back(std::vector<unsigned char>(0));
    scriptWitness.stack.push_back(
        std::vector<unsigned char>(witnessScript.begin(), witnessScript.end()));

    BuildTxs(spendingTx, coins, creationTx, scriptPubKey, scriptSig,
             scriptWitness);
    assert(GetTransactionSigOpCost(CTransaction(spendingTx), coins, flags) ==
           2);
    assert(VerifyWithFlag(creationTx, spendingTx, flags) ==
           SCRIPT_ERR_CHECKMULTISIGVERIFY);
  }
}

BOOST_AUTO_TEST_SUITE_END()
