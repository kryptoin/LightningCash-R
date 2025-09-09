// Copyright (c) 2015-2025 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <zmq/zmqabstractnotifier.h>
#include <util.h>

CZMQAbstractNotifier::~CZMQAbstractNotifier()
{
    assert(!psocket);
}

bool CZMQAbstractNotifier::NotifyBlock(const CBlockIndex *
)
{
    return true;
}

bool CZMQAbstractNotifier::NotifyTransaction(const CTransaction &
)
{
    return true;
}
