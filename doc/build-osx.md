LightningCashr Core – macOS (Apple Silicon) Build Guide
========================================================

This guide is tailored for modern Apple Silicon Macs (M1, M2, M3, M4) using macOS Ventura or newer. It focuses on building headless (non-GUI) LightningCashr Core using Homebrew.

---

System Preparation
------------------
1. Install Xcode Command Line Tools (required for compilers and build tools):

    xcode-select --install

2. Install Homebrew from https://brew.sh (if not already installed):

    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

After installation, add Homebrew to your shell profile if prompted.

---

Dependencies
------------
Install required packages using Homebrew:

    brew install automake libtool boost openssl@3 pkg-config libevent \\
    berkeley-db@4 miniupnpc protobuf zeromq python3

Set environment paths for OpenSSL and Berkeley DB 4.8:

    export CPPFLAGS="-I/opt/homebrew/opt/openssl@3/include -I/opt/homebrew/opt/berkeley-db@4/include"
    export LDFLAGS="-L/opt/homebrew/opt/openssl@3/lib -L/opt/homebrew/opt/berkeley-db@4/lib"
    export PKG_CONFIG_PATH="/opt/homebrew/opt/openssl@3/lib/pkgconfig"

If you installed Homebrew under a non-default location (like Intel-based Macs), adjust paths accordingly (`/usr/local/opt/...`).

---

Berkeley DB 4.8 (Wallet Support)
--------------------------------
Homebrew provides Berkeley DB 4.8 as `berkeley-db@4`.

If you need to build it manually:

    ./contrib/install_db4.sh `pwd`
    export BDB_PREFIX=`pwd`/db4
    export BDB_CFLAGS="-I$BDB_PREFIX/include"
    export BDB_LIBS="-L$BDB_PREFIX/lib -ldb_cxx"

Only required if building with wallet support.

---

Build LightningCashr Core (Headless)
------------------------------------
Clone the source:

    git clone https://github.com/lightningcashr-project/lightningcashr.git
    cd lightningcashr

Run the build steps:

    ./autogen.sh
    ./configure --without-gui $BDB_CFLAGS $BDB_LIBS
    make -j$(sysctl -n hw.ncpu)

Optional tests:

    make check

Optional install:

    sudo make install

Alternatively, copy binaries manually:

    cp src/lightningcashrd /usr/local/bin/
    cp src/lightningcashr-cli /usr/local/bin/

---

Running
-------
1. Create the configuration file:

    mkdir -p "$HOME/Library/Application Support/LightningCashr"
    echo -e "rpcuser=lightningcashrrpc\\nrpcpassword=$(xxd -l 16 -p /dev/urandom)" > "$HOME/Library/Application Support/LightningCashr/lightningcashr.conf"
    chmod 600 "$HOME/Library/Application Support/LightningCashr/lightningcashr.conf"

2. Start the daemon:

    lightningcashrd -daemon

3. Monitor progress:

    tail -f "$HOME/Library/Application Support/LightningCashr/debug.log"

4. Run commands:

    lightningcashr-cli help
    lightningcashr-cli getblockchaininfo

---

Notes
-----
- This guide targets Apple Silicon (arm64). Ensure your Homebrew is installed under `/opt/homebrew`.
- macOS Intel users should substitute `/opt/homebrew` with `/usr/local` where appropriate.
- To disable the wallet completely: `./configure --disable-wallet --without-gui`
- ZMQ and UPnP support are included via `zeromq` and `miniupnpc` packages.

---

Done!
-----
You’ve successfully built a headless LightningCashr Core node on your Apple Silicon Mac 🎉