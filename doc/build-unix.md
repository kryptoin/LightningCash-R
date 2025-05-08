LightningCash-R Core – Ubuntu Build Guide
========================================

This guide provides step-by-step instructions to build LightningCash-R Core from source on Ubuntu 20.04 LTS or newer.

System Preparation
------------------
Install required dependencies:

    sudo apt update
    sudo apt install build-essential libtool autotools-dev automake pkg-config \\
    libssl-dev libevent-dev bsdmainutils python3 software-properties-common \\
    libboost-system-dev libboost-filesystem-dev libboost-chrono-dev \\
    libboost-program-options-dev libboost-test-dev libboost-thread-dev

Berkeley DB 4.8 (Wallet Support)
--------------------------------
Install precompiled version:

    sudo add-apt-repository ppa:bitcoin/bitcoin
    sudo apt update
    sudo apt install libdb4.8-dev libdb4.8++-dev

Set Berkeley DB path:

    export BDB_PREFIX="/usr"

Alternatively, build from source:

    ./contrib/install_db4.sh `pwd`
    export BDB_PREFIX=`pwd`/db4

Build Instructions
------------------
Clone and prepare:

    git clone https://github.com/kryptoin/LightningCash-R.git
    cd lightningcashr
    ./autogen.sh

Configure with wallet:

    ./configure BDB_CFLAGS="-I$BDB_PREFIX/include" BDB_LIBS="-L$BDB_PREFIX/lib -ldb_cxx"

Build:

    make -j$(nproc)

Optional install:

    sudo make install

Build Without Wallet or GUI
---------------------------
    export BDB_PREFIX='/db4'
    CFLAGS="-fPIC" CXXFLAGS="-fPIC" ./configure BDB_LIBS="-L${BDB_PREFIX}/lib -ldb_cxx-4.8" BDB_CFLAGS="-I${BDB_PREFIX}/include"  --disable-wallet --without-gui --without-miniupnpc
    make -j$(nproc)

Low Memory Build
----------------
    ./configure CXXFLAGS="--param ggc-min-expand=1 --param ggc-min-heapsize=32768"

Optional Features
-----------------
ZMQ support:

    sudo apt install libzmq3-dev

UPnP support:

    sudo apt install libminiupnpc-dev

GUI (lightningcashr-qt)
-----------------------
Qt 5:

    sudo apt install qttools5-dev qttools5-dev-tools libqt5gui5 libqt5core5a \\
    libqt5dbus5 libprotobuf-dev protobuf-compiler

Qt 4:

    sudo apt install libqt4-dev libprotobuf-dev protobuf-compiler

QR code support:

    sudo apt install libqrencode-dev

Security Hardening
------------------
Enabled by default. To toggle:

    ./configure --enable-hardening
    ./configure --disable-hardening

Check protections:

    sudo apt install pax-utils
    scanelf -e ./src/lightningcashrd

Expect to see:
    TYPE: ET_DYN
    STK: RW-

Boost (Custom Builds Only)
--------------------------
    export BOOST_ROOT="/path/to/boost"

ARM Cross-Compilation
---------------------
    sudo apt install g++-arm-linux-gnueabihf curl
    cd depends
    make HOST=arm-linux-gnueabihf NO_QT=1
    cd ..
    ./configure --prefix=$PWD/depends/arm-linux-gnueabihf --enable-glibc-back-compat \\
    --enable-reduce-exports LDFLAGS=-static-libstdc++
    make -j$(nproc)

Notes
-----
- Use absolute paths for --prefix, $BDB_PREFIX, and $BOOST_ROOT
- Use ./configure --help for full options
- Run make clean to rebuild from scratch

Done!
-----
You’ve successfully built LightningCash-R Core on Ubuntu 🎉