// Copyright (c) 2011-2025 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_TRANSACTIONDESC_H
#define BITCOIN_QT_TRANSACTIONDESC_H

#include <QObject>
#include <QString>

class TransactionRecord;

class CWallet;
class CWalletTx;

class TransactionDesc : public QObject {
  Q_OBJECT

public:
  static QString toHTML(CWallet *wallet, CWalletTx &wtx, TransactionRecord *rec,
                        int unit);

private:
  TransactionDesc() {}

  static QString FormatTxStatus(const CWalletTx &wtx);
};

#endif
