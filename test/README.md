LightningCash-R Core Testing Guide (2025)
================================================================

This directory contains integration tests for `lightningcashrd` and its utilities. These tests evaluate the full behavior of the system, including RPC and P2P functionality. Unit tests are located separately in:

- /src/test
- /src/wallet/test

Overview of Test Suites
------------------------

There are two main types of tests included:

- functional: Tests lightningcashrd and lightningcashr-qt through their public interfaces (RPC and P2P).
- util: Tests the LightningCashr utilities, such as lightningcashr-tx.

Util tests are run automatically via `make check`. Functional tests are run during CI workflows (e.g., GitHub Actions) and can also be executed locally.

Running Tests Locally on Ubuntu
-------------------------------

Ensure that the build includes wallet, utils, and daemon support:

    ./configure --with-utils --with-daemon --enable-wallet
    make

Functional Test Dependencies
----------------------------

To run the functional tests, install the following packages:

    sudo apt update
    sudo apt install -y python3 python3-pip python3-venv python3-zmq
    pip3 install -r test/functional/requirements.txt

Note: requirements.txt should list all needed Python packages, such as pytest, pyzmq, and requests.

Running Functional Tests
------------------------

Running Individual Tests:

    test/functional/wallet_create_tx.py

Or with the test runner:

    test/functional/test_runner.py wallet_create_tx.py

Running the Full Suite:

    test/functional/test_runner.py

Running Extended Tests:

    test/functional/test_runner.py --extended

Parallel Test Execution:

    test/functional/test_runner.py --jobs=6

Debugging Test Failures
-----------------------

Port Conflicts:

    pkill -9 lightningcashrd

Cache Issues:

    rm -rf test/cache
    pkill -9 lightningcashrd

Logging and Verbose Output:

    less test/test_framework.log

Combine and colorize logs:

    test/functional/combine_logs.py -c test/functional/tempdir | less -r

Trace all RPC calls:

    test/functional/test_runner.py --tracerpc

Keep the test data directory:

    test/functional/test_runner.py --nocleanup

Attaching a Debugger
--------------------

Insert a Python breakpoint:

    import pdb; pdb.set_trace()

To attach gdb to a running lightningcashrd instance:

    pid=$(cat /tmp/test_xxx/node0/regtest/lightningcashrd.pid)
    sudo gdb $(which lightningcashrd) $pid

Util Tests
----------

Util tests can be executed directly:

    test/util/lightningcashr-util-test.py -v

Writing Functional Tests
------------------------

New functionality must include accompanying functional tests.

To create a new test:
- Place it in test/functional/
- Use the BitcoinTestFramework base class.
- Follow the structure of existing test scripts.

Consult the documentation and examples in test/functional/ for best practices.